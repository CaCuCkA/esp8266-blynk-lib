#include "stuff/util.h"
#include "internal/packet_handler.h"
#include "internal/protocol_parser.h"

#define TAG "[PROTOCOL PARSER]"


typedef void (* parser_func_t)(blynk_device_t*, uint8_t);

typedef enum {
    PARSE_CMD, PARSE_ID, PARSE_LEN, PARSE_PAYLOAD
} parser_state_t;


static void parse_msg_id(blynk_device_t* device, uint8_t message_frame);

static void parse_msg_cmd(blynk_device_t* device, uint8_t message_frame);

static void parse_msg_len(blynk_device_t* device, uint8_t message_frame);

static void parse_msg_payload(blynk_device_t* device, uint8_t message_frame);


static parser_state_t next_state = PARSE_CMD;

static parser_func_t parsers[] = {
        parse_msg_cmd,
        parse_msg_id,
        parse_msg_len,
        parse_msg_payload
};


void
message_payload_parser(blynk_device_t* device, uint8_t message_frame) {
    if (CHECK_PTR(TAG, device)) return;
    parsers[next_state](device, message_frame);
}


static void
parse_msg_cmd(blynk_device_t* device, uint8_t message_frame) {
    device->priv_data.message.command = message_frame;
    device->priv_data.byte_count = 0;
    next_state = PARSE_ID;
}


static void
parse_msg_id(blynk_device_t* device, uint8_t message_frame) {
    device->priv_data.message.id = (device->priv_data.message.id << 8) | message_frame;
    device->priv_data.byte_count++;
    if (device->priv_data.byte_count >= 2) {
        device->priv_data.byte_count = 0;
        next_state = PARSE_LEN;
    }
}

static void
parse_msg_len(blynk_device_t* device, uint8_t message_frame) {
    blynk_message_t* message = &device->priv_data.message;

    message->length = (message->length << 8) | message_frame;
    device->priv_data.byte_count++;

    if (device->priv_data.byte_count < 2) return;

    if (message->command != BLYNK_CMD_RESPONSE && message->length > 0) {
        device->priv_data.byte_count = 0;
        next_state = PARSE_PAYLOAD;
    } else {
        handle_message_packet(device);
        next_state =  PARSE_CMD;
    }
}


static void
parse_msg_payload(blynk_device_t* device, uint8_t message_frame) {
    blynk_private_data_t* priv_data = &device->priv_data;
    uint8_t* payload = priv_data->message.payload;

    if (priv_data->byte_count < sizeof(priv_data->message.payload)) {
        payload[priv_data->byte_count] = message_frame;
    }

    priv_data->byte_count++;

    if (priv_data->byte_count >= priv_data->message.length) {
        if (priv_data->byte_count > sizeof(priv_data->message.payload)) {
            priv_data->message.length = sizeof(priv_data->message.payload);
        }
        handle_message_packet(device);
        next_state = PARSE_CMD;
    }
}
