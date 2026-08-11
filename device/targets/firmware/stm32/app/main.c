#include <stdint.h>

#include "stm32f4xx.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_utils.h"

#include "tk_core.h"
#include "tk_esp32_link.h"
#include "tk_platform.h"
#include "tk_telemetry_json.h"
#include "tk_types.h"

#define LED_GPIO_PORT GPIOA
#define LED_PIN LL_GPIO_PIN_5

#define MAINS_STATUS_GPIO_PORT GPIOC
#define MAINS_STATUS_PIN LL_GPIO_PIN_0

#define PUMP_STATUS_GPIO_PORT GPIOC
#define PUMP_STATUS_PIN LL_GPIO_PIN_1

#define USART2_TX_GPIO_PORT GPIOA
#define USART2_TX_PIN LL_GPIO_PIN_2
#define USART2_BAUDRATE 115200U

#define TELEMETRY_PAYLOAD_SIZE 256

static void clock_init(void);
static void led_init(void);
static void inputs_init(void);
static void usart2_init(void);
static void usart2_write_char(char ch);
static void usart2_write_string(const char *text);

static void stm32_log(const char *message);
static tk_wall_time_state_t stm32_get_unix_time_ms(uint64_t *out);
static int stm32_read_inputs(tk_input_state_t *out);
static int stm32_publish_telemetry(const tk_telemetry_t *telemetry);
static int stm32_should_stop(void);
static int stm32_status_led_toggle(void);

static const tk_platform_t platform = {
    .log = stm32_log,
    .read_inputs = stm32_read_inputs,
    .get_unix_time_ms = stm32_get_unix_time_ms,
    .publish_telemetry = stm32_publish_telemetry,
    .should_stop = stm32_should_stop,
    .status_led_toggle = stm32_status_led_toggle
};

int main(void)
{
    clock_init();
    led_init();
    inputs_init();
    usart2_init();
    tk_esp32_link_init();

    stm32_log("tilekoumanto stm32 alive");

    (void)tk_core_run(&platform);

    stm32_log("tk_core_run returned unexpectedly");

    for (;;) {
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

static void inputs_init(void)
{
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);

    LL_GPIO_InitTypeDef gpio = {0};

    gpio.Pin = MAINS_STATUS_PIN | PUMP_STATUS_PIN;
    gpio.Mode = LL_GPIO_MODE_INPUT;
    gpio.Pull = LL_GPIO_PULL_UP;

    LL_GPIO_Init(MAINS_STATUS_GPIO_PORT, &gpio);
}

static void usart2_init(void)
{
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

    LL_GPIO_InitTypeDef gpio = {0};

    gpio.Pin = USART2_TX_PIN;
    gpio.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio.Pull = LL_GPIO_PULL_UP;
    gpio.Alternate = LL_GPIO_AF_7;

    LL_GPIO_Init(USART2_TX_GPIO_PORT, &gpio);

    LL_USART_InitTypeDef usart = {0};

    usart.BaudRate = USART2_BAUDRATE;
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

static void stm32_log(const char *message)
{
    usart2_write_string(message);
    usart2_write_string("\r\n");
}

static tk_wall_time_state_t stm32_get_unix_time_ms(
    uint64_t *out
)
{
    if (out == 0) {
        return TK_WALL_TIME_FAULT;
    }

    *out = 0U;

    return TK_WALL_TIME_UNSYNCED;
}

static int stm32_read_inputs(
    tk_input_state_t *out
)
{
    if (out == 0) {
        return -1;
    }

    out->mains_power =
        LL_GPIO_IsInputPinSet(
            MAINS_STATUS_GPIO_PORT,
            MAINS_STATUS_PIN
        )
            ? TK_MAINS_POWER_NOT_PRESENT
            : TK_MAINS_POWER_PRESENT;

    out->pump_relay =
        LL_GPIO_IsInputPinSet(
            PUMP_STATUS_GPIO_PORT,
            PUMP_STATUS_PIN
        )
            ? TK_PUMP_RELAY_INACTIVE
            : TK_PUMP_RELAY_ACTIVE;

    return 0;
}

static int stm32_publish_telemetry(const tk_telemetry_t *telemetry)
{
    char payload[TELEMETRY_PAYLOAD_SIZE];

    int length = tk_telemetry_json_serialize(
        telemetry,
        payload,
        sizeof(payload)
    );

    if (length < 0) {
        return -1;
    }

    return tk_esp32_link_send(payload);
}

static int stm32_should_stop(void)
{
    return 0;
}

static int stm32_status_led_toggle(void)
{
    LL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
    return 0;
}

void vApplicationIdleHook(void)
{
    __WFI();
}