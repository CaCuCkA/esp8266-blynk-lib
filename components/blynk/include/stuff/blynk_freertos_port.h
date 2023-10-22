/*
 * MIT License - CaCuCkA (2023)
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee
 * is hereby granted, provided the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY. See the full MIT License for details.
 */

#ifndef ESP8266_BLYNK_LIB_FREERTOS_PORT_H
#define ESP8266_BLYNK_LIB_FREERTOS_PORT_H

#if FREERTOS

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#endif

// FreeRTOS's system types
typedef void* queue_t;
typedef uint32_t tick_t;
typedef void* task_handle_t;
typedef void* semaphore_handle_t;

typedef void (* task_func_t)(void*);


#if FREERTOS
extern const tick_t custom_port_tick_max_rate;
#else
// Replace with your non-FreeRTOS implementation
#endif

typedef enum {
    MUTEX_TYPE_FREERTOS,
    // Add other types as needed
} mutex_t;


typedef union {
    semaphore_handle_t freertosMtx;
    // Add other mutex types here as needed
} mutex_u;


typedef struct {
    mutex_t type;
    mutex_u mutex;
} mutex_wrap_t;


tick_t get_tick_count(void);

void task_delay(tick_t ticks);

bool queue_reset(queue_t queue);

void task_delete(task_handle_t task);

tick_t ms_to_ticks(uint32_t milliseconds);

semaphore_handle_t create_semaphore(void);

bool mutex_wrapper_take(mutex_wrap_t* wrap);

bool mutex_wrapper_give(mutex_wrap_t* wrap);

queue_t create_queue(size_t queue_length, size_t element_size);

bool queue_receive(queue_t queue, void* item, tick_t timeout_ms);

bool queue_send(queue_t queue, const void* item, tick_t timeout_ms);

bool create_task(const char* task_name, task_func_t task_function, void* task_parameters, uint16_t stack_size,
                 task_handle_t* task_handle);

#endif //BLYNK_FREERTOS_PORT_H
