#ifndef __SYSTICK_H__
#define __SYSTICK_H__

#include <stdint.h>

void systick_init_config(uint64_t ticks);
void systick_init(void);
void systick_deinit(void);
uint64_t systick_get_tick(void);
void systick_delay_ms(uint32_t delay_ms);

#endif
