#include <setup_uart.h>
#include <blynk.h>
#include <wifi.h>

#define WAIT               10
#define WIFI_SSID          "wifi_SSID"
#define AUTH_TOKEN         "YourAuthToken"
#define WIFI_PASSWORD      "wifi_PASSWORD"


static int
get_temperature() {
    return 25;
}


static int
get_light_intensity() {
    return 70;
}


static void
vr_handler(blynk_handler_params_t* params) {
    if (params->argc > 0) {
        char* pend;
        int pin = strtol(params->argv[0], &pend, 10);

        switch (pin) {
            case 1: {
                int temperature = get_temperature();
                blynk_send(params->device, BLYNK_CMD_HARDWARE, WAIT, "sii", "vw", pin, temperature);
                break;
            }
            case 2: {
                int light_intensity = get_light_intensity();
                blynk_send(params->device, BLYNK_CMD_HARDWARE, WAIT, "sii", "vw", pin, light_intensity);
                break;
            }
            default:
                printf("No handler for virtual pin V%d\n", pin);
                break;
        }
    } else {
        printf("VR command received with no arguments\n");
    }
}


void
app_main() {
    initialize_uart(NULL);
    initialize_wifi(WIFI_SSID, WIFI_PASSWORD);
    blynk_device_t* device = malloc(sizeof(blynk_device_t));
    blynk_begin(device, AUTH_TOKEN);
    blynk_register_cmd_handler(device, "vr", vr_handler, NULL);

    blynk_run(device);
}
