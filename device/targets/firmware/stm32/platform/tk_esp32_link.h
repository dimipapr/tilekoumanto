#ifndef TK_STM32_LINK_H
#define TK_STM32_LINK_H

void tk_esp32_link_init(void);
int tk_esp32_link_send(const char *payload);

#endif