#include "vga.h"
#include <stdarg.h>

static void itoa(int value, char *str, int base);

static uint16_t *vga_buffer = (uint16_t *)VGA_MEMORY;
static int vga_x = 0;
static int vga_y = 0;
static vga_color_t vga_fg = VGA_COLOR_WHITE;
static vga_color_t vga_bg = VGA_COLOR_BLACK;

static uint8_t vga_make_color(vga_color_t fg, vga_color_t bg)
{
    return fg | (bg << 4);
}

static uint16_t vga_make_vram_entry(unsigned char c, uint8_t color)
{
    return (uint16_t)c | ((uint16_t)color << 8);
}

void vga_init(void)
{
    vga_x = 0;
    vga_y = 0;
    vga_fg = VGA_COLOR_WHITE;
    vga_bg = VGA_COLOR_BLACK;
    vga_clear();
}

void vga_clear(void)
{
    uint8_t color = vga_make_color(vga_fg, vga_bg);
    for (int y = 0; y < VGA_HEIGHT; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            int index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_make_vram_entry(' ', color);
        }
    }
    vga_x = 0;
    vga_y = 0;
}

void vga_set_color(vga_color_t fg, vga_color_t bg)
{
    vga_fg = fg;
    vga_bg = bg;
}

void vga_set_cursor(int x, int y)
{
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (x >= VGA_WIDTH)
        x = VGA_WIDTH - 1;
    if (y >= VGA_HEIGHT)
        y = VGA_HEIGHT - 1;

    vga_x = x;
    vga_y = y;
}

void vga_putchar(char c)
{
    uint8_t color = vga_make_color(vga_fg, vga_bg);

    if (c == '\n')
    {
        vga_y++;
        vga_x = 0;
    }
    else if (c == '\r')
    {
        vga_x = 0;
    }
    else if (c == '\t')
    {
        vga_x += 4;
    }
    else if (c >= 32 && c < 127)
    {
        int index = vga_y * VGA_WIDTH + vga_x;
        vga_buffer[index] = vga_make_vram_entry(c, color);
        vga_x++;
    }

    if (vga_x >= VGA_WIDTH)
    {
        vga_x = 0;
        vga_y++;
    }

    if (vga_y >= VGA_HEIGHT)
    {
        for (int i = 0; i < VGA_WIDTH * (VGA_HEIGHT - 1); i++)
        {
            vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
        }
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            int index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
            vga_buffer[index] = vga_make_vram_entry(' ', color);
        }
        vga_y = VGA_HEIGHT - 1;
    }
}

void vga_puts(const char *s)
{
    if (!s)
        return;
    for (int i = 0; s[i]; i++)
    {
        vga_putchar(s[i]);
    }
}

void vga_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    for (int i = 0; format[i]; i++)
    {
        if (format[i] == '%' && format[i + 1])
        {
            i++;
            switch (format[i])
            {
            case 'd':
            {
                int val = va_arg(args, int);
                char buf[12];
                itoa(val, buf, 10);
                vga_puts(buf);
                break;
            }
            case 'x':
            {
                unsigned int val = va_arg(args, unsigned int);
                char buf[12];
                itoa(val, buf, 16);
                vga_puts(buf);
                break;
            }
            case 's':
            {
                const char *s = va_arg(args, const char *);
                vga_puts(s);
                break;
            }
            case 'c':
            {
                char c = va_arg(args, int);
                vga_putchar(c);
                break;
            }
            default:
                vga_putchar('%');
                vga_putchar(format[i]);
                break;
            }
        }
        else
        {
            vga_putchar(format[i]);
        }
    }

    va_end(args);
}

static void itoa(int value, char *str, int base)
{
    if (base < 2 || base > 16)
        return;

    static const char digits[] = "0123456789abcdef";
    char buf[12];
    int i = 0;
    int negative = 0;

    if (value < 0)
    {
        negative = 1;
        value = -value;
    }

    if (value == 0)
    {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    while (value > 0)
    {
        buf[i++] = digits[value % base];
        value /= base;
    }

    if (negative)
    {
        str[0] = '-';
        for (int j = 0; j < i; j++)
        {
            str[j + 1] = buf[i - j - 1];
        }
        str[i + 1] = '\0';
    }
    else
    {
        for (int j = 0; j < i; j++)
        {
            str[j] = buf[i - j - 1];
        }
        str[i] = '\0';
    }
}