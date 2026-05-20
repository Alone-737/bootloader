#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define TIMER_FREQ 100

void timer_init(void);
void timer_handler(void);
uint32_t timer_get_ticks(void);

#endif
