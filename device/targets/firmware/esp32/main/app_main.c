#include <string.h>

#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "wifi_secrets.h"


#include "esp_log.h"
#include "led_strip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "tilekoumanto";

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data
){
    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START){
            ESP_ERROR_CHECK(esp_wifi_connect());
    }

    if (event_base == WIFI_EVENT &&
    event_id == WIFI_EVENT_STA_DISCONNECTED) {

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
        } 
}

static void wifi_init_sta(void){
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
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(
        esp_wifi_set_config(WIFI_IF_STA, &wifi_config)
    );
    ESP_ERROR_CHECK(esp_wifi_start());
}

#define LED_GPIO GPIO_NUM_48

void app_main(void)
{

    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "============ ESP32-S3 alive ============");
    ESP_LOGI(TAG, "========================================");


    wifi_init_sta();


    led_strip_handle_t led_strip;

    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO,
        .max_leds = 1,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags.invert_out = false,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 0,
        .flags.with_dma = false,
    };

    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(
            &strip_config,
            &rmt_config,
            &led_strip
        )
    );

    while (1) {

        led_strip_set_pixel(led_strip, 0, 0, 1, 0);
        led_strip_refresh(led_strip);

        vTaskDelay(pdMS_TO_TICKS(500));


        led_strip_clear(led_strip);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}