
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"

#include "tk_comms.h"
#include "tk_stm32_link.h"

#define LED_GPIO GPIO_NUM_48
#define LED_BLINK_INTERVAL_MS 500


#define DEVICE_UUID "1c2bf9ab-8ca2-41ed-8df5-a5ac116ebecf"
#define TELEMETRY_TOPIC "devices/" DEVICE_UUID "/pump/telemetry"

#define STM32_MESSAGE_TASK_STACK_SIZE 4096
#define STM32_MESSAGE_TASK_PRIORITY 5

#define LED_TASK_STACK_SIZE 4096
#define LED_TASK_PRIORITY 1

static const char *TAG = "tilekoumanto";

static void led_blink_task(void *arg){
    (void)arg;

    led_strip_handle_t led_strip;

    const led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO,
        .max_leds = 1,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags.invert_out = false,
    };

    const led_strip_rmt_config_t rmt_config = {
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

    while (1){
        ESP_ERROR_CHECK(
            led_strip_set_pixel(
                led_strip,
                0,
                0,
                1,
                0
            )
        );

        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(LED_BLINK_INTERVAL_MS));

        ESP_ERROR_CHECK(led_strip_clear(led_strip));
        vTaskDelay(pdMS_TO_TICKS(LED_BLINK_INTERVAL_MS));
    }
}

static void stm32_message_task(void *arg){
    (void)arg;

    tk_stm32_message_t message;

    while(1){
        esp_err_t err = tk_stm32_link_receive(
            &message,
            portMAX_DELAY
        );

        if (err != ESP_OK){
            ESP_LOGE(
                TAG,
                "Failed to receive STM32 message: %s",
                esp_err_to_name(err)
            );

            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        if (
            message.data[0] != '{' ||
            message.data[message.length - 1] != '}'
        ){
            ESP_LOGW(TAG, "Ignoring invalid STM32 message");
            continue;
        }

        if (!tk_comms_mqtt_is_connected()){
            ESP_LOGW(
                TAG,
                "Dropping STM32 telemetry: MQTT disconnected"
            );

            continue;
        }

        int message_id = tk_comms_publish(
            TELEMETRY_TOPIC,
            message.data
        );

        if (message_id >= 0){
            ESP_LOGI(
                TAG,
                "STM32 telemetry queued, message_id=%d",
                message_id
            );
        } else {
            ESP_LOGW(
                TAG,
                "STM32 telemetry publish failed"
            );
        }
    }
}

void app_main(void){
    ESP_LOGI(TAG, "ESP32-S3 target starting");

    ESP_ERROR_CHECK(tk_stm32_link_init());
    ESP_ERROR_CHECK(tk_comms_init());

    ESP_ERROR_CHECK(
        xTaskCreate(
            stm32_message_task,
            "stm32_message",
            STM32_MESSAGE_TASK_STACK_SIZE,
            NULL,
            STM32_MESSAGE_TASK_PRIORITY,
            NULL
        ) == pdPASS
            ? ESP_OK
            : ESP_ERR_NO_MEM
    );

    ESP_ERROR_CHECK(
        xTaskCreate(
            led_blink_task,
            "led_blink",
            LED_TASK_STACK_SIZE,
            NULL,
            LED_TASK_PRIORITY,
            NULL
        ) == pdPASS?ESP_OK:ESP_ERR_NO_MEM);
    return;
}