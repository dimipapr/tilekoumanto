// device/targets/stm32/app/main.c

#include <stdint.h>

#include "stm32f4xx.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_utils.h"

#include "tk_core.h"
#include "tk_platform.h"
#include "tk_types.h"

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

static void stm32_log(const char *message);
static uint64_t stm32_unix_time_ms(void);
static int stm32_read_telemetry(tk_telemetry_t *out);
static int stm32_publish_telemetry(const tk_telemetry_t *telemetry);
static int stm32_should_stop(void);
static int stm32_status_led_toggle(void);


static const tk_platform_t platform = {
        .log = stm32_log,
        .unix_time_ms = stm32_unix_time_ms,
        .read_telemetry = stm32_read_telemetry,
        .publish_telemetry = stm32_publish_telemetry,
        .should_stop = stm32_should_stop,
        .status_led_toggle = stm32_status_led_toggle
    };

int main(void)
{


    clock_init();
    led_init();
    usart2_init();

    usart2_write_string("tilekoumanto stm32 alive\r\n");

    (void)tk_core_run(&platform);

    usart2_write_string("tk_core_run returned unexpectedly\r\n");

    for(;;){};
}

static int stm32_status_led_toggle(void){
    LL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
    return 0;
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

static void stm32_log(const char *message){
    usart2_write_string(message);
    usart2_write_string("\r\n");
}

static uint64_t stm32_unix_time_ms(void){
    //bringup stub
    return (uint64_t)tk_core_runtime_ms();
}

static int stm32_read_telemetry(tk_telemetry_t *out){
    if (out == 0)return -1;

    out->mains_power = TK_MAINS_POWER_PRESENT;
    out->pump_relay = TK_PUMP_RELAY_INACTIVE;
    out->unix_time_ms = stm32_unix_time_ms();
    out->seq = 0;

    return 0;
}

static int stm32_publish_telemetry(const tk_telemetry_t *telemetry){
    (void)telemetry;

    usart2_write_string("stm32 publish stub\r\n");
    return 0;
}

static int stm32_should_stop(void){return 0;}

void vApplicationIdleHook(void){
    __WFI();
}