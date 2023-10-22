#include <sys/unistd.h>
#include <memory.h>
#include <errno.h>

#include "stuff/log.h"
#include "stuff/util.h"
#include "stuff/types.h"
#include "internal/protocol.h"
#include "stuff/communication.h"
#include "internal/internal_comm.h"
#include "internal/protocol_stuff.h"
#include "internal/protocol_parser.h"
#include "stuff/blynk_freertos_port.h"

#define TAG "[PROTOCOL]"


static uint32_t free_pointer_time = 0;

static blynk_err_t send_heartbit(blynk_device_t* device);

static blynk_err_t blynk_busy_loop(blynk_device_t* device);

static blynk_err_t authorize_device(blynk_device_t* device);

static void update_heartbeat_deadline(blynk_device_t* device);

static void prepare_device_communication(blynk_device_t* device);

static blynk_err_t handle_read_from_ctl_socket(blynk_device_t* device);

static blynk_err_t manage_communication_deadlines(blynk_device_t* device);

static inline bool has_deadline_passed(tick_t deadline, tick_t current_time);

static bool determined_closest_deadline(blynk_device_t* device, tick_t* deadline);

static void setup_fd_sets(fd_set* rdset, fd_set* wrset, int communication_socket,
                          const blynk_private_data_t* priv_data);

static void process_device_communication(blynk_device_t* device, int communication_socket);

static blynk_err_t handle_write_to_main_socket(blynk_device_t* device, int communication_socket);

static blynk_err_t handle_read_from_main_socket(blynk_device_t* device, int communication_socket);

static void authentication_handler(blynk_device_t* device, blynk_status_t status, void* data);

static void heartbit_callback(blynk_device_t* device, blynk_status_t status, UNUSED void* data);

static void handle_awaiting_timer(blynk_device_t* device, tick_t now, blynk_awaiting_t* awaiting);

static blynk_err_t prepare_blynk_request(blynk_device_t* device, blynk_request_info_t** request_ptr,
                                         int socket_fd, fd_set* write_set);

static int wait_for_fd_activity(fd_set* read_fds, fd_set* write_fds, struct timeval* timeout_value,
                                blynk_device_t* device);

void
blynk_run_task(void* pvParameters) {
    blynk_device_t* device = (blynk_device_t*) pvParameters;
    mutex_wrap_t wrap = {
            .type = MUTEX_TYPE_FREERTOS,
            .mutex = {device->control.mtx},
    };

    while (blynk_busy_loop(device) == BLYNK_EC_OK) {
        mutex_wrapper_take(&wrap);
        tick_t delay = device->control.connection_config.connection.reconnection_interval_ms;
        mutex_wrapper_give(&wrap);

        task_delay(delay / custom_port_tick_max_rate);
    }

    log_warn("%s: Task %s was terminated", TAG, __func__);
    task_delete(CURRENT_TASK_HANDLE);
}


static blynk_err_t
blynk_busy_loop(blynk_device_t* device) {
    mutex_wrap_t wrap = {
            .type = MUTEX_TYPE_FREERTOS,
            .mutex = {device->control.mtx},
    };

    mutex_wrapper_take(&wrap);
    blynk_config_t conn_config = device->control.connection_config;
    mutex_wrapper_give(&wrap);

    int communication_socket;
    conn_err_t status_code = setup_cloud_connection(conn_config.server.server_url, &communication_socket);

    if (status_code != CONN_EC_OK) {
        if (status_code == CONN_EC_GET_HOST_AND_PORT) return BLYNK_EC_INVALID_OPTION;

        blynk_err_t error_type = (status_code != CONN_EC_GET_ADDRINFO) ? BLYNK_EC_ERRNO : BLYNK_EC_GAI;
        int error_detail = (status_code != CONN_EC_GET_ADDRINFO) ? errno : (int) status_code;

        disconnect_device(device, error_type, error_detail);
        return BLYNK_EC_OK;
    }

    prepare_device_communication(device);

    process_device_communication(device, communication_socket);

    return BLYNK_EC_OK;
}


