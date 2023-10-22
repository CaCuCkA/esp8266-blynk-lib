#ifndef SETUP_UART_H
#define SETUP_UART_H

#include <freertos/FreeRTOS.h>
#include <driver/uart.h>
#include <stdint.h>


/**
 * @brief Initializes the UART interface with specified configurations.
 *
 * Configures the UART interface based on the provided settings such as baud rate, data bits,
 * parity, stop bits, and flow control. Depending on the platform target, additional pin configurations
 * may be set. After setting the parameters, the UART driver is installed.
 *
 * @param queue Pointer to the queue handle, used for UART events and communications.
 */
void initialize_uart(QueueHandle_t* queue);


/**
 * @brief Accepts UART messages and reads bytes into the provided buffer.
 *
 * Reads specified length of bytes from the UART interface into the provided buffer.
 *
 * @param sign   Pointer to the buffer where the read bytes will be stored.
 * @param length Number of bytes to read from the UART interface.
 */
void accept_uart_message(uint8_t* sign, uint32_t length);

#endif //SETUP_UART_H
