#ifndef VGA_H
#define VGA_H

#include <stddef.h>
#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

typedef enum
{
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_RED = 4,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_WHITE = 15,
} vga_color_t;

void vga_init(void);
void vga_clear(void);
void vga_putchar(char c);
void vga_puts(const char *s);
void vga_set_color(vga_color_t fg, vga_color_t bg);
void vga_set_cursor(int x, int y);
void vga_printf(const char *format, ...);

#endif