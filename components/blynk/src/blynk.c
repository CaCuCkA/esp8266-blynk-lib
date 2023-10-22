/*
 * MIT License
 *
 * Copyright (c) 2023 CaCuCkA
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <memory.h>

#include "blynk.h"
#include "stuff/log.h"
#include "stuff/util.h"
#include "internal/protocol.h"
#include "stuff/communication.h"
#include "internal/dispatching.h"
#include "internal/internal_comm.h"

#define TAG "[BLYNK]"


static void update_device_config(blynk_device_t* device, tick_t value, tick_t* config_field);

static blynk_err_t set_device_options(blynk_device_t* device, const char* authentication_token);

static void state_handler(UNUSED blynk_device_t* device, const blynk_state_event_t* event, UNUSED void* data);

static blynk_err_t blynk_set_state_handler(blynk_device_t* device, blynk_state_handler_t handler, void* data);


blynk_err_t
blynk_begin(blynk_device_t* device, const char* authentication_token) {
    memset(device, 0, sizeof(blynk_device_t));

    device->control.state = BLYNK_STATE_STOPPED;

    if (!(device->control.mtx = create_semaphore())) {
        log_error("%s: Function %s failed to create semaphore", TAG, __func__);
        return BLYNK_EC_MEM;
    }

    device->priv_data.ctl_queue = create_queue(QUEUE_SIZE, sizeof(blynk_device_t*));
    if (device->priv_data.ctl_queue == NULL) {
        log_error("%s: Function %s unable to create communication queue", TAG, __func__);
        return BLYNK_EC_MEM;
    }

    if (configure_internal_communication(device->priv_data.ctl_sockets) != CONN_EC_OK) {
        log_error("%s: Function %s failed to configure internal communication.", TAG, __func__);
        return BLYNK_EC_ERRNO;
    }

    device->valid = true;

    if (set_device_options(device, authentication_token) != BLYNK_EC_OK) {
        log_error("%s: Function %s failed to configure default device settings.", TAG, __func__);
        return BLYNK_EC_SET_OPTIONS;
    }

    return BLYNK_EC_OK;
}


static blynk_err_t
set_device_options(blynk_device_t* device, const char* authentication_token) {
    if (CHECK_PTR(TAG, authentication_token)) return BLYNK_EC_NULL_PTR;

    strncpy(device->control.connection_config.server.auth_token, authentication_token, BLYNK_AUTH_TOKEN_SIZE);
    device->control.connection_config.server.auth_token[33] = '\0';

    mutex_wrap_t wrap = {
            .type = MUTEX_TYPE_FREERTOS,
            .mutex = {device->control.mtx},
    };

    mutex_wrapper_take(&wrap);
    snprintf(device->control.connection_config.server.server_url,
             sizeof(DEFAULT_CLOUD_URL) + sizeof(DEFAULT_CLOUD_PORT), "%s:%s", DEFAULT_CLOUD_URL, DEFAULT_CLOUD_PORT);

    device->control.connection_config.connection.heartbeat_interval_ms = DEFAULT_HEARTBEAT_INTERVAL;
    device->control.connection_config.connection.connection_timeout_ms = DEFAULT_TIMEOUT;
    device->control.connection_config.connection.reconnection_interval_ms = DEFAULT_RECONNECT_DELAY;
    mutex_wrapper_give(&wrap);

    if (blynk_set_state_handler(device, state_handler, NULL) != BLYNK_EC_OK) {
        log_error("%s: Function %s failed to set state handler", TAG, __func__);
        return BLYNK_EC_SET_OPTIONS;
    }

    return BLYNK_EC_OK;
}


static blynk_err_t
blynk_set_state_handler(blynk_device_t* device, blynk_state_handler_t handler, void* data) {
    if (!BLYNK_DEVICE_IS_VALID(device)) {
        log_error("&s: Function %s. Device is not valid. Failed to set state handler", TAG, __func__);
        return BLYNK_EC_NOT_INITIALIZED;
    }

    mutex_wrap_t wrap = {
            .type = MUTEX_TYPE_FREERTOS,
            .mutex = {device->control.mtx},
    };

    mutex_wrapper_take(&wrap);
    device->control.on_state_change = handler;
    device->control.callback_user_data = data;
    mutex_wrapper_give(&wrap);

    return BLYNK_EC_OK;
}


static void
state_handler(UNUSED blynk_device_t* device, const blynk_state_event_t* event, UNUSED void* data) {
    log_info("%s: Blynk connection state: %d", TAG, event->conn_status);
}


blynk_state_t
blynk_get_device_state(blynk_device_t* device) {
    mutex_wrap_t wrap = {
            .type = MUTEX_TYPE_FREERTOS,
            .mutex = {device->control.mtx},
    };

    mutex_wrapper_take(&wrap);
    blynk_state_t state = device->control.state;
    mutex_wrapper_give(&wrap);

    return state;
}


blynk_err_t
blynk_send_with_callback(blynk_device_t* device,
                         uint8_t cmd,
                         blynk_response_handler_t handler,
                         void* data,
                         tick_t wait,
                         const char* fmt,
                         ...) {
    va_list ap;
    va_start(ap, fmt);
    blynk_err_t ret = dispatch_blynk_request(device, cmd, handler, data, wait, fmt, ap);
    va_end(ap);
    return ret;
}


blynk_err_t
blynk_send(blynk_device_t* device, blynk_cmd_t cmd, tick_t wait, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    blynk_err_t status_code = dispatch_blynk_request(device, cmd, NULL, NULL, wait, fmt, ap);
    va_end(ap);
    return status_code;
}


blynk_err_t
blynk_send_response(blynk_device_t* device, uint16_t id, uint16_t status, tick_t wait) {
    if (!BLYNK_DEVICE_IS_VALID(device)) {
        log_error("&s: Function %s. Device is not valid. Failed to send blynk response", TAG, __func__);
        return BLYNK_EC_NOT_INITIALIZED;
    }


    blynk_packet_t packet = {
            .device = device,
            .cmd = BLYNK_CMD_RESPONSE,
            .id = id,
            .len = status,
            .payload = NULL,
            .handler = NULL,
            .data = NULL,
            .wait = wait,
    };

    return blynk_notify_packet_ready(&packet);
}


blynk_err_t
update_default_timeout(blynk_device_t* device, tick_t timeout) {
    if (!BLYNK_DEVICE_IS_VALID(device)) {
        log_error("&s: Function %s. Device is not valid. Failed to update default timeout", TAG, __func__);
        return BLYNK_EC_NOT_INITIALIZED;
    }

    update_device_config(device, timeout, &device->control.connection_config.connection.connection_timeout_ms);

    return BLYNK_EC_OK;
}


blynk_err_t
update_heartbeat_interval(blynk_device_t* device, tick_t heartbit_interval) {
    if (!BLYNK_DEVICE_IS_VALID(device)) {
        log_error("&s: Function %s. Device is not valid. Failed to update default heartbeat interval", TAG, __func__);
        return BLYNK_EC_NOT_INITIALIZED;
    }

    update_device_config(device, heartbit_interval,
                         &device->control.connection_config.connection.heartbeat_interval_ms);

    return BLYNK_EC_OK;
}


blynk_err_t
update_default_reconnection_delay(blynk_device_t* device, tick_t reconnection_delay) {
    if (!BLYNK_DEVICE_IS_VALID(device)) {
        log_error("&s: Function %s. Device is not valid. Failed to update default reconnection delay", TAG, __func__);
        return BLYNK_EC_NOT_INITIALIZED;

    }

    update_device_config(device, reconnection_delay,
                         &device->control.connection_config.connection.reconnection_interval_ms);

    return BLYNK_EC_OK;
}


static void
update_device_config(blynk_device_t* device, tick_t value, tick_t* config_field) {
    mutex_wrap_t wrap = {
            .type = MUTEX_TYPE_FREERTOS,
            .mutex = {device->control.mtx},
    };

    mutex_wrapper_take(&wrap);
    *config_field = value;
    mutex_wrapper_give(&wrap);
}


blynk_err_t
update_default_state_handler(blynk_device_t* device, blynk_state_handler_t handler, void* user_data) {
    return blynk_set_state_handler(device, handler, user_data);
}


blynk_err_t
blynk_register_cmd_handler(blynk_device_t* device, const char* action, blynk_cmd_handler_t handler, void* data) {
    if (!BLYNK_DEVICE_IS_VALID(device)) {
        log_error("[BLYNK]: Device is not valid. Failed to register command handler for action: %s", action);
        return BLYNK_EC_NOT_INITIALIZED;
    }

    mutex_wrap_t wrap = {
            .type = MUTEX_TYPE_FREERTOS,
            .mutex = {device->control.mtx},
    };

    mutex_wrapper_take(&wrap);

    for (uint8_t i = 0; i < BLYNK_MAX_HANDLERS; ++i) {
        if (!strncmp(device->control.handlers[i].action, action, BLYNK_ACTION_SIZE)) {
            device->control.handlers[i].handler = handler;
            device->control.handlers[i].data = data;
            mutex_wrapper_give(&wrap);
            return BLYNK_EC_OK;
        }
    }

    for (uint8_t i = 0; i < BLYNK_MAX_HANDLERS; ++i) {
        if (!device->control.handlers[i].action[0]) {
            strlcpy(device->control.handlers[i].action, action, BLYNK_ACTION_SIZE);
            device->control.handlers[i].handler = handler;
            device->control.handlers[i].data = data;
            mutex_wrapper_give(&wrap);
            return BLYNK_EC_OK;
        }
    }

    log_error("[BLYNK]: Failed to register handler for action %s. No available slots.", action);
    mutex_wrapper_give(&wrap);
    return BLYNK_EC_MEM;
}


blynk_err_t
blynk_deregister_cmd_handler(blynk_device_t* device, const char* action) {
    if (!BLYNK_DEVICE_IS_VALID(device)) {
        log_error("[BLYNK]: Device is not valid. Failed to register command handler for action: %s", action);
        return BLYNK_EC_NOT_INITIALIZED;
    }

    mutex_wrap_t wrap = {
            .type = MUTEX_TYPE_FREERTOS,
            .mutex = {device->control.mtx},
    };

    mutex_wrapper_take(&wrap);
    for (uint8_t i = 0; i < BLYNK_MAX_HANDLERS; ++i) {
        if (!strncmp(device->control.handlers[i].action, action, BLYNK_ACTION_SIZE)) {
            device->control.handlers[i].handler = NO_HANDLERS;
            device->control.handlers[i].data = NO_CALLBACK_DATA;
            device->control.handlers[i].action[0] = NO_ACTION;
            break;
        }
    }
    mutex_wrapper_give(&wrap);

    return BLYNK_EC_OK;
}


blynk_err_t
blynk_run(blynk_device_t* device) {
    if (!BLYNK_DEVICE_IS_VALID(device)) {
        log_error("%s: Function %s. Device is not valid. Failed to start Blynk run task.", TAG, __func__);
        return BLYNK_EC_NOT_INITIALIZED;
    }

    mutex_wrap_t wrap = {
            .type = MUTEX_TYPE_FREERTOS,
            .mutex = {device->control.mtx},
    };

    mutex_wrapper_take(&wrap);

    if (device->control.state != BLYNK_STATE_STOPPED) {
        log_error("%s: Function %s attempted to start Blynk run task, but device is already running.", TAG, __func__);
        mutex_wrapper_give(&wrap);
        return BLYNK_EC_RUNNING;
    }

    if (!create_task("blynk run task", blynk_run_task, device, BLYNK_STACK_SIZE, &device->control.task)) {
        log_error("%s: Function %s failed to create Blynk run task. Insufficient memory.", TAG, __func__);
        mutex_wrapper_give(&wrap);
        return BLYNK_EC_MEM;
    }

    device->control.state = BLYNK_STATE_DISCONNECTED;

    void* data = device->control.callback_user_data;
    blynk_state_handler_t handler = device->control.on_state_change;
    mutex_wrapper_give(&wrap);

    blynk_state_event_t event = {
            .conn_status = BLYNK_STATE_DISCONNECTED,
            .disconnected = {
                    .reason = BLYNK_EC_OK,
            },
    };

    if (handler) {
        handler(device, &event, data);
    } else {
        log_warn("%s: Function %s State handler is not set. State changes may go unnoticed.", TAG, __func__);
    }

    return BLYNK_EC_OK;
}
