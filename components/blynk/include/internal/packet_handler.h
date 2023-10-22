#ifndef ESP8266_BLYNK_LIB_PACKET_HANDLER_H
#define ESP8266_BLYNK_LIB_PACKET_HANDLER_H

#include "stuff/types.h"

/**
 * @brief Process incoming message packets based on their command type.
 *
 * This function examines the command in the received message packet and dispatches
 * the packet to the appropriate handler based on the command type.
 *
 * @param device Pointer to the Blynk device structure containing the packet to be processed.
 */
void handle_message_packet(blynk_device_t* device);

#endif //ESP8266_BLYNK_LIB_PACKET_HANDLER_H
