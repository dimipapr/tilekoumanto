#ifndef TK_STM32_LINK_H
#define TK_STM32_LINK_H

#include <stddef.h>

#include "esp_err.h"
#include "freertos/FreeRTOS.h"

#define TK_STM32_MESSAGE_MAX_SIZE 256

typedef struct {
    size_t length;
    char data[TK_STM32_MESSAGE_MAX_SIZE];
}tk_stm32_message_t;

esp_err_t tk_stm32_link_init(void);
esp_err_t tk_stm32_link_receive(
    tk_stm32_message_t *message,
    TickType_t ticks_to_wait
);

#endif