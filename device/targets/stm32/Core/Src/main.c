#include "FreeRTOS.h"
#include "task.h"
#include "tk_hal.h"

/**
 * Required by the STM32 startup assembly code to initialize
 * low-level hardware microcontroller system clocks.
 */

 void SystemInit(void){
    //for now empty, letting the chip to boot into its default internal RC oscillator (16MHz)
    return;
 }

 void vApplicationIdleHook(void){
    return;
 }

 int main(void){
    // TODO: Hardware initialization

    // TODO: Call tk_call_init() with actual hardware HAL callbacks

    // Fallback infinite loop to prevent main from returning

    while (1)
    {;}
    return 0;
    
 }