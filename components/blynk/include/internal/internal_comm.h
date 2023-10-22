#ifndef ESP8266_BLYNK_LIB_INTERNAL_COMM_H
#define ESP8266_BLYNK_LIB_INTERNAL_COMM_H

#include "stuff/types.h"
#include "stuff/exceptions.h"


/**
 * @brief Notify that a packet is ready for processing.
 *
 * This function prepares a request information structure based on the provided packet
 * and then sends this structure to a control queue for further processing. If the packet
 * contains a payload, it copies the payload into the request information structure. After
 * the structure is sent to the control queue, the function sends a notification message
 * to the device's control socket to notify that a new packet is ready.
 *
 * @param packet Pointer to the packet that is ready for processing.
 * @return BLYNK_EC_OK if the notification was successful or an error code indicating the
 *         type of error that occurred.
 */
blynk_err_t blynk_notify_packet_ready(blynk_packet_t* packet);

#endif //ESP8266_BLYNK_LIB_INTERNAL_COMM_H