static void
prepare_device_communication(blynk_device_t* device) {
    update_device_communication_state(device, BLYNK_STATE_CONNECTED);

    memset(device->priv_data.awaiting, 0, sizeof(device->priv_data.awaiting));

    queue_reset(device->priv_data.ctl_queue);

    device->priv_data.parser = message_payload_parser;
    device->priv_data.request_id = 1;
    device->priv_data.buf_size = 0;

    update_heartbeat_deadline(device);
}


static void
process_device_communication(blynk_device_t* device, int communication_socket) {
    blynk_err_t status_code = authorize_device(device);
    if (status_code != BLYNK_EC_OK) {
        log_error("%s: Function %s cannot log the device", TAG, __func__);
        disconnect_device(device, status_code, status_code == BLYNK_EC_ERRNO ? errno : BLYNK_EC_OK);
        close(communication_socket);
        return;
    }

    while (true) {
        fd_set rdset;
        fd_set wrset;
        setup_fd_sets(&rdset, &wrset, communication_socket, &device->priv_data);

        blynk_request_info_t* request_ptr;
        if (prepare_blynk_request(device, &request_ptr, communication_socket, &wrset)) break;

        tick_t deadline;
        struct timeval timeval;
        bool deadline_detected = determined_closest_deadline(device, &deadline);

        if (deadline_detected) {
            const time_t total_microseconds = (time_t) deadline * custom_port_tick_max_rate;
            timeval.tv_sec = total_microseconds / MS_TO_SEC;
            timeval.tv_usec = (total_microseconds % MS_TO_SEC) * MS_TO_USEC;
        }

        int active_fd_count = wait_for_fd_activity(&rdset, NULL, deadline_detected ? &timeval : NULL, device);
        if (active_fd_count < 0) {
            if (request_ptr) free(request_ptr);
            break;
        }

        status_code = manage_communication_deadlines(device);
        if (status_code != BLYNK_EC_OK) {
            log_error("%s: Function %s cannot manage communication deadlines", TAG, __func__);
            disconnect_device(device, status_code, status_code == BLYNK_EC_ERRNO ? errno : 0);
            if (request_ptr) free(request_ptr);

            break;
        }

        if (device->control.state == BLYNK_STATE_DISCONNECTED) {
            log_error("Device is disconnect");
            if (request_ptr) free(request_ptr);
            break;
        }

        if (active_fd_count == 0) continue;

        if (FD_ISSET(device->priv_data.ctl_sockets[READ_SOCK_ID], &rdset)) {
            if (handle_read_from_ctl_socket(device) != BLYNK_EC_OK) {
                if (request_ptr) free(request_ptr);
                break;
            }
        }

        if (FD_ISSET(communication_socket, &rdset)) {
            if (handle_read_from_main_socket(device, communication_socket) != BLYNK_EC_OK) {
                if (request_ptr) free(request_ptr);
                break;
            }
        }

        if (device->priv_data.buf_size && FD_ISSET(communication_socket, &wrset)) {
            if (handle_write_to_main_socket(device, communication_socket) != BLYNK_EC_OK) {
                if (request_ptr) free(request_ptr);
                break;
            }
        }

    }

    close(communication_socket);
}


static blynk_err_t
authorize_device(blynk_device_t* device) {
    blynk_packet_t packet = {
            .device = device,
            .cmd = BLYNK_CMD_LOGIN,
            .id = LOGIN_RESPONSE_ID,
            .len = strlen(device->control.connection_config.server.auth_token),
            .payload = (uint8_t*) device->control.connection_config.server.auth_token,
            .handler = authentication_handler,
            .data = NO_CALLBACK_DATA,
            .wait = 0,

    };

    return blynk_notify_packet_ready(&packet);
}


static void
authentication_handler(blynk_device_t* device, blynk_status_t status, UNUSED void* data) {
    if (status != BLYNK_STATUS_SUCCESS) {
        if (status == BLYNK_STATUS_TIMEOUT) {
            log_error("Device does not connected!");
            disconnect_device(device, BLYNK_EC_TIMEOUT, 0);
        } else {
            disconnect_device(device, BLYNK_EC_STATUS, status);
        }
    } else {
        update_device_communication_state(device, BLYNK_STATE_AUTHENTICATED);
    }
}


static void
setup_fd_sets(fd_set* rdset, fd_set* wrset, int communication_socket, const blynk_private_data_t* priv_data) {
    FD_ZERO(rdset);
    FD_ZERO(wrset);

    FD_SET(communication_socket, rdset);
    FD_SET(priv_data->ctl_sockets[READ_SOCK_ID], rdset);
}


