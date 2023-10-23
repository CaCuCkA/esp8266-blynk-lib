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

#include <sys/unistd.h>
#include <memory.h>

#include "stuff/log.h"
#include "internal/internal_comm.h"
#include "stuff/blynk_freertos_port.h"

#define TAG "[INTERNAL COMMUNICATION]"


static tick_t get_timeout(blynk_device_t* device);


blynk_err_t
blynk_notify_packet_ready(blynk_packet_t* packet) {
    blynk_request_info_t req_info;

    req_info.message.command = packet->cmd;
    req_info.message.id = packet->id;
    req_info.message.length = packet->len;
    req_info.deadline = packet->handler ? get_timeout(packet->device) + get_tick_count() : 0;
    req_info.handler = packet->handler;
    req_info.data = packet->data;

    if (packet->cmd != BLYNK_CMD_RESPONSE && packet->len && packet->payload) {
        if (packet->len > sizeof(req_info.message.payload)) {
            packet->len = req_info.message.length = sizeof(req_info.message.payload);
        }
        memcpy(req_info.message.payload, packet->payload, packet->len);
    }

    if (!queue_send(packet->device->priv_data.ctl_queue, &req_info, ms_to_ticks(packet->wait))) {
        log_error("%s: Function %s cannot send payload by queue", TAG, __func__);
        return BLYNK_EC_MEM;
    }

    uint8_t dummy = 0;
    int fd = packet->device->priv_data.ctl_sockets[WRITE_SOCK];
    ssize_t write_status = write(fd, &dummy, sizeof(dummy));

    if (SYSCALL_FAILED(write_status)) {
        log_error("%s: Function %s cannot send notify message", TAG, __func__);
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
