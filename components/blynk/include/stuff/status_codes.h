/*
 * MIT License - CaCuCkA (2023)
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee
 * is hereby granted, provided the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY. See the full MIT License for details.
 */

#ifndef ESP8266_BLYNK_LIB_STATUS_CODES_H
#define ESP8266_BLYNK_LIB_STATUS_CODES_H

typedef enum {
    BLYNK_STATUS_SUCCESS                = 200,
    BLYNK_STATUS_QUOTA_LIMIT_EXCEPTION  = 1,
    BLYNK_STATUS_ILLEGAL_COMMAND        = 2,
    BLYNK_STATUS_NOT_REGISTERED         = 3,
    BLYNK_STATUS_ALREADY_REGISTERED     = 4,
    BLYNK_STATUS_NOT_AUTHENTICATED      = 5,
    BLYNK_STATUS_NOT_ALLOWED            = 6,
    BLYNK_STATUS_DEVICE_NOT_IN_NETWORK  = 7,
    BLYNK_STATUS_NO_ACTIVE_DASHBOARD    = 8,
    BLYNK_STATUS_INVALID_TOKEN          = 9,
    BLYNK_STATUS_ILLEGAL_COMMAND_BODY   = 11,
    BLYNK_STATUS_GET_GRAPH_DATA_EXCEPTION = 12,
    BLYNK_STATUS_NO_DATA_EXCEPTION      = 17,
    BLYNK_STATUS_DEVICE_WENT_OFFLINE    = 18,
    BLYNK_STATUS_SERVER_EXCEPTION       = 19,

    BLYNK_STATUS_NTF_INVALID_BODY       = 13,
    BLYNK_STATUS_NTF_NOT_AUTHORIZED     = 14,
    BLYNK_STATUS_NTF_ECXEPTION          = 15,

    BLYNK_STATUS_TIMEOUT                = 16,

    BLYNK_STATUS_NOT_SUPPORTED_VERSION  = 20,
    BLYNK_STATUS_ENERGY_LIMIT           = 21,

    BLYNK_STATUS_OPERATION_ACCEPT       = 23,
    BLYNK_STATUS_OPERATION_DECLINE      = 24,
} blynk_status_t;


typedef enum {
    BLYNK_STATE_STOPPED = 0,
    BLYNK_STATE_DISCONNECTED,
    BLYNK_STATE_CONNECTED,
    BLYNK_STATE_AUTHENTICATED,
} blynk_state_t;


typedef enum {
    BLYNK_CMD_RESPONSE       = 0,

    BLYNK_CMD_PING           = 6,

    BLYNK_CMD_BRIDGE         = 15,
    BLYNK_CMD_HARDWARE_SYNC  = 16,
    BLYNK_CMD_INTERNAL       = 17,
    BLYNK_CMD_PROPERTY       = 19,
    BLYNK_CMD_HARDWARE       = 20,
    BLYNK_CMD_GROUP          = 21,

    BLYNK_CMD_LOGIN       = 29,

    BLYNK_CMD_REDIRECT       = 41,
    BLYNK_CMD_DEBUG_PRINT    = 55,
    BLYNK_CMD_EVENT_LOG      = 64,
    BLYNK_CMD_EVENT_CLEAR    = 65
} blynk_cmd_t;

#endif //ESP8266_BLYNK_LIB_STATUS_CODES_H
