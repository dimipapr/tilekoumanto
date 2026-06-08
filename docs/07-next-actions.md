# Next Actions 

## Current focus 

Clean up the STM32 shared-core runtime stub and prepare for real input integration. 

## Immediate next actions 

   1. Clean up STM32 bring-up logging. 
      
      Success condition: 
      - temporary debug logs used during scheduler bring-up are removed 
      - stable logs remain readable 
      - USART output confirms core start, task start, telemetry stub behavior, and publish/skip behavior 
   2. Keep LTE, MQTT, certificate storage, identity loading, modem publishing, and real field input handling out of scope until the STM32 shared-core stub is clean and documented. 
   3. After cleanup, define the smallest STM32 real-input step. 
      Success condition: 
      - identify which GPIO pins will represent the MVP mains power and pump relay inputs on the NUCLEO-F446RE 
      - keep input reading local and simple 
      - do not add debouncing, MQTT, LTE, or production wiring yet unless documented as a separate next step