#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>

#include "tk_comms.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "wifi_secrets.h"
#include "mqtt_client.h"
#include "mqtt_config.h"

static esp_mqtt_client_handle_t mqtt_client=NULL;
static int mqtt_connected = 0;

static const char *TAG = "tk_comms";

static int connected = 0;

static void mqtt_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data)
{
    (void)arg;
    (void)event_base;
    (void)event_data;

    if (event_id == MQTT_EVENT_CONNECTED) {
        mqtt_connected = 1;
        ESP_LOGI(TAG, "MQTT connected");
    }

    if (event_id == MQTT_EVENT_DISCONNECTED) {
        mqtt_connected = 0;
        ESP_LOGW(TAG, "MQTT disconnected");
    }
}

#include <netdb.h>
#include <arpa/inet.h>

static void debug_dns(void)
{
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
    };

    struct addrinfo *res = NULL;

    int err = getaddrinfo(
        "mqtt-dev.tilekoumanto.gr",
        "8883",
        &hints,
        &res
    );

    if (err != 0) {
        ESP_LOGE(TAG, "getaddrinfo failed: %d", err);
        return;
    }

    for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
        char addr[INET6_ADDRSTRLEN];

        if (p->ai_family == AF_INET) {
            struct sockaddr_in *sa = (struct sockaddr_in *)p->ai_addr;

            inet_ntop(
                AF_INET,
                &sa->sin_addr,
                addr,
                sizeof(addr)
            );

            ESP_LOGI(TAG, "MQTT DNS IPv4: %s", addr);
        }

        if (p->ai_family == AF_INET6) {
            struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)p->ai_addr;

            inet_ntop(
                AF_INET6,
                &sa6->sin6_addr,
                addr,
                sizeof(addr)
            );

            ESP_LOGI(TAG, "MQTT DNS IPv6: %s", addr);
        }
    }

    freeaddrinfo(res);
}

static void test_tcp(void)
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(8883),
    };

    inet_pton(AF_INET, "10.19.90.65", &addr.sin_addr);

    ESP_LOGI(TAG, "testing TCP to broker...");

    int rc = connect(
        sock,
        (struct sockaddr *)&addr,
        sizeof(addr)
    );

    if (rc == 0) {
        ESP_LOGI(TAG, "TCP connection successful");
    } else {
        ESP_LOGE(TAG, "TCP connection failed, errno=%d", errno);
    }

    close(sock);
}

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
        // debug_dns();
        // test_tcp();
        ESP_ERROR_CHECK(esp_mqtt_client_start(mqtt_client));

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

    extern const unsigned char ca_cert_start[]
    asm("_binary_ca_crt_start");

    extern const unsigned char client_cert_start[]
        asm("_binary_device_crt_start");

    extern const unsigned char client_key_start[]
        asm("_binary_device_key_start");

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = MQTT_BROKER_URI,
            .verification.certificate = (const char *)ca_cert_start,
        },

        .credentials = {
            .authentication = {
                .certificate = (const char *)client_cert_start,
                .key = (const char *)client_key_start,
            },
        },
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

    ESP_ERROR_CHECK(
        esp_mqtt_client_register_event(
            mqtt_client,
            ESP_EVENT_ANY_ID,
            mqtt_event_handler,
            NULL
        )
    );


    return 0;
}

int tk_comms_is_connected(void)
{
    return connected;
}