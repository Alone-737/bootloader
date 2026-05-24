#include "errors.h"
#include "vga.h"
#include <stdarg.h>

void panic(const char *fmt, ...)
{
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    vga_puts("\nKERNEL PANIC: ");

    va_list args;
    va_start(args, fmt);
    vga_vprintf(fmt, args);
    va_end(args);

    vga_puts("\nSystem Halted.");

    __asm__ volatile("cli");
    for (;;)
        __asm__ volatile("hlt");
}
