#ifndef TK_LOG_H
#define TK_LOG_H

#include "tk_platform.h"

/**
 * Writes a log message immediately thtough platform->log().
 * 
 * Intended for startup, shutdown, and other contexts where the logging
 * task is unnavailable. Do not call concurrently from multiple tasks.
 * 
 * Returns 0 on success, or -1 if the arguments are invalid or formatting fails.
 */

 int tk_log_sync(
    const tk_platform_t *platform,
    const char *format,
    ...
 );

 /**
  * Formats and enqueues a log message without waiting for transmission.
  * 
  * Intended for normal task-constext logging after the core runtime has
  * initialized the logging queue. The message is dropped if the queue is unavalable of full.
  * 
  * Must not be called from an interrupt.
  * 
  * Returns 0 if the message was queued, or -1 if the queue is unavailable,
  * full, or formatting fails.
  */

int tk_log_enqueue(
    const char *format,
    ...
);

#endif