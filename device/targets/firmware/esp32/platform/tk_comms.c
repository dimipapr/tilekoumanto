#include "tk_comms.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "wifi_secrets.h"

static const char *TAG = "tk_comms";

static int connected = 0;

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data)
{
    (void)arg;

    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START) {

        ESP_ERROR_CHECK(esp_wifi_connect());
    }

    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_DISCONNECTED) {

        wifi_event_sta_disconnected_t *event = event_data;

        connected = 0;

        ESP_LOGW(
            TAG,
            "Wi-Fi disconnected, reason=%d",
            event->reason
        );
    }

    if (event_base == IP_EVENT &&
        event_id == IP_EVENT_STA_GOT_IP) {

        ip_event_got_ip_t *event = event_data;

        connected = 1;

        ESP_LOGI(
            TAG,
            "Wi-Fi connected, IP=" IPSTR,
            IP2STR(&event->ip_info.ip)
        );
    }
}

int tk_comms_init(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            wifi_event_handler,
            NULL
        )
    );

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            wifi_event_handler,
            NULL
        )
    );

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifi_config
        )
    );

    ESP_ERROR_CHECK(esp_wifi_start());

    return 0;
}

int tk_comms_is_connected(void)
{
    return connected;
}