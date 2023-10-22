#ifndef ESP8266_BLYNK_LIB_DISPATCHING_H
#define ESP8266_BLYNK_LIB_DISPATCHING_H

#include "stuff/types.h"
#include "stuff/blynk_freertos_port.h"


/**
 *
 * @brief Dispatches a Blynk request with a formatted payload.
 *
 * The function prepares and sends a Blynk request, based on a set of provided arguments
 * and a format string derived from Python's struct format. This format string guides
 * the generation of the payload from the variadic arguments.
 *
 * Format Guide:
 *   - client:    char
 *   - b:    signed char integer
 *   - B:    unsigned char
 *   - ?:    bool
 *   - h:    short
 *   - H:    unsigned short
 *   - i:    int
 *   - I:    unsigned int
 *   - l:    long
 *   - L:    unsigned long
 *   - q:    long long
 *   - Q:    unsigned long long
 *   - f:    float
 *   - d:    double
 *   - s,p:  char*
 *
 * All callback functions are executed from the Blynk client task.
 *
 * @param device Pointer to the Blynk device structure.
 * @param command Blynk command to be executed.
 * @param handler Callback function to be triggered in response to the request.
 * @param data User-specific data to be passed to the callback function.
 * @param wait The duration to wait for the request to be processed.
 * @param format A format string for the payload, derived from Python's struct format.
 * @param args Variadic arguments corresponding to the format string.
 * @return Returns an error code indicating the result of the operation.
 */
blynk_err_t dispatch_blynk_request(blynk_device_t* device, blynk_cmd_t command, blynk_response_handler_t handler,
                                   void* data, tick_t wait, const char* format, va_list args);

#endif //ESP8266_BLYNK_LIB_DISPATCHING_H
