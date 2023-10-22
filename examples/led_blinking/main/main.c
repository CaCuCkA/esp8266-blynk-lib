#include <setup_uart.h>
#include <rom/gpio.h>
#include <blynk.h>
#include <gpio.h>
#include <wifi.h>

#define LED_PIN             GPIO_NUM_2
#define WIFI_SSID           "wifi_SSID"
#define AUTH_TOKEN          "YourAuthToken"
#define WIFI_PASSWORD       "wifi_PASSWORD"


static void
initialize_gpio() {
    gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
}


static void
vw_handler(blynk_handler_params_t* params) {
    if (params->argc > 1) {
        char* pend;
        int led_state = strtol(params->argv[1], &pend, 10);
        gpio_set_level(LED_PIN, led_state);
    }
}


void
app_main() {
    initialize_uart(NULL);
    initialize_gpio();
    initialize_wifi(WIFI_SSID, WIFI_PASSWORD);
    blynk_device_t* device = malloc(sizeof(blynk_device_t));
    blynk_begin(device, AUTH_TOKEN);
    blynk_register_cmd_handler(device, "vw", vw_handler, NULL);

    blynk_run(device);
}
