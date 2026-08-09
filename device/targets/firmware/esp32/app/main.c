
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"

#include "tk_comms.h"

#define LED_GPIO GPIO_NUM_48
#define LED_BLINK_INTERVAL_MS 500

static const char *TAG = "tilekoumanto";

#define DEVICE_UUID "1c2bf9ab-8ca2-41ed-8df5-a5ac116ebecf"
#define TELEMETRY_TOPIC "devices/" DEVICE_UUID "/pump/telemetry"

#define STM32_UART UART_NUM_1
#define STM32_UART_RX_GPIO GPIO_NUM_17
#define STM32_UART_RX_BUFFER_SIZE 256

static void stm32_uart_receive_task(void *arg){
    (void)arg;

    char payload[STM32_UART_RX_BUFFER_SIZE];
    size_t payload_length = 0;
    int discard_line = 0;

    while (1){
        uint8_t byte;

        int length = uart_read_bytes(
            STM32_UART,
            &byte,
            1,
            portMAX_DELAY
        );

        if (length != 1){
            continue;
        }

        if (byte == '\r'){
            continue;
        }

        if (byte == '\n'){
            if (discard_line){
                discard_line = 0;
                payload_length = 0;
                continue;
            }

            if (payload_length == 0){
                continue;
            }

            payload[payload_length] = '\0';

            if (payload[0] != '{' ||
                payload[payload_length - 1] != '}'){

                ESP_LOGW(TAG, "Ignoring invalid STM32 UART line");
                payload_length = 0;
                continue;
            }

            if (!tk_comms_mqtt_is_connected()){
                ESP_LOGW(
                    TAG,
                    "Dropping STM32 telemetry: MQTT disconnected"
                );

                payload_length = 0;
                continue;
            }

            int message_id = tk_comms_publish(
                TELEMETRY_TOPIC,
                payload
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

            payload_length = 0;
            continue;
        }

        if (payload_length >= sizeof(payload) - 1){
            ESP_LOGW(TAG, "STM32 UART line too long");
            payload_length = 0;
            discard_line = 1;
            continue;
        }

        payload[payload_length] = (char)byte;
        payload_length++;
    }
}

static void stm32_uart_init(void){
    ESP_ERROR_CHECK(
        uart_driver_install(
            STM32_UART,
            STM32_UART_RX_BUFFER_SIZE,
            0,
            0,
            NULL,
            0
        )
    );

    const uart_config_t config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_param_config(STM32_UART, &config));
    
    ESP_ERROR_CHECK(
        uart_set_pin(
            STM32_UART,
            UART_PIN_NO_CHANGE,
            STM32_UART_RX_GPIO,
            UART_PIN_NO_CHANGE,
            UART_PIN_NO_CHANGE
        )
    );

    ESP_ERROR_CHECK(
        gpio_set_pull_mode(
            STM32_UART_RX_GPIO,
            GPIO_PULLUP_ONLY
        )
    );

    ESP_ERROR_CHECK(
        xTaskCreate(
            stm32_uart_receive_task,
            "stm32_uart_rx",
            4096,
            NULL,
            5,
            NULL
        ) == pdPASS
            ? ESP_OK
            : ESP_ERR_NO_MEM
    );
}

void app_main(void){

    ESP_LOGI(TAG, "ESP32-S3 target starting");
    stm32_uart_init();
    ESP_ERROR_CHECK(tk_comms_init());

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

    while (1) {
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 0, 1, 0));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(LED_BLINK_INTERVAL_MS));

        ESP_ERROR_CHECK(led_strip_clear(led_strip));
        vTaskDelay(pdMS_TO_TICKS(LED_BLINK_INTERVAL_MS));
    }
}