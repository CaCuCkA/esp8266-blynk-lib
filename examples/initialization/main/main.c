#include <setup_uart.h>
#include <blynk.h>
#include <wifi.h>

#define WIFI_SSID          "wifi_SSID"
#define AUTH_TOKEN         "YourAuthToken"
#define WIFI_PASSWORD      "wifi_PASSWORD"


void
app_main(void) {
    initialize_uart(NULL);
    initialize_wifi(WIFI_SSID, WIFI_PASSWORD);

    blynk_device_t* device = malloc(sizeof(blynk_device_t));
    blynk_begin(device, BLYNK_AUTH_TOKEN);

    blynk_run(device);
}
