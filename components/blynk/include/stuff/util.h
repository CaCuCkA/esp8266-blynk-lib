#ifndef ESP8266_BLYNK_LIB_UTIL_H
#define ESP8266_BLYNK_LIB_UTIL_H

#include <stdint.h>
#include <stdio.h>

#include "log.h"
#include "exceptions.h"

// Utility macro to get the size of an array
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define UNUSED __attribute__((unused))
#define FALLTHROUGH __attribute__((fallthrough))

// Macros for checking pointers
#define CHECK_PTR(TAG, ...) \
    check_ptrs(TAG, __VA_ARGS__, NULL)


static uint8_t
check_ptr(const void* ptr, const char* TAG, const char* function_name) {
    if (ptr == NULL) {
        log_fatal("%s: Function %s accepted null pointer!", TAG, function_name);
        return PTR_EC_NULL;
    }
    return PTR_EC_OK;
}


UNUSED static uint8_t
check_ptrs(const char* TAG, const void* ptr, ...) {
    va_list args;
    va_start(args, ptr);

    while (ptr != NULL) {
        if (check_ptr(ptr, TAG, __func__) != PTR_EC_OK) {
            va_end(args);
            return PTR_EC_NULL;
        }
        ptr = va_arg(args, void*);
    }

    va_end(args);
    return PTR_EC_OK;
}

#endif //ESP8266_BLYNK_LIB_UTIL_H
