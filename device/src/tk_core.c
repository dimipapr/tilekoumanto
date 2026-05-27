#include "tk_hal.h"
#include "FreeRTOS.h"
#include "task.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <unistd.h>

// Internal State and callbacks

static char device_uuid[64] = {0};
static char topic[128] = {0};
static char mqtt_payload[256] = {0};

static hal_get_telemetry_cb get_telemetry = NULL;
static hal_mqtt_publish_cb mqtt_publish = NULL;
static hal_get_unix_time_cb get_unix_time = NULL;
static hal_lock_cb lock = NULL;
static hal_unlock_cb unlock = NULL;

#define POLL_INTERVAL_MS 100
#define TELEMETRY_INTERVAL_MS 5000

static tk_telemetry_t last_published_state = {0};

//FreeRTOS Static Allocation Memory Blocks
static StaticTask_t xLogicTaskBuffer;
#define LOGIC_TASK_STACK_SIZE 2048
static StackType_t uxLogicTaskStack[LOGIC_TASK_STACK_SIZE];

/*
 * Mandatory Kernel Hook.
 * FreeRTOS demands this when configSUPPORT_STATIC_ALLOCATION is 1.
 * It provides the static memory buffers required to run the internal Idle Task.
 */

void vApplicationGetIdleTaskMemory(
    StaticTask_t **ppxIdleTaskTCBBuffer,
    StackType_t **ppxIdleTaskStackBuffer,
    configSTACK_DEPTH_TYPE *puxIdleTaskStackSize
){
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *puxIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationIdleHook(void){
    usleep(1000);
}

//core proccessing loop

static void vLogicTask(void *pvParameters){
    (void)pvParameters;

    TickType_t xLastWakeTime = xTaskGetTickCount();

    const TickType_t xPollFrequency = pdMS_TO_TICKS(POLL_INTERVAL_MS);
    const TickType_t xTelemetryTimeout = pdMS_TO_TICKS(TELEMETRY_INTERVAL_MS);
    
    TickType_t xLastPublishTime = xTaskGetTickCount();
    
    while(1){
        vTaskDelayUntil(&xLastWakeTime, xPollFrequency);

        tk_telemetry_t current_state = {0};

        if (lock) lock();
        get_telemetry(&current_state);
        if (unlock) unlock();

        bool state_changed = (current_state.relay_active != last_published_state.relay_active) ||
                                (current_state.mains_present != last_published_state.mains_present);

        //check for timeout
        TickType_t xCurrentTime = xTaskGetTickCount();
        bool timeout_reached = (xCurrentTime - xLastPublishTime) >= xTelemetryTimeout;

        if (state_changed || timeout_reached){//publish telemetry decision
            uint64_t epoch_time = get_unix_time();

            snprintf(
                mqtt_payload,
                sizeof(mqtt_payload),
                "{\"metadata\":{\"device_uuid\":\"%s\",\"timestamp\":%"PRIu64"},"
                "\"payload\":{\"mains_present\":%s,\"relay_active\":%s}}",
                device_uuid, 
                epoch_time, 
                current_state.mains_present ? "true" : "false", 
                current_state.relay_active ? "true" : "false"
            );
            mqtt_publish(topic, mqtt_payload);

            last_published_state = current_state;
            xLastPublishTime = xCurrentTime;
        }
    }
}

void tk_core_init(
    const char *_uuid,
    hal_get_telemetry_cb tel_cb,
    hal_mqtt_publish_cb pub_cb,
    hal_get_unix_time_cb time_cb,
    hal_lock_cb lock_cb,
    hal_unlock_cb unlock_cb
){
    if (!_uuid || !tel_cb || !pub_cb || !time_cb) {
        // Halt right here. Do not let FreeRTOS start up.
        while (1) {
            // Trap here if a function pointer is missing.
            //TODO On physical STM32, turn on an error LED here
        }
    }
    get_telemetry = tel_cb;
    mqtt_publish = pub_cb;
    get_unix_time = time_cb;
    lock = lock_cb;
    unlock = unlock_cb;

    strncpy(device_uuid, _uuid, sizeof(device_uuid)-1);
    device_uuid[sizeof(device_uuid) - 1] = '\0';

    snprintf(topic, sizeof(topic), "devices/%s/pump/telemetry", device_uuid);

    //initialize the task workspace

    xTaskCreateStatic(
        vLogicTask,
        "LogicTask",
        LOGIC_TASK_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY +1,
        uxLogicTaskStack,
        &xLogicTaskBuffer
    );

    //start freertos scheduler and hand off control
    vTaskStartScheduler();
}
// void vPortEndScheduler(void);

void tk_core_stop(void){
    // vPortEndScheduler();
}

#if defined(PLATFORM_EMBEDDED)
/**
 * Required by the STM32 startup assembly code to initialize 
 * low-level hardware microcontroller system clocks.
 */
void SystemInit(void) {
    // For now, keep it empty to let the chip boot into its default internal RC oscillator (16 MHz)
}

/**
 * Hardware execution entry point called by Reset_Handler
 */
int main(void) {
    // Your hardware initialization and FreeRTOS task startup logic will go here
    
    // Fallback infinite loop to prevent main from returning
    while(1) {
    }
    return 0;
}
#endif