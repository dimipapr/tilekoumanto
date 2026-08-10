#include <stdint.h>

#include "stm32f4xx.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_usart.h"

#include "tk_esp32_link.h"

#define ESP32_LINK_USART USART1
#define ESP32_LINK_GPIO_PORT GPIOA
#define ESP32_LINK_TX_PIN LL_GPIO_PIN_9
#define ESP32_LINK_BAUDRATE 115200U

static void write_char(char ch);
static void write_string(const char *text);

void tk_esp32_link_init(void)
{
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

    LL_GPIO_InitTypeDef gpio = {0};

    gpio.Pin = ESP32_LINK_TX_PIN;
    gpio.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio.Pull = LL_GPIO_PULL_UP;
    gpio.Alternate = LL_GPIO_AF_7;

    LL_GPIO_Init(ESP32_LINK_GPIO_PORT, &gpio);

    LL_USART_InitTypeDef usart = {0};

    usart.BaudRate = ESP32_LINK_BAUDRATE;
    usart.DataWidth = LL_USART_DATAWIDTH_8B;
    usart.StopBits = LL_USART_STOPBITS_1;
    usart.Parity = LL_USART_PARITY_NONE;
    usart.TransferDirection = LL_USART_DIRECTION_TX;
    usart.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    usart.OverSampling = LL_USART_OVERSAMPLING_16;

    LL_USART_Init(ESP32_LINK_USART, &usart);
    LL_USART_ConfigAsyncMode(ESP32_LINK_USART);
    LL_USART_Enable(ESP32_LINK_USART);
}

int tk_esp32_link_send(const char *payload)
{
    if (payload == 0) {
        return -1;
    }

    write_string(payload);
    write_char('\n');

    while (LL_USART_IsActiveFlag_TC(ESP32_LINK_USART) == 0) {
    }

    return 0;
}

static void write_char(char ch)
{
    while (LL_USART_IsActiveFlag_TXE(ESP32_LINK_USART) == 0) {
    }

    LL_USART_TransmitData8(ESP32_LINK_USART, (uint8_t)ch);
}

static void write_string(const char *text)
{
    while (*text != '\0') {
        write_char(*text);
        text++;
    }
}