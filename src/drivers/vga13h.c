#include "vga13h.h"
#include <stdint.h>

static uint8_t *fb = (uint8_t *)VGA13H_FB;
static int vga_x = 0;
static int vga_y = 0;
static uint8_t vga_fg = 15;
static uint8_t vga_bg = 0;

#include "font8x8.h"

void vga13h_init(void)
{
    vga13h_clear(0);
    vga_x = 0;
    vga_y = 0;
}

void vga13h_clear(uint8_t color)
{
    for (int i = 0; i < VGA13H_WIDTH * VGA13H_HEIGHT; i++)
        fb[i] = color;
    vga_x = 0;
    vga_y = 0;
}

void vga13h_putpixel(int x, int y, uint8_t color)
{
    if (x < 0 || x >= VGA13H_WIDTH || y < 0 || y >= VGA13H_HEIGHT)
        return;
    fb[y * VGA13H_WIDTH + x] = color;
}

void vga13h_putchar(int x, int y, char c, uint8_t fg, uint8_t bg)
{
    int idx = (unsigned char)c - 32;
    if (idx < 0 || idx >= 95)
        idx = 0;

    for (int row = 0; row < VGA13H_FONT_H; row++)
    {
        uint8_t bits = font8x8[idx][row];
        for (int col = 0; col < VGA13H_FONT_W; col++)
        {
            int px = x + col;
            int py = y + row;
            if (px >= 0 && px < VGA13H_WIDTH && py >= 0 && py < VGA13H_HEIGHT)
            {
                if (bits & (0x80 >> col))
                    fb[py * VGA13H_WIDTH + px] = fg;
                else
                    fb[py * VGA13H_WIDTH + px] = bg;
            }
        }
    }
}

static void vga13h_scroll(void)
{
    int row_h = VGA13H_FONT_H;
    for (int y = 0; y < VGA13H_HEIGHT - row_h; y++)
    {
        for (int x = 0; x < VGA13H_WIDTH; x++)
        {
            fb[y * VGA13H_WIDTH + x] = fb[(y + row_h) * VGA13H_WIDTH + x];
        }
    }
    for (int y = VGA13H_HEIGHT - row_h; y < VGA13H_HEIGHT; y++)
    {
        for (int x = 0; x < VGA13H_WIDTH; x++)
        {
            fb[y * VGA13H_WIDTH + x] = vga_bg;
        }
    }
    vga_y--;
}

void vga13h_putchar_console(char c)
{
    if (c == '\n')
    {
        vga_x = 0;
        vga_y++;
    }
    else if (c == '\r')
    {
        vga_x = 0;
    }
    else if (c == '\t')
    {
        vga_x += 4;
    }
    else if (c == '\b')
    {
        if (vga_x > 0)
        {
            vga_x--;
            int px = vga_x * VGA13H_FONT_W;
            int py = vga_y * VGA13H_FONT_H;
            vga13h_putchar(px, py, ' ', vga_fg, vga_bg);
        }
    }
    else if (c >= 32 && c < 127)
    {
        int px = vga_x * VGA13H_FONT_W;
        int py = vga_y * VGA13H_FONT_H;
        vga13h_putchar(px, py, c, vga_fg, vga_bg);
        vga_x++;
    }

    if (vga_x >= VGA13H_COLS)
    {
        vga_x = 0;
        vga_y++;
    }

    while (vga_y >= VGA13H_ROWS)
    {
        vga13h_scroll();
    }
}

void vga13h_puts(const char *s)
{
    if (!s) return;
    for (int i = 0; s[i]; i++)
    {
        if (s[i] == ' ')
        {
            int word_len = 0;
            for (int j = i + 1; s[j] && s[j] != ' ' && s[j] != '\n'; j++)
                word_len++;
            if (vga_x + 1 + word_len >= VGA13H_COLS)
            {
                vga13h_putchar_console('\n');
                continue;
            }
        }
        vga13h_putchar_console(s[i]);
    }
}
void vga13h_set_fg(uint8_t fg)
{
    vga_fg = fg;
}

void vga13h_set_bg(uint8_t bg)
{
    vga_bg = bg;
}

void vga13h_set_cursor(int col, int row)
{
    if (col < 0) col = 0;
    if (row < 0) row = 0;
    if (col >= VGA13H_COLS) col = VGA13H_COLS - 1;
    if (row >= VGA13H_ROWS) row = VGA13H_ROWS - 1;
    vga_x = col;
    vga_y = row;
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

void vga13h_printf(const char *format, ...)
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
