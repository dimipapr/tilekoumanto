#include "FreeRTOS.h"
#include "task.h"
#include "tk_hal.h"

#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_cortex.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_gpio.h"

/**
 * Required by the STM32 startup assembly code to initialize
 * low-level hardware microcontroller system clocks.
 */

static StaticTask_t xBlinkTaskBuffer;
static StackType_t uxBlinkTaskStack[configMINIMAL_STACK_SIZE];

void vBlinkTask(void *pvParameters) {
    (void)pvParameters;
    while(1) {
        LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_5); // Optimized LL function
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vApplicationIdleHook(void){
   return;
}

static void MX_GPIO_Init(void)
{
    /* Enable GPIOA Clock */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

    /* Configure PA5 */
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_5, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_5, LL_GPIO_SPEED_FREQ_LOW);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_5, LL_GPIO_PULL_NO);
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_0)
  {
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE3);
  LL_PWR_DisableOverDriveMode();
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {

  }
  LL_Init1msTick(16000000);
  LL_SetSystemCoreClock(16000000);
  LL_RCC_SetTIMPrescaler(LL_RCC_TIM_PRESCALER_TWICE);
}

int main(void){
   
   // Hardware initialization
   SystemClock_Config();
   MX_GPIO_Init();

   // Spin up the blinky
   xTaskCreateStatic(
        vBlinkTask,
        "Blinky",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        uxBlinkTaskStack,
        &xBlinkTaskBuffer
    );

    //hand over control to FreeRTOS
    vTaskStartScheduler();

   // Fallback infinite loop to prevent main from returning

   while (1)
   {;}
   return 0;
   
}