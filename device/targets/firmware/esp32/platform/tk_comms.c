#include "tk_comms.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "nvs_flash.h"

#include "mqtt_config.h"
#include "wifi_secrets.h"

#include <stdbool.h>

static const char *TAG = "tk_comms";

static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_connected = false;

static void mqtt_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data
){
    (void)arg;
    (void)event_base;
    esp_mqtt_event_handle_t event = event_data;

    if (event_id == MQTT_EVENT_CONNECTED){
        mqtt_connected = true;
        ESP_LOGI(TAG, "MQTT connected");
    }

    if (event_id == MQTT_EVENT_DISCONNECTED){
        mqtt_connected = false;
        ESP_LOGW(TAG, "MQTT disconnected");
    }

    if (event_id == MQTT_EVENT_PUBLISHED){
    ESP_LOGI(
        TAG,
        "MQTT message published, message_id=%d",
        event->msg_id
    );
}
}

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data
){
    (void)arg;

    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START){

        ESP_ERROR_CHECK(esp_wifi_connect());
    }

    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_DISCONNECTED){

        wifi_event_sta_disconnected_t *event = event_data;

        ESP_LOGW(
            TAG,
            "Wi-Fi disconnected, reason=%d",
            event->reason
        );
    }

    if (event_base == IP_EVENT &&
        event_id == IP_EVENT_STA_GOT_IP){

        ip_event_got_ip_t *event = event_data;

        ESP_LOGI(
            TAG,
            "Wi-Fi connected, IP=" IPSTR,
            IP2STR(&event->ip_info.ip)
        );

        ESP_ERROR_CHECK(esp_mqtt_client_start(mqtt_client));
    }
}

esp_err_t tk_comms_init(void){
    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));

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

    extern const unsigned char ca_cert_start[]
        asm("_binary_ca_crt_start");

    extern const unsigned char client_cert_start[]
        asm("_binary_device_crt_start");

    extern const unsigned char client_key_start[]
        asm("_binary_device_key_start");

    esp_mqtt_client_config_t mqtt_config = {
        .broker = {
            .address.uri = MQTT_BROKER_URI,
            .verification.certificate =
                (const char *)ca_cert_start,
        },
        .credentials = {
            .authentication = {
                .certificate =
                    (const char *)client_cert_start,
                .key =
                    (const char *)client_key_start,
            },
        },
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_config);

    ESP_ERROR_CHECK(
        esp_mqtt_client_register_event(
            mqtt_client,
            ESP_EVENT_ANY_ID,
            mqtt_event_handler,
            NULL
        )
    );

    return ESP_OK;
}

bool tk_comms_is_ready(void){
    return mqtt_connected;
}

int tk_comms_publish(
    const char *topic,
    const char *payload
){
    if (mqtt_client == NULL ||
        !mqtt_connected ||
        topic == NULL ||
        payload == NULL){
            return -1;
        }
    
        return esp_mqtt_client_publish(
            mqtt_client,
            topic,
            payload,
            0,
            1,
            0
        );
}