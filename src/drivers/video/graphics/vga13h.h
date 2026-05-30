#ifndef VGA13H_H
#define VGA13H_H

#include <stdint.h>
#include <stddef.h>

#define VGA13H_WIDTH 320
#define VGA13H_HEIGHT 200
#define VGA13H_FB 0xA0000
#define VGA13H_FONT_W 8
#define VGA13H_FONT_H 8
#define VGA13H_COLS (VGA13H_WIDTH / VGA13H_FONT_W)
#define VGA13H_ROWS (VGA13H_HEIGHT / VGA13H_FONT_H)

void vga13h_init(void);
void vga13h_clear(uint8_t color);
void vga13h_putpixel(int x, int y, uint8_t color);
void vga13h_putchar(int x, int y, char c, uint8_t fg, uint8_t bg);
void vga13h_puts(const char *s);
void vga13h_putchar_console(char c);
void vga13h_set_fg(uint8_t fg);
void vga13h_set_bg(uint8_t bg);
void vga13h_set_cursor(int col, int row);
#endif
