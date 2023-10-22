/**
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef ESP8266_BLYNK_LIB_LOG_H
#define ESP8266_BLYNK_LIB_LOG_H

#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>


typedef struct {
    va_list args;
    const char* format;
    const char* file_name;
    struct tm* timestamp;
    void* userdata;
    int32_t line_num;
    int32_t log_level;
} log_event_t;

typedef enum {
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL,
} log_status_t;

typedef void (* log_func)(log_event_t* event);

typedef void (* lock_func)(bool should_lock, void* user_data);


#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)


/**
 * Logs a message based on the provided log level, file name, line number, and format string.
 *
 * This function first checks the global logging configuration to determine whether the
 * message should be printed or if it should be passed to any registered callbacks.
 * If printing to standard output, the message may include a timestamp, log level,
 * file name, and line number as a prefix. The actual log message is constructed using
 * the format string and variable arguments similar to the printf family of functions.
 *
 * @param level     The log level of the message (e.g., DEBUG, INFO, WARN, etc.)
 * @param file_name The name of the file where the log call originated.
 * @param code_line The line number in the file where the log call originated.
 * @param fmt       A format string for the log message (similar to printf).
 * @param ...       Variable arguments to be inserted into the format string.
 *
 * @note
 * The function uses locks to ensure thread-safety when accessing shared resources.
 * It is also capable of coloring output when the LOG_USE_COLOR macro is defined.
 */
void log_log(int32_t level, const char* file_name, int32_t code_line, const char* fmt, ...);

#endif //ESP8266_BLYNK_LIB_LOG_H
