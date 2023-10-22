#include <sys/unistd.h>
#include <memory.h>

#include "stuff/log.h"
#include "internal/internal_comm.h"
#include "stuff/blynk_freertos_port.h"

#define TAG "[INTERNAL COMMUNICATION]"


static tick_t get_timeout(blynk_device_t* device);


blynk_err_t
blynk_notify_packet_ready(blynk_packet_t* packet) {
    blynk_request_info_t* req_info = (blynk_request_info_t*) malloc(sizeof(blynk_request_info_t));

    if (!req_info) {
        log_error("%s: Function %s cannot allocate memory", TAG, __func__);
        return BLYNK_EC_MEM;
    }

    req_info->message.command = packet->cmd;
    req_info->message.id = packet->id;
    req_info->message.length = packet->len;
    req_info->deadline = packet->handler ? get_timeout(packet->device) + get_tick_count() : 0;
    req_info->handler = packet->handler;
    req_info->data = packet->data;

    if (packet->cmd != BLYNK_CMD_RESPONSE && packet->len && packet->payload) {
        if (packet->len > sizeof(req_info->message.payload)) {
            packet->len = req_info->message.length = sizeof(req_info->message.payload);
        }
        memcpy(req_info->message.payload, packet->payload, packet->len);
    }

    if (!queue_send(packet->device->priv_data.ctl_queue, &req_info, ms_to_ticks(packet->wait))) {
        log_error("%s: Function %s cannot send payload by queue", TAG, __func__);
        free(req_info);
        return BLYNK_EC_MEM;
    }

    uint8_t dummy = 0;
    int fd = packet->device->priv_data.ctl_sockets[WRITE_SOCK];
    ssize_t write_status = write(fd, &dummy, sizeof(dummy));

    if (SYSCALL_FAILED(write_status)) {
        log_error("%s: Function %s cannot send notify message", TAG, __func__);
        free(req_info);
        return BLYNK_EC_ERRNO;
    }

    return BLYNK_EC_OK;
}


static tick_t
get_timeout(blynk_device_t* device) {
    mutex_wrap_t wrap = {
            .type = MUTEX_TYPE_FREERTOS,
            .mutex = {device->control.mtx},
    };

    mutex_wrapper_take(&wrap);
    tick_t timeout = device->control.connection_config.connection.connection_timeout_ms;
    mutex_wrapper_give(&wrap);

    return ms_to_ticks(timeout);
}
