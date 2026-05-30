#include "vga.h"
#include "vga13h.h"
#include <stdint.h>
#include <stdarg.h>

extern int vsnprintf(char *buf, size_t sz, const char *fmt, va_list ap);

void vga_init(void)
{
    vga13h_init();
}

void vga_clear(void)
{
    vga13h_clear(0);
}

void vga_putchar(char c)
{
    vga13h_putchar_console(c);
}

void vga_puts(const char *s)
{
    vga13h_puts(s);
}

void vga_set_color(vga_color_t fg, vga_color_t bg)
{
    vga13h_set_fg((uint8_t)fg);
    vga13h_set_bg((uint8_t)bg);
}

void vga_set_cursor(int x, int y)
{
    vga13h_set_cursor(x, y);
}

void vga_vprintf(const char *format, va_list args)
{
    char buf[256];
    vsnprintf(buf, sizeof(buf), format, args);
    vga13h_puts(buf);
}

void vga_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char buf[256];
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    vga13h_puts(buf);
}
