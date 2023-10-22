/*
 * MIT License
 *
 * Copyright (c) 2023 CaCuCkA
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "defines.h"
#include "stuff/util.h"
#include "stuff/blynk_freertos_port.h"

#define TAG "[BLYNK FREERTOS PORT]"


#if FREERTOS
const tick_t custom_port_tick_max_rate = 1000 / configTICK_RATE_HZ;
#else
// Replace with your non-FreeRTOS implementation
#endif


static bool mutex_operation(mutex_wrap_t* wrap, bool is_take_operation);

static bool handle_freertos_mutex(mutex_wrap_t* wrap, bool is_take_operation);


bool
mutex_wrapper_take(mutex_wrap_t* wrap) {
    return mutex_operation(wrap, true);
}


bool
mutex_wrapper_give(mutex_wrap_t* wrap) {
    return mutex_operation(wrap, false);
}


static bool
mutex_operation(mutex_wrap_t* wrap, bool is_take_operation) {
    if (CHECK_PTR(TAG, wrap)) return false;

    switch (wrap->type) {
        case MUTEX_TYPE_FREERTOS:
            return handle_freertos_mutex(wrap, is_take_operation);
            // Handle other mutex types here as needed
        default:
            return false;
    }
}

#if FREERTOS
static bool
handle_freertos_mutex(mutex_wrap_t* wrap, bool is_take_operation) {
    if (is_take_operation) {
        return xSemaphoreTake(wrap->mutex.freertosMtx, portMAX_DELAY) == pdTRUE;
    } else {
        return xSemaphoreGive(wrap->mutex.freertosMtx) == pdTRUE;
    }
}
#endif


tick_t
get_tick_count(void) {
#if defined(FREERTOS)
    return xTaskGetTickCount();
#else
    return 0; // Replace with your non-FreeRTOS implementation
#endif
}


tick_t
ms_to_ticks(uint32_t milliseconds) {
#if defined(FREERTOS)
    return pdMS_TO_TICKS(milliseconds);
#else
    return 0; // Replace with your non-FreeRTOS implementation
#endif
}


bool
queue_send(queue_t queue, const void* item, tick_t timeout_ms) {
#if defined(FREERTOS)
    return xQueueSend(queue, item, timeout_ms) == pdTRUE;
#else
    return 0; // Replace with your non-FreeRTOS implementation
#endif
}


bool
queue_reset(queue_t queue) {
#ifdef FREERTOS
    return xQueueReset((QueueHandle_t) queue) == pdPASS;
#else
    // Replace with your non-FreeRTOS implementation
    return false;
#endif
}


bool
queue_receive(queue_t queue, void* item, tick_t timeout_ms) {
#ifdef FREERTOS
    return xQueueReceive(queue, item, pdMS_TO_TICKS(timeout_ms)) == pdPASS;
#else
    // Replace with your non-FreeRTOS implementation
    return false;
#endif
}


void
task_delete(task_handle_t task) {
#if defined(FREERTOS)
    return vTaskDelete(task);
#else
    return 0; // Replace with your non-FreeRTOS implementation
#endif
}


void
task_delay(tick_t ticks) {
#if defined(FREERTOS)
    return vTaskDelay(ticks);
#else
    return 0; // Replace with your non-FreeRTOS implementation
#endif
}


semaphore_handle_t
create_semaphore(void) {
    semaphore_handle_t semaphore;

#ifdef FREERTOS
    semaphore = xSemaphoreCreateMutex();
#elif defined(USING_OTHEROS)
    // Replace with your non-FreeRTOS implementation
    semaphore = OtherOS_CreateMutex();
#else
#error "OS not supported!"
#endif

    return semaphore;
}


bool
create_task(const char* task_name, task_func_t task_function, void* task_parameters, uint16_t stack_size,
            task_handle_t* task_handle) {
#ifdef FREERTOS
    return xTaskCreate(task_function, task_name, stack_size, task_parameters, BLYNK_TASK_PRIORITY, task_handle) ==
           pdPASS;
#elif defined(USING_OTHEROS)
    // Replace with your non-FreeRTOS implementation
    semaphore = OtherOS_CreateMutex();
#else
#error "OS not supported!"
#endif
}


queue_t
create_queue(size_t queue_length, size_t element_size) {
#ifdef FREERTOS
    return xQueueCreate(queue_length, element_size);
#elif defined(USING_OTHEROS)
    // Replace with your non-FreeRTOS implementation
    semaphore = OtherOS_CreateMutex();
#else
#error "OS not supported!"
#endif
}