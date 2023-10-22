#ifndef ESP8266_BLYNK_LIB_PROTOCOL_PARSER_H
#define ESP8266_BLYNK_LIB_PROTOCOL_PARSER_H

#include "stuff/types.h"


/**
 * @brief Main message parser for Blynk messages.
 *
 * This function dispatches a message frame to the appropriate parser function based on the
 * current state of the message parsing process.
 *
 * @param device Pointer to the Blynk device structure containing the message.
 * @param message_frame The current byte/frame of the message to be parsed.
 */
void message_payload_parser(blynk_device_t* device, uint8_t message_frame);

#endif //ESP8266_BLYNK_LIB_PROTOCOL_PARSER_H
