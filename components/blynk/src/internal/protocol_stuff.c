#include <memory.h>

#include "stuff/log.h"
#include "internal/protocol_stuff.h"
#include "stuff/blynk_freertos_port.h"

#define TAG "[PROTOCOL STUFF]"


void
update_device_communication_state(blynk_device_t* device, blynk_state_t state) {
    blynk_state_event_t event = {
            .conn_status = state,
    };

    mutex_wrap_t wrap = {
            .type = MUTEX_TYPE_FREERTOS,
            .mutex = {device->control.mtx},
    };

    mutex_wrapper_take(&wrap);
    device->control.state = state;
    blynk_state_handler_t handler = device->control.on_state_change;
    void* data = device->control.callback_user_data;
    mutex_wrapper_give(&wrap);

    if (handler) handler(device, &event, data);
}


void
disconnect_device(blynk_device_t* device, blynk_err_t reason, int code) {
    blynk_state_event_t event = {
            .conn_status = BLYNK_STATE_DISCONNECTED,
            .disconnected = {
                    .reason = reason,
                    .code = code,
            }
    };

    mutex_wrap_t wrap = {
            .type = MUTEX_TYPE_FREERTOS,
            .mutex = {device->control.mtx},
    };

    mutex_wrapper_take(&wrap);
    device->control.state = BLYNK_STATE_DISCONNECTED;
    blynk_state_handler_t handler = device->control.on_state_change;
    void* data = device->control.callback_user_data;
    mutex_wrapper_give(&wrap);

    log_error("%s: Function %s set disconnected state, reason: %d, code: %d", TAG, __func__, reason, code);

    if (handler) handler(device, &event, data);
}


uint64_t
compose_blynk_message(uint8_t* output_buffer, uint64_t max_size, const blynk_message_t* message) {
    // | command 1 byte | message_id 2 bytes | length 2 bytes |
    output_buffer[0] = message->command;
    output_buffer[1] = (message->id >> BYTE_SIZE) & BYTE_MASK;
    output_buffer[2] = message->id & BYTE_MASK;
    output_buffer[3] = (message->length >> BYTE_SIZE) & BYTE_MASK;
    output_buffer[4] = message->length & BYTE_MASK;

    uint64_t header_size = BLYNK_HEADER_SIZE;

    if (message->length && message->command != BLYNK_CMD_RESPONSE) {
        uint64_t payload_size = message->length;

        if (payload_size > max_size - header_size) {
            payload_size = max_size - header_size;
        }

        memcpy(output_buffer + header_size, message->payload, payload_size);
        header_size += payload_size;
    }

    return header_size;
}


uint16_t
allocate_request_id(blynk_device_t* device, tick_t deadline, blynk_response_handler_t handler, void* context) {
    if (device->priv_data.request_id == UINT16_MAX) {
        device->priv_data.request_id = 0;
        memset(device->priv_data.awaiting, 0, sizeof(device->priv_data.awaiting));
    }

    uint16_t request_id = device->priv_data.request_id++;

    if (!handler) return request_id;

    for (uint16_t i = 0; i < BLYNK_MAX_AWAITING; i++) {
        if (!device->priv_data.awaiting[i].id) {
            device->priv_data.awaiting[i].handler = handler;
            device->priv_data.awaiting[i].id = request_id;
            device->priv_data.awaiting[i].data = context;
            device->priv_data.awaiting[i].deadline = deadline;
            return request_id;
        }
    }

    return 0;
}
