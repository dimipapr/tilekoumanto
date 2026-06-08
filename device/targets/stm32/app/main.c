#include <stdint.h>

#include "stm32f4xx.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_utils.h"

#define LED_GPIO_PORT GPIOA
#define LED_PIN LL_GPIO_PIN_5

#define USART_TX_GPIO_PORT GPIOA
#define USART_TX_PIN LL_GPIO_PIN_2
#define USART_BAUDRATE 115200U

static void clock_init(void);
static void led_init(void);
static void usart2_init(void);
static void usart2_write_char(char ch);
static void usart2_write_string(const char *text);

int main(void)
{
    clock_init();
    led_init();
    usart2_init();

    usart2_write_string("tilekoumanto stm32 alive\r\n");

    while (1) {
        LL_GPIO_SetOutputPin(LED_GPIO_PORT, LED_PIN);
        usart2_write_string("tick\r\n");
        LL_mDelay(100);

        LL_GPIO_ResetOutputPin(LED_GPIO_PORT, LED_PIN);
        LL_mDelay(900);
    }
}

static void clock_init(void)
{
    LL_RCC_HSI_Enable();
    while (LL_RCC_HSI_IsReady() != 1) {
    }

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI) {
    }

    SystemCoreClock = 16000000U;

    LL_Init1msTick(SystemCoreClock);
    LL_SetSystemCoreClock(SystemCoreClock);
}

static void led_init(void)
{
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

    LL_GPIO_InitTypeDef gpio = {0};

    gpio.Pin = LED_PIN;
    gpio.Mode = LL_GPIO_MODE_OUTPUT;
    gpio.Speed = LL_GPIO_SPEED_FREQ_LOW;
    gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio.Pull = LL_GPIO_PULL_NO;

    LL_GPIO_Init(LED_GPIO_PORT, &gpio);
}

static void usart2_init(void)
{
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

    LL_GPIO_InitTypeDef gpio = {0};

    gpio.Pin = USART_TX_PIN;
    gpio.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio.Pull = LL_GPIO_PULL_UP;
    gpio.Alternate = LL_GPIO_AF_7;

    LL_GPIO_Init(USART_TX_GPIO_PORT, &gpio);

    LL_USART_InitTypeDef usart = {0};

    usart.BaudRate = USART_BAUDRATE;
    usart.DataWidth = LL_USART_DATAWIDTH_8B;
    usart.StopBits = LL_USART_STOPBITS_1;
    usart.Parity = LL_USART_PARITY_NONE;
    usart.TransferDirection = LL_USART_DIRECTION_TX;
    usart.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    usart.OverSampling = LL_USART_OVERSAMPLING_16;

    LL_USART_Init(USART2, &usart);
    LL_USART_ConfigAsyncMode(USART2);
    LL_USART_Enable(USART2);
}

static void usart2_write_char(char ch)
{
    while (LL_USART_IsActiveFlag_TXE(USART2) == 0) {
    }

    LL_USART_TransmitData8(USART2, (uint8_t)ch);
}

static void usart2_write_string(const char *text)
{
    while (*text != '\0') {
        usart2_write_char(*text);
        text++;
    }

    while (LL_USART_IsActiveFlag_TC(USART2) == 0) {
    }
}