static blynk_err_t
prepare_blynk_request(blynk_device_t* device, blynk_request_info_t** request_ptr, int socket_fd, fd_set* write_set) {
    blynk_private_data_t* device_data = &device->priv_data;

    if (device_data->buf_size != 0) {
        FD_SET(socket_fd, write_set);
        return BLYNK_EC_OK;
    }

    blynk_request_info_t* req_buffer = NULL;
    bool is_received = queue_receive(device_data->ctl_queue, &req_buffer, NO_WAITING);

    if (!is_received) return BLYNK_EC_OK;

    if (*request_ptr != NULL) free(*request_ptr);

    if (!req_buffer->message.id) {
        uint16_t msg_id = allocate_request_id(device,
                                              req_buffer->deadline,
                                              req_buffer->handler,
                                              req_buffer->data);

        if (!msg_id) {
            log_error("%s: %s failed to generate message ID", TAG, __func__);
            disconnect_device(device, BLYNK_EC_MEM, 0);
            if (req_buffer) free(req_buffer);

            return BLYNK_EC_MEM;
        }

        req_buffer->message.id = msg_id;
    }

    device_data->buf_size = compose_blynk_message(device_data->write_buffer, BLYNK_MAX_PAYLOAD_LEN,
                                                  &req_buffer->message);
    device_data->total_byte_send = 0;

    *request_ptr = req_buffer;
    FD_SET(socket_fd, write_set);

    return BLYNK_EC_OK;
}


static bool
determined_closest_deadline(blynk_device_t* device, tick_t* deadline) {
    bool deadline_found = false;
    tick_t closest_deadline = 0;
    tick_t current_time = get_tick_count();

    for (uint16_t i = 0; i < BLYNK_MAX_AWAITING; ++i) {
        if (!device->priv_data.awaiting[i].id || !device->priv_data.awaiting[i].deadline) continue;

        if (has_deadline_passed(device->priv_data.awaiting[i].deadline, current_time)) {
            *deadline = 0;
            return true;
        }

        tick_t time_left = device->priv_data.awaiting[i].deadline - current_time;
        if (!deadline_found || time_left < closest_deadline) {
            closest_deadline = time_left;
            deadline_found = true;
        }
    }

    if (device->priv_data.heartbit_deadline) {
        if (has_deadline_passed(device->priv_data.heartbit_deadline, current_time)) {
            *deadline = 0;
            return true;
        }

        tick_t time_left_for_sending_heartbit = device->priv_data.heartbit_deadline - current_time;
        if (!deadline_found || time_left_for_sending_heartbit < closest_deadline) {
            closest_deadline = time_left_for_sending_heartbit;
            deadline_found = true;
        }
    }


    if (deadline_found) *deadline = closest_deadline;

    return deadline_found;
}


static int
wait_for_fd_activity(fd_set* read_fds, fd_set* write_fds, struct timeval* timeout_value,
                     blynk_device_t* device) {
    int active_fd_count = select(FD_SETSIZE, read_fds, write_fds, NULL, timeout_value);
    if (active_fd_count < 0) {
        disconnect_device(device, BLYNK_EC_SYSTEM, errno);
    }
    return active_fd_count;
}


static blynk_err_t
manage_communication_deadlines(blynk_device_t* device) {
    tick_t current_time = get_tick_count();

    for (uint16_t i = 0; i < BLYNK_MAX_AWAITING; ++i) {
        handle_awaiting_timer(device, current_time, &device->priv_data.awaiting[i]);
    }

    if (device->priv_data.heartbit_deadline && has_deadline_passed(device->priv_data.heartbit_deadline, current_time)) {
        update_heartbeat_deadline(device);
        return send_heartbit(device);
    }

    return BLYNK_EC_OK;
}


static void
handle_awaiting_timer(blynk_device_t* device, tick_t now, blynk_awaiting_t* awaiting) {
    if (!awaiting->id || !awaiting->deadline) return;

    if (has_deadline_passed(awaiting->deadline, now)) {
        if (awaiting->handler) {
            awaiting->handler(device, BLYNK_STATUS_TIMEOUT, awaiting->data);
        }
        awaiting->id = 0;
    }
}


