#include "vga.h"
#include "vga13h.h"
#include <stdint.h>

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

static void print_dec(int val)
{
    char buf[12];
    int n = 0;
    if (val < 0) { vga13h_putchar_console('-'); val = -val; }
    if (val == 0) { vga13h_putchar_console('0'); return; }
    while (val > 0) { buf[n++] = '0' + (val % 10); val /= 10; }
    while (n > 0) vga13h_putchar_console(buf[--n]);
}

static void print_hex(unsigned int val)
{
    char buf[12];
    int n = 0;
    if (val == 0) { vga13h_putchar_console('0'); return; }
    while (val > 0) {
        int d = val % 16;
        buf[n++] = d < 10 ? '0' + d : 'a' + d - 10;
        val /= 16;
    }
    while (n > 0) vga13h_putchar_console(buf[--n]);
}

void vga_printf(const char *format, ...)
{
    uint32_t *arg = (uint32_t *)&format + 1;

    for (int i = 0; format[i]; i++)
    {
        if (format[i] == '%' && format[i + 1])
        {
            i++;
            switch (format[i])
            {
            case 'd':
                print_dec((int)*arg++);
                break;
            case 'x':
                print_hex((unsigned int)*arg++);
                break;
            case 's':
                vga13h_puts((const char *)*arg++);
                break;
            case 'c':
                vga13h_putchar_console((char)*arg++);
                break;
            case '0':
                if (format[i + 1] == '2' && format[i + 2] == 'd')
                {
                    i += 2;
                    int val = (int)*arg++;
                    if (val >= 0 && val < 10)
                        vga13h_putchar_console('0');
                    print_dec(val);
                    break;
                }
                vga13h_putchar_console('%');
                vga13h_putchar_console('0');
                break;
            default:
                vga13h_putchar_console('%');
                vga13h_putchar_console(format[i]);
                break;
            }
        }
        else
        {
            vga13h_putchar_console(format[i]);
        }
    }
}
