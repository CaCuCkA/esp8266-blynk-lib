#include "setup_uart.h"

#define UART_PORT              0
#define UART_BAUD_RATE         115200
#define UART_QUEUE_SIZE        4096
#define UART_RX_BUFFER_SIZE    256
#define UART_TX_BUFFER_SIZE    512


void
initialize_uart(QueueHandle_t* queue) {
    uart_config_t uart_config = {
            .baud_rate = UART_BAUD_RATE,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config))

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, UART_RX_BUFFER_SIZE * 2,
                                        UART_TX_BUFFER_SIZE, UART_QUEUE_SIZE, queue, 0))
}


void
accept_uart_message(uint8_t* sign, uint32_t length) {
    uart_read_bytes(UART_PORT, sign, length, 0);
}
