#include "timer.h"
#include "io.h"

#define PIT_CH0 0x40
#define PIT_CMD 0x43
#define PIT_OCW 0x36
#define PIT_HZ 1193182

static volatile uint32_t tick_count = 0;

void timer_init(void)
{
    uint32_t divisor = PIT_HZ / TIMER_FREQ;
    outb(PIT_CMD, PIT_OCW);
    outb(PIT_CH0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CH0, (uint8_t)((divisor >> 8) & 0xFF));
}

void timer_handler(void)
{
    tick_count++;
}

uint32_t timer_get_ticks(void)
{
    return tick_count;
}
