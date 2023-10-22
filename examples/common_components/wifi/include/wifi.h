#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>


/**
 * @brief Initializes the WiFi subsystem.
 *
 * This function sets up and initializes the necessary components for the WiFi subsystem.
 * It carries out the following steps:
 * 1. Creates an event group to handle WiFi-related events.
 * 2. Initializes the TCP/IP stack.
 * 3. Registers the default event loop for the WiFi driver.
 * 4. Initializes the WiFi subsystem with the default configuration.
 * 5. Registers event handlers to manage various WiFi events.
 * 6. Retrieves the WiFi configuration and starts the WiFi in station mode.
 * 7. Logs the completion of the initialization process.
 * 8. Handles WiFi connection events and returns the resulting status.
 *
 * @return Status of the WiFi connection. The value indicates whether the initialization
 * and subsequent connection procedures were successful.
 */
uint8_t initialize_wifi(const char* ssid, const char* password);

#endif //WIFI_H
