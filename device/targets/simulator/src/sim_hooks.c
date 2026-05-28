#include "FreeRTOS.h"
#include "task.h"
#include <unistd.h>

/** Simulator-only Idle Hook
 *  Throttles the host Linux CPU so the POSIX simulation doesn't peg a CPU core at 100%.
 */

void vApplicationIdleHook(void){
    usleep(1000);
}