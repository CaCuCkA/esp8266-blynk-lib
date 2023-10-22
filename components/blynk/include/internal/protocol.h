/*
 * MIT License - CaCuCkA (2023)
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee
 * is hereby granted, provided the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY. See the full MIT License for details.
 */

#ifndef ESP8266_BLYNK_LIB_PROTOCOL_H
#define ESP8266_BLYNK_LIB_PROTOCOL_H


/**
 * @brief Task responsible for running and maintaining Blynk's communication.
 *
 * This function is intended to be used as a task in a multitasking environment.
 * It continuously tries to maintain a connection to the Blynk server. It will
 * periodically attempt to reconnect based on the reconnection interval.
 *
 * @param pvParameters Expected to be a pointer to a blynk_device_t structure representing the device.
 */
void blynk_run_task(void* pvParameters);

#endif //ESP8266_BLYNK_LIB_PROTOCOL_H
