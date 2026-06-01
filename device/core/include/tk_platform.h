#ifndef TK_PLATFORM_H
#define TK_PLATFORM_H

typedef struct 
{
    void (*toggle_status_led)(void);
}tk_platform_t;

#endif
