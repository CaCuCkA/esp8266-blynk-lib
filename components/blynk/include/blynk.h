/*
 * MIT License - CaCuCkA (2023)
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee
 * is hereby granted, provided the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY. See the full MIT License for details.
 */

#ifndef ESP8266_BLYNK_LIB_H
#define ESP8266_BLYNK_LIB_H

#include "stuff/types.h"
#include "stuff/exceptions.h"


/**
 * Initializes a Blynk device using the provided authentication token.
 *
 * @param device Pointer to the device structure to be initialized.
 * @param authentication_token String containing the authentication token for the device.
 *
 * @return BLYNK_EC_OK on successful initialization, else appropriate error code.
 */
blynk_err_t blynk_begin(blynk_device_t* device, const char* authentication_token);


/**
 * Fetches the current state of the Blynk device.
 *
 * @param device Pointer to the device structure.
 *
 * @return Current state of the device.
 */
blynk_state_t blynk_get_device_state(blynk_device_t* device);


/**
 * Sends a Blynk request with a callback handler and a formatted payload.
 *
 * @param device Pointer to the device structure.
 * @param cmd Command code for the Blynk request.
 * @param handler Callback function to handle the response.
 * @param data Additional data for the handler.
 * @param wait Duration to wait for.
 * @param fmt Format string for the payload.
 * @param ... Additional arguments for formatting the payload.
 *
 * @return BLYNK_EC_OK on successful dispatch, else appropriate error code.
 */
blynk_err_t blynk_send_with_callback(blynk_device_t* device, uint8_t cmd, blynk_response_handler_t handler, void* data,
                                     tick_t wait, const char* fmt, ...);


/**
 * Sends a Blynk request with a formatted payload.
 *
 * @param device Pointer to the device structure.
 * @param cmd Command code for the Blynk request.
 * @param wait Duration to wait for.
 * @param fmt Format string for the payload.
 * @param ... Additional arguments for formatting the payload.
 *
 * @return Status code indicating the result of the operation.
 */
blynk_err_t blynk_send(blynk_device_t* device, blynk_cmd_t cmd, tick_t wait, const char* fmt, ...);


/**
 * Sends a response packet to Blynk.
 *
 * @param device Pointer to the device structure.
 * @param id ID of the response.
 * @param status Status code for the response.
 * @param wait Duration to wait for.
 *
 * @return BLYNK_EC_OK on successful dispatch, else appropriate error code.
 */
blynk_err_t blynk_send_response(blynk_device_t* device, uint16_t id, uint16_t status, tick_t wait);


/**
 * Updates the default timeout duration for the device.
 *
 * @param device Pointer to the device structure.
 * @param timeout New timeout duration.
 *
 * @return BLYNK_EC_OK on successful update, else appropriate error code.
 */
blynk_err_t update_default_timeout(blynk_device_t* device, tick_t timeout);


/**
 * Updates the heartbeat interval for the device.
 *
 * @param device Pointer to the device structure.
 * @param heartbit_interval New heartbeat interval duration.
 *
 * @return BLYNK_EC_OK on successful update, else appropriate error code.
 */
blynk_err_t update_heartbeat_interval(blynk_device_t* device, tick_t heartbit_interval);


/**
 * Updates the default reconnection delay for the device.
 *
 * @param device Pointer to the device structure.
 * @param reconnection_delay New reconnection delay duration.
 *
 * @return BLYNK_EC_OK on successful update, else appropriate error code.
 */
blynk_err_t update_default_reconnection_delay(blynk_device_t* device, tick_t heartbit_interval);


/**
 * @brief Updates the default state handler for the Blynk device.
 *
 * This function is a user-friendly wrapper for the internal function `blynk_set_state_handler`.
 * It allows the caller to set a state handler which will be triggered when the device state changes.
 *
 * @param device Pointer to the Blynk device structure.
 * @param handler Pointer to the function which will handle state changes.
 * @param user_data Pointer to any user data that will be passed to the state handler when it's triggered.
 *
 * @return BLYNK_EC_OK if successful, or an appropriate error code.
 */
blynk_err_t update_default_state_handler(blynk_device_t* device, blynk_state_handler_t handler, void* user_data);


/**
 * Registers a new command handler for a specific Blynk action.
 *
 * @param device Pointer to the device structure.
 * @param action Blynk action string.
 * @param handler Callback function to handle the specific action.
 * @param data Additional data for the handler.
 *
 * @return BLYNK_EC_OK on successful registration, else appropriate error code.
 */
blynk_err_t blynk_register_cmd_handler(blynk_device_t* device, const char* action, blynk_cmd_handler_t handler,
                                       void* data);


/**
 * Deregisters an existing command handler for a specific Blynk action.
 *
 * @param device Pointer to the device structure.
 * @param action Blynk action string.
 *
 * @return BLYNK_EC_OK on successful deregistration, else appropriate error code.
 */
blynk_err_t blynk_deregister_cmd_handler(blynk_device_t* device, const char* action);


/**
 * Starts a Blynk run task for the device, effectively establishing a connection to Blynk services.
 * Ensures that only one such task runs for the device at any time.
 *
 * @param device Pointer to the device structure.
 *
 * @return BLYNK_EC_OK on successful task creation, else appropriate error code.
 */
blynk_err_t blynk_run(blynk_device_t* device);

#endif // ESP8266_BLYNK_LIB_H
