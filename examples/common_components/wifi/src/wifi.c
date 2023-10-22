#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <string.h>

#include "wifi.h"


#define TAG                 "[WIFI]"
#define WIFI_FAIL_BIT       BIT1
#define WIFI_CONNECTED_BIT  BIT0
#define MAX_AMOUNT_OF_TRIES (10)

#define UNUSED __attribute__((unused))


typedef enum {
    WIFI_EC_OK,
    WIFI_EC_UNSUPPORTED_EVENT,
} wifi_exception_t;


static void init_tcpip();

static void create_event_group();

static void handle_sta_start_event();

static void register_default_event_loop();

static void cleanup_wifi_initialization();

static void register_wifi_event_handlers();

static void handle_sta_disconnected_event();

static void init_wifi(wifi_init_config_t* cfg);

static void handle_got_ip_event(void* event_data);

static void start_wifi_station_mode(wifi_config_t* wifi_config);

static wifi_config_t get_wifi_config(const char* ssid, const char* password);

static uint8_t handle_connection_events(const char* ssid, const char* password);

static void wifi_event_handler(UNUSED void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);


static int s_retry_num = 0;

static EventGroupHandle_t s_wifi_event_group;


uint8_t initialize_wifi(const char* ssid, const char* password) {
    create_event_group();
    init_tcpip();
    register_default_event_loop();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT()
    init_wifi(&cfg);

    register_wifi_event_handlers();

    wifi_config_t wifi_config = get_wifi_config(ssid, password);
    start_wifi_station_mode(&wifi_config);

    printf("%s: Function %s finished wifi initialization\n", TAG, __func__);

    uint8_t result = handle_connection_events(ssid, password);

    return result;
}


static void
create_event_group() {
    s_wifi_event_group = xEventGroupCreate();
}


static void
init_tcpip() {
    tcpip_adapter_init();
}


static void
register_default_event_loop() {
    ESP_ERROR_CHECK(esp_event_loop_create_default())
}


static void
init_wifi(wifi_init_config_t* cfg) {
    ESP_ERROR_CHECK(esp_wifi_init(cfg))
}


static void
register_wifi_event_handlers() {
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL))
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL))
}


static void
wifi_event_handler(__attribute__((unused)) void* arg, esp_event_base_t event_base,
                   int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                handle_sta_start_event();
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                handle_sta_disconnected_event();
                break;
            default:
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        handle_got_ip_event(event_data);
    }
}


static void
handle_sta_start_event() {
    esp_wifi_connect();
}


static void
handle_sta_disconnected_event() {
    if (s_retry_num < MAX_AMOUNT_OF_TRIES) {
        ++s_retry_num;
        esp_wifi_connect();
        printf("%s: Function %s retry to connect to the wifi\n", TAG, __func__);
    } else {
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        printf("%s: Function %s failed to connect to the wifi\n", TAG, __func__);
    }
}


static void
handle_got_ip_event(void* event_data) {
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    printf("%s: Function %s got ip: %s\n", TAG, __func__, ip4addr_ntoa(&event->ip_info.ip));
    s_retry_num = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
}


static wifi_config_t
get_wifi_config(const char* ssid, const char* password) {
    wifi_config_t wifi_config = {0};

    strncpy((char*) wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*) wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);

    if (strlen(password)) {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }

    return wifi_config;
}


static void
start_wifi_station_mode(wifi_config_t* wifi_config) {
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA))
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config))
    ESP_ERROR_CHECK(esp_wifi_start())
}


static uint8_t
handle_connection_events(const char* ssid, const char* password) {
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        printf("%s: Function %s successfully connected to wifi. SSID: %s, password: %s\n", TAG, __func__,
                 ssid, password);
        return WIFI_EC_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        printf("%s: Function %s failed to connect to wifi. SSID: %s, password: %s\n", TAG, __func__, ssid,
                  password);
    } else {
        printf("%s: Function %s accepted unexpected event\n", TAG, __func__);
    }

    cleanup_wifi_initialization();

    return WIFI_EC_UNSUPPORTED_EVENT;
}


static void
cleanup_wifi_initialization() {
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler))
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler))
    vEventGroupDelete(s_wifi_event_group);
}