static blynk_err_t
send_heartbit(blynk_device_t* device) {
    blynk_packet_t heartbit_packet = {
            .device = device,
            .cmd = BLYNK_CMD_PING,
            .id = 0,
            .len = 0,
            .payload = NULL,
            .handler = heartbit_callback,
            .data = NO_CALLBACK_DATA,
            .wait = NO_WAITING,
    };

    return blynk_notify_packet_ready(&heartbit_packet);
}


static inline bool
has_deadline_passed(tick_t deadline, tick_t current_time) {
    return (int32_t) ((uint32_t) deadline - (uint32_t) current_time) <= 0;
}


static void
heartbit_callback(blynk_device_t* device, blynk_status_t status, UNUSED void* data) {
    if (status != BLYNK_STATUS_SUCCESS) {
        if (status == BLYNK_STATUS_TIMEOUT) {
            disconnect_device(device, BLYNK_EC_TIMEOUT, 0);
        } else {
            disconnect_device(device, BLYNK_EC_STATUS, status);
        }
    }
}


static void
update_heartbeat_deadline(blynk_device_t* device) {
    mutex_wrap_t wrap = {
            .type = MUTEX_TYPE_FREERTOS,
            .mutex = {device->control.mtx},
    };

    mutex_wrapper_take(&wrap);
    TickType_t heartbeat_interval_ticks = device->control.connection_config.connection.heartbeat_interval_ms;
    mutex_wrapper_give(&wrap);

    device->priv_data.heartbit_deadline = get_tick_count() + heartbeat_interval_ticks / custom_port_tick_max_rate;
}


static blynk_err_t
handle_read_from_ctl_socket(blynk_device_t* device) {
    uint8_t dummy;
    int32_t read_descriptor = read(device->priv_data.ctl_sockets[READ_SOCK_ID], &dummy, sizeof(dummy));
    if (read_descriptor < 0 && errno != EAGAIN) {
        log_error("%s: Error %s while reading from ctl socket", TAG, __func__, strerror(errno));
        disconnect_device(device, BLYNK_EC_ERRNO, errno);
        return BLYNK_EC_FAILED_TO_READ;
    }

    return BLYNK_EC_OK;
}


static blynk_err_t
handle_read_from_main_socket(blynk_device_t* device, int communication_socket) {
    int read_bytes_num = read(communication_socket, device->priv_data.read_buffer, BLYNK_MAX_PAYLOAD_LEN);

    if (read_bytes_num < 0 && errno != EAGAIN) {
        log_error("%s: Error %s while reading from main socket", TAG, __func__, strerror(errno));
        disconnect_device(device, BLYNK_EC_ERRNO, errno);
        return BLYNK_EC_FAILED_TO_READ;
    } else if (read_bytes_num == 0) {
        log_error("%s: Unable to read from socket", TAG, __func__);
        disconnect_device(device, BLYNK_EC_CLOSED, 0);
        return BLYNK_EC_FAILED_TO_READ;
    }

    uint8_t* data = device->priv_data.read_buffer;
    while (read_bytes_num--) {
        device->priv_data.parser(device, *(data++));
        if (device->control.state == BLYNK_STATE_DISCONNECTED) {
            log_error("%s: Detected device disconnection", TAG, __func__);
            return BLYNK_EC_DEVICE_DISCONNECT;
        }
    }

    return BLYNK_EC_OK;
}


static blynk_err_t
handle_write_to_main_socket(blynk_device_t* device, int communication_socket) {
    ssize_t length = write(communication_socket,
                           device->priv_data.write_buffer + device->priv_data.total_byte_send,
                           device->priv_data.buf_size - device->priv_data.total_byte_send);

    if (length < 0 && errno != EAGAIN) {
        log_error("%s: Error %s while writing to main socket", TAG, __func__, strerror(errno));
        disconnect_device(device, BLYNK_EC_ERRNO, errno);
        return BLYNK_EC_FAILED_TO_WRITE;
    }

    device->priv_data.total_byte_send += length;

    if (device->priv_data.total_byte_send >= device->priv_data.buf_size) device->priv_data.buf_size = 0;

    return BLYNK_EC_OK;
}
