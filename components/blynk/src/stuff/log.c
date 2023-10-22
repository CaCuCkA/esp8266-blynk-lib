/*
 * Copyright (c) 2020 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "stuff/log.h"

#define MAX_CALLBACKS       32
#define TIME_BUFFER_SIZE    16


typedef struct {
    log_func func;
    void* user_data;
    int32_t log_level;
} callback_t;

static struct {
    void* user_data;
    lock_func lock;
    int32_t log_level;
    bool quiet;
    callback_t callbacks[MAX_CALLBACKS];
} log_config;

static const char* log_levels[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char* level_colors[] = {
        "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif


static void lock_function(void);

static void unlock_function(void);

static void stdout_callback_function(log_event_t* event);

static void init_event_function(log_event_t* event, void* user_data);


void
log_log(int32_t level, const char* file_name, int32_t code_line, const char* fmt, ...) {
    log_event_t log_event = {
            .format = fmt,
            .file_name = file_name,
            .line_num = code_line,
            .log_level = level,
    };

    lock_function();

    if (!log_config.quiet && level >= log_config.log_level) {
        init_event_function(&log_event, stderr);
        va_start(log_event.args, fmt);
        stdout_callback_function(&log_event);
        va_end(log_event.args);
    }

    for (int32_t i = 0; i < MAX_CALLBACKS && log_config.callbacks[i].func; ++i) {
        callback_t* callback = &log_config.callbacks[i];
        if (level >= callback->log_level) {
            init_event_function(&log_event, callback->user_data);
            va_start(log_event.args, fmt);
            callback->func(&log_event);
            va_end(log_event.args);
        }
    }

    unlock_function();
}


static void
lock_function(void) {
    if (log_config.lock) log_config.lock(true, log_config.user_data);
}


static void
unlock_function(void) {
    if (log_config.lock) log_config.lock(false, log_config.user_data);
}


static void
stdout_callback_function(log_event_t* event) {
    char buf[TIME_BUFFER_SIZE] = "";

#ifdef LOG_WITH_TIME
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", event->timestamp)] = '\0';
#endif

#ifdef LOG_USE_COLOR
    fprintf(
        event->userdata, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
        buf, level_colors[event->log_level], log_levels[event->log_level],
        event->file_name, event->line_num);
#else
    fprintf(
            event->userdata, "%s %-5s %s:%d: ",
            buf, log_levels[event->log_level], event->file_name, event->line_num);
#endif

    vfprintf(event->userdata, event->format, event->args);
    fprintf(event->userdata, "\n");
    fflush(event->userdata);
}


static void
init_event_function(log_event_t* event, void* user_data) {
    if (!event->timestamp) {
        time_t t = time(NULL);
        event->timestamp = localtime(&t);
    }
    event->userdata = user_data;
}
