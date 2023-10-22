#ifndef ESP8266_BLYNK_LIB_TYPES_H
#define ESP8266_BLYNK_LIB_TYPES_H

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <stdbool.h>
#include <stdint.h>

#include "defines.h"
#include "exceptions.h"
#include "status_codes.h"
#include "blynk_freertos_port.h"


typedef struct blynk_device blynk_device_t;
typedef struct blynk_config blynk_config_t;
typedef struct blynk_packet blynk_packet_t;
typedef struct blynk_message blynk_message_t;
typedef struct blynk_control blynk_control_t;
typedef struct blynk_awaiting blynk_awaiting_t;
typedef struct blynk_state_event blynk_state_event_t;
typedef struct blynk_private_data blynk_private_data_t;
typedef struct blynk_handler_data blynk_handler_data_t;
typedef struct blynk_request_info blynk_request_info_t;
typedef struct blynk_server_config blynk_server_config_t;
typedef struct blynk_handler_params blynk_handler_params_t;
typedef struct blynk_connection_settings blynk_connection_settings_t;

// Function pointers
typedef void (* blynk_command_parser_t)(blynk_device_t*, uint8_t);

typedef void (* blynk_response_handler_t)(blynk_device_t*, blynk_status_t, void*);

typedef void (* blynk_state_handler_t)(blynk_device_t*, const blynk_state_event_t*, void*);

typedef void (* blynk_cmd_handler_t)(blynk_handler_params_t* params);


struct blynk_message {
    uint8_t command;
    uint16_t length;
    uint16_t id;
    uint8_t payload[BLYNK_MAX_PAYLOAD_LEN];
};


struct blynk_handler_data {
    char action[BLYNK_ACTION_SIZE];
    blynk_cmd_handler_t handler;
    void* data;
};


struct blynk_server_config {
    char server_url[BLYNK_MAX_URL_SIZE];
    char auth_token[BLYNK_AUTH_TOKEN_SIZE];
};


struct blynk_connection_settings {
    uint32_t connection_timeout_ms;
    uint32_t heartbeat_interval_ms;
    uint32_t reconnection_interval_ms;
};


struct blynk_config {
    blynk_server_config_t server;
    blynk_connection_settings_t connection;
};


struct blynk_awaiting {
    uint16_t id;
    tick_t deadline;
    blynk_response_handler_t handler;
    void* data;
};


struct blynk_state_event {
    blynk_state_t conn_status;
    struct {
        int code;
        blynk_err_t reason;
    } disconnected;
};


struct blynk_private_data {
    int ctl_sockets[2];
    queue_t ctl_queue;
    uint16_t request_id;
    uint16_t byte_count;

    blynk_command_parser_t parser;

    blynk_message_t message;
    blynk_awaiting_t awaiting[BLYNK_MAX_AWAITING];
    tick_t heartbit_deadline;
    uint8_t read_buffer[BLYNK_MAX_PAYLOAD_LEN];
    uint8_t write_buffer[BLYNK_MAX_PAYLOAD_LEN];

    uint64_t buf_size;
    uint64_t total_byte_send;
};


struct blynk_control {
    task_handle_t task;
    semaphore_handle_t mtx;
    blynk_state_t state;
    blynk_config_t connection_config;
    blynk_state_handler_t on_state_change;
    void* callback_user_data;
    blynk_handler_data_t handlers[BLYNK_MAX_HANDLERS];
};


struct blynk_device {
    bool valid;
    blynk_control_t control;
    blynk_private_data_t priv_data;
};


struct blynk_request_info {
    blynk_message_t message;
    tick_t deadline;
    blynk_response_handler_t handler;
    void* data;
};


struct blynk_packet {
    blynk_device_t* device;
    uint8_t cmd;
    uint16_t id;
    uint16_t len;
    uint8_t* payload;
    blynk_response_handler_t handler;
    void* data;
    tick_t wait;
};


struct blynk_handler_params {
    blynk_device_t* device;
    uint16_t id;
    const char* command;
    int argc;
    char** argv;
    void* data;
};

#endif //ESP8266_BLYNK_LIB_TYPES_H
