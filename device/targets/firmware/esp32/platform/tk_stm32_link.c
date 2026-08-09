#include "tk_stm32_link.h"

#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#define STM32_UART UART_NUM_1
#define STM32_UART_RX_GPIO GPIO_NUM_17
#define STM32_UART_RX_BUFFER_SIZE 256

#define STM32_MESSAGE_QUEUE_LENGTH 8

#define STM32_UART_TASK_STACK_SIZE 4096
#define STM32_UART_TASK_PRIORITY 5

static const char *TAG = "tk_stm32_link";

static QueueHandle_t message_queue;

static void stm32_uart_receive_task(void *arg){
    (void)arg;

    tk_stm32_message_t message = {0};
    bool discarding_message = false;
    uint8_t byte;

    while (1){
        int received = uart_read_bytes(
            STM32_UART,
            &byte,
            1,
            portMAX_DELAY
        );

        if (received <= 0){
            continue;
        }

        if (byte == '\r'){
            continue;
        }

        if (byte == '\n'){
            if (discarding_message){
                discarding_message = false;
                message.length = 0;
                continue;
            }

            if (message.length == 0){
                continue;
            }

            message.data[message.length] = '\0';

            if (xQueueSend(message_queue, &message, 0) != pdPASS){
                ESP_LOGW(
                    TAG,
                    "Message queue full, dropping STM32 message"
                );
            }

            message.length = 0;
            continue;
        }

        if (discarding_message){
            continue;
        }

        if (message.length >= sizeof(message.data) - 1){
            ESP_LOGW(
                TAG,
                "STM32 message exceeds maximum size, discarding"
            );

            discarding_message = true;
            message.length = 0;
            continue;
        }

        message.data[message.length] = (char)byte;
        message.length++;
    }
}

esp_err_t tk_stm32_link_init(void){
    if (message_queue != NULL){
        return ESP_ERR_INVALID_STATE;
    }

    message_queue = xQueueCreate(
        STM32_MESSAGE_QUEUE_LENGTH,
        sizeof(tk_stm32_message_t)
    );

    if (message_queue == NULL){
        return ESP_ERR_NO_MEM;
    }

    esp_err_t err = uart_driver_install(
        STM32_UART,
        STM32_UART_RX_BUFFER_SIZE,
        0,
        0,
        NULL,
        0
    );

    if (err != ESP_OK){
        vQueueDelete(message_queue);
        message_queue = NULL;
        return err;
    }

    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    err = uart_param_config(STM32_UART, &uart_config);

    if (err != ESP_OK){
        uart_driver_delete(STM32_UART);
        vQueueDelete(message_queue);
        message_queue = NULL;
        return err;
    }

    err = uart_set_pin(
        STM32_UART,
        UART_PIN_NO_CHANGE,
        STM32_UART_RX_GPIO,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    );

    if (err != ESP_OK){
        uart_driver_delete(STM32_UART);
        vQueueDelete(message_queue);
        message_queue = NULL;
        return err;
    }

    err = gpio_set_pull_mode(
        STM32_UART_RX_GPIO,
        GPIO_PULLUP_ONLY
    );

    if (err != ESP_OK){
        uart_driver_delete(STM32_UART);
        vQueueDelete(message_queue);
        message_queue = NULL;
        return err;
    }

    BaseType_t task_created = xTaskCreate(
        stm32_uart_receive_task,
        "stm32_uart_rx",
        STM32_UART_TASK_STACK_SIZE,
        NULL,
        STM32_UART_TASK_PRIORITY,
        NULL
    );

    if (task_created != pdPASS){
        uart_driver_delete(STM32_UART);
        vQueueDelete(message_queue);
        message_queue = NULL;
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

esp_err_t tk_stm32_link_receive(
    tk_stm32_message_t *message,
    TickType_t ticks_to_wait
){
    if (message == NULL){
        return ESP_ERR_INVALID_ARG;
    }

    if (message_queue == NULL){
        return ESP_ERR_INVALID_STATE;
    }

    if (
        xQueueReceive(
            message_queue,
            message,
            ticks_to_wait
        ) != pdPASS
    ){
        return ESP_ERR_TIMEOUT;
    }

    return ESP_OK;
}