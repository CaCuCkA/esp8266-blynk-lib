/*
 * MIT License - CaCuCkA (2023)
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee
 * is hereby granted, provided the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY. See the full MIT License for details.
 */

#ifndef ESP8266_BLYNK_LIB_EXCEPTIONS_H
#define ESP8266_BLYNK_LIB_EXCEPTIONS_H

typedef enum {
    PTR_EC_OK,
    PTR_EC_NULL
} ptr_err_t;

typedef enum {
    BLYNK_EC_OK = 0,
    BLYNK_EC_MEM,
    BLYNK_EC_GAI,
    BLYNK_EC_ERRNO,
    BLYNK_EC_STATUS,
    BLYNK_EC_SYSTEM,
    BLYNK_EC_CLOSED,
    BLYNK_EC_TIMEOUT,
    BLYNK_EC_RUNNING,
    BLYNK_EC_NULL_PTR,
    BLYNK_EC_SET_OPTIONS,
    BLYNK_EC_NOT_CONNECTED,
    BLYNK_EC_FAILED_TO_READ,
    BLYNK_EC_INVALID_OPTION,
    BLYNK_EC_FAILED_TO_WRITE,
    BLYNK_EC_NOT_INITIALIZED,
    BLYNK_EC_NOT_AUTHENTICATED,
    BLYNK_EC_DEVICE_DISCONNECT,
} blynk_err_t;


typedef enum {
    CONN_EC_OK = 0,
    CONN_EC_NULL_PTR,
    CONN_EC_GET_NAME,
    CONN_EC_SOCK_CONN,
    CONN_EC_SOCK_BIND,
    CONN_EC_CREATE_SOCK,
    CONN_EC_GET_ADDRINFO,
    CONN_EC_GET_HOST_AND_PORT,
    CONN_EC_FAILED_ESTALE_CONN,
} conn_err_t;

#endif //ESP8266_BLYNK_LIB_EXCEPTIONS_H
