#ifndef ESP8266_BLYNK_LIB_DEFINES_H
#define ESP8266_BLYNK_LIB_DEFINES_H

// types.h
#define BLYNK_MAX_ARGS                  32
#define BLYNK_ACTION_SIZE               5
#define BLYNK_MAX_URL_SIZE              2048
#define BLYNK_MAX_HANDLERS              8
#define BLYNK_MAX_AWAITING              32
#define BLYNK_AUTH_TOKEN_SIZE           64
#define BLYNK_MAX_PAYLOAD_LEN           512


// connection.h
#define UDP_PROTOCOL                    0
#define URL_DELIMITER                   ':'
#define HOSTNAME_SIZE                   128
#define IPV4_LOCALHOST                  0x7f000001
#define CONNECTION_FAILED               (-1)
#define IS_SOCKET_INVALID(fd)           ((fd) < 0)

// internal communication
#define WRITE_SOCK                      1
#define SYSCALL_FAILED(call)            (call < 0)

// packet handler
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// internal
#define NO_WAITING                      0
#define NOT_EMPTY(c)                    ((c) != 0)
#define NO_CALLBACK_DATA                NULL
#define LOGIN_RESPONSE_ID               0
#define CURRENT_TASK_HANDLE             NULL

// protocol stuff
#define BYTE_SIZE                       8
#define MS_TO_SEC                       1000
#define BYTE_MASK                       0xFF
#define MS_TO_USEC                      1000
#define READ_SOCK_ID                    0
#define WRITE_SOCK_ID                   1
#define AUTO_ASSIGN_PORT                0

// dispatching.c
#define BLYNK_HEADER_SIZE               5
#define FORMAT_BUFFER_SIZE              32
#define BLYNK_DEVICE_IS_VALID(device_ptr) ((device_ptr) && (device_ptr)->valid)


// blynk.c
#define QUEUE_SIZE                      5
#define NO_ACTION                       '\0'
#define NO_HANDLERS                     NULL
#define DEFAULT_TIMEOUT                 5000
#define BLYNK_STACK_SIZE                8000
#define DEFAULT_CLOUD_PORT              "8080"
#define DEFAULT_CLOUD_URL               "blynk.cloud"
#define DEFAULT_HEARTBEAT_INTERVAL      2000
#define DEFAULT_RECONNECT_DELAY         5000

// blynk_freertos_port.c
#define BLYNK_TASK_PRIORITY             4

#endif //ESP8266_BLYNK_LIB_DEFINES_H
