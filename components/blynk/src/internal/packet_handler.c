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

#include <string.h>
#include <errno.h>

#include "stuff/log.h"
#include "stuff/util.h"
#include "stuff/defines.h"
#include "internal/internal_comm.h"
#include "internal/packet_handler.h"
#include "internal/protocol_stuff.h"
#include "stuff/blynk_freertos_port.h"

#define TAG "[PACKET HANDLER]"

static void handle_response(blynk_device_t* device);

static void handle_hardware(blynk_device_t* device);

static blynk_err_t handle_hardware_package(blynk_device_t* device);

static blynk_cmd_handler_t find_handler_for_command(blynk_control_t* ctl, char* command);

static int32_t extract_args_from_payload(blynk_private_data_t* private_data, char* args[]);

static void process_hardware_message(blynk_device_t* device, char* args[], int32_t args_num);

static int32_t split_payload_into_args(char* payload, uint32_t len, char** args, uint32_t size);


void
handle_message_packet(blynk_device_t* device) {
    blynk_private_data_t* priv_data = &device->priv_data;

    switch (priv_data->message.command) {
        case BLYNK_CMD_RESPONSE:
            handle_response(device);
            break;
        case BLYNK_CMD_HARDWARE:
            handle_hardware(device);
            break;
        default:
            log_error("%s: Function % cannot detect command %d", TAG, __func__, priv_data->message.command);
            break;
    }
}


static void
handle_response(blynk_device_t* device) {
    blynk_private_data_t* private_data = &device->priv_data;

    for (uint16_t i = 0; i < BLYNK_MAX_AWAITING; ++i) {
        if (private_data->awaiting[i].id && private_data->awaiting[i].id == private_data->message.id) {
            if (private_data->awaiting[i].handler)
                private_data->awaiting[i].handler(device, private_data->message.length, private_data->awaiting[i].data);

            private_data->awaiting[i].id = 0;
            return;
        }
    }
}


static void
handle_hardware(blynk_device_t* device) {
    blynk_private_data_t* priv_data = &device->priv_data;
    char* extracted_args[BLYNK_MAX_ARGS];

    int32_t arg_count = extract_args_from_payload(priv_data, extracted_args);
    if (arg_count > 0) {
        process_hardware_message(device, extracted_args, arg_count);
    }
}


static int32_t
extract_args_from_payload(blynk_private_data_t* private_data, char* args[]) {
    if (private_data->message.length <= 0) return 0;

    uint32_t len = MIN(private_data->message.length, sizeof(private_data->message.payload) - 1);
    return split_payload_into_args((char*) private_data->message.payload, len, args, BLYNK_MAX_ARGS);
}


static int32_t
split_payload_into_args(char* payload, uint32_t len, char** args, uint32_t size) {
    char* p = payload;
    int32_t arg_count = 0;

    while (arg_count < size && len > 0) {
        args[arg_count++] = p;

        while (len > 0 && *p != '\0') {
            p++;
            len--;
        }

        if (len > 0) {
            p++;
            len--;
        }
    }

    return arg_count;
}


static void
process_hardware_message(blynk_device_t* device, char* args[], int32_t args_num) {
    blynk_control_t* ctl = &device->control;
    blynk_private_data_t* private_data = &device->priv_data;

    mutex_wrap_t state_mtx = {
            .type = MUTEX_TYPE_FREERTOS,
            .mutex = {.freertosMtx = ctl->mtx}
    };

    mutex_wrapper_take(&state_mtx);
    blynk_cmd_handler_t handler = find_handler_for_command(ctl, args[0]);
    mutex_wrapper_give(&state_mtx);

    if (handler != NULL) {
        blynk_handler_params_t params = {
                .device = device,
                .id = private_data->message.id,
                .argv = args + 1,
                .command = args[0],
                .argc = args_num - 1,
                .data = ctl->handlers[0].data

        };

        handler(&params);
        return;
    }

    blynk_err_t status_code = handle_hardware_package(device);
    if (status_code != BLYNK_EC_OK) {
        disconnect_device(device, status_code, status_code == BLYNK_EC_ERRNO ? errno : 0);
    }
}


static blynk_cmd_handler_t
find_handler_for_command(blynk_control_t* ctl, char* command) {
    for (uint16_t i = 0; i < BLYNK_MAX_HANDLERS; ++i) {
        if (!strncmp((const char*) ctl->handlers[i].action, command, sizeof(ctl->handlers[i].action))
            && ctl->handlers[i].handler != NULL) {
            return ctl->handlers[i].handler;
        }
    }
    return NULL;
}


static blynk_err_t
handle_hardware_package(blynk_device_t* device) {
    blynk_packet_t packet = {
            .device = device,
            .cmd = BLYNK_CMD_RESPONSE,
            .id = device->priv_data.message.id,
            .len = BLYNK_STATUS_ILLEGAL_COMMAND,
            .payload = NULL,
            .handler = NULL,
            .data = NULL,
            .wait = 0,
    };

    return blynk_notify_packet_ready(&packet);
}
