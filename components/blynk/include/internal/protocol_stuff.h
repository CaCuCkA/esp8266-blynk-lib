#ifndef ESP8266_BLYNK_LIB_PROTOCOL_STUFF_H
#define ESP8266_BLYNK_LIB_PROTOCOL_STUFF_H

#include "stuff/types.h"


/**
 * @brief Update the communication state of a Blynk device.
 *
 * This function updates the internal communication state of a Blynk device
 * and invokes a registered state change handler, if present.
 *
 * @param device Pointer to the Blynk device structure.
 * @param state The new communication state for the device.
 */
void update_device_communication_state(blynk_device_t* device, blynk_state_t state);


/**
 * @brief Disconnect a Blynk device and log the reason.
 *
 * This function updates the internal state of a Blynk device to DISCONNECTED,
 * logs the reason and error code for the disconnection, and invokes a registered
 * state change handler, if present.
 *
 * @param device Pointer to the Blynk device structure.
 * @param reason The Blynk error code representing the reason for disconnection.
 * @param code An integer error code providing additional details about the disconnection.
 */
void disconnect_device(blynk_device_t* device, blynk_err_t reason, int code);


/**
 * @brief Compose a Blynk protocol message.
 *
 * This function serializes a given Blynk message structure into a byte buffer
 * suitable for transmission over the network.
 *
 * @param output_buffer A buffer where the serialized message should be stored.
 * @param max_size The maximum size of the output_buffer.
 * @param message Pointer to the Blynk message structure to be serialized.
 * @return The size in bytes of the serialized message.
 */
uint64_t compose_blynk_message(uint8_t* output_buffer, uint64_t max_size, const blynk_message_t* message);


/**
 * @brief Allocate a unique request ID for Blynk communication.
 *
 * This function provides a unique request ID for communication with the Blynk platform.
 * If a response handler is provided, the function also registers it for later invocation
 * when a corresponding response is received.
 *
 * @param device Pointer to the Blynk device structure.
 * @param deadline The system tick time after which the request can be considered as timed out.
 * @param handler The response handler function to be invoked upon receiving a corresponding response.
 * @param context An optional context pointer to be passed to the handler when invoked.
 * @return A unique request ID, or 0 if allocation fails.
 */
uint16_t allocate_request_id(blynk_device_t* device, tick_t deadline, blynk_response_handler_t handler, void* context);

#endif //ESP8266_BLYNK_LIB_PROTOCOL_STUFF_H
