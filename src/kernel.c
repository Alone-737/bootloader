#include "kernel.h"

static inline unsigned char inb(unsigned short port)
{
    unsigned char value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static const char scancode_map[128] = {
    0,
    27,
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',
    '=',
    '\b',
    '\t',
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    '\n',
    0,
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',
    '\'',
    '`',
    0,
    '\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    0,
    '*',
    0,
    ' ',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    '7',
    '8',
    '9',
    '-',
    '4',
    '5',
    '6',
    '+',
    '1',
    '2',
    '3',
    '0',
    '.',
    0,
    0,
};

static char keyboard_getchar(void)
{
    for (;;)
    {
        while ((inb(0x64) & 0x01) == 0)
        {
        }

        unsigned char scancode = inb(0x60);
        if (scancode & 0x80)
        {
            continue;
        }

        char c = scancode_map[scancode];
        if (c != 0)
        {
            return c;
        }
    }
}

void keyboard_handler(void)
{
    unsigned char scancode = inb(0x60);
    (void)scancode;
}

static void clear_login_area(void)
{
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_set_cursor(0, 0);
    vga_puts("=== Bootloader Initialized ===\n\n");

    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);

    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_set_cursor(0, 8);
    vga_puts("login: ");
    vga_set_cursor(7, 8);

    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_set_cursor(50, 2);
    vga_puts("User Panel");

    vga_set_cursor(50, 4);
    vga_puts("User: ");
}

static void clear_login_input(int length)
{
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_set_cursor(7, 8);
    for (int i = 0; i < length; i++)
    {
        vga_putchar(' ');
    }
    vga_set_cursor(7 + length, 8);
}

static void update_user_panel(const char *username)
{
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_set_cursor(56, 4);
    vga_puts("                    ");
    vga_set_cursor(56, 4);
    vga_puts(username);

    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_set_cursor(50, 6);
    vga_puts("Welcome, ");
    vga_puts(username);
    vga_puts("    ");
}

static void redraw_login_input(const char *username)
{
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_set_cursor(7, 8);
    vga_puts("                              ");
    vga_set_cursor(7, 8);
    vga_puts(username);
}

void kernel_main()
{
    vga_init();

    clear_login_area();

    char username[32];
    int username_length = 0;
    username[0] = '\0';

    for (;;)
    {
        char c = keyboard_getchar();
        if (c == '\n')
        {
            username[username_length] = '\0';
            update_user_panel(username);
            redraw_login_input(username);
            vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
            vga_set_cursor(0, 10);
            vga_puts("Logged in as: ");
            vga_puts(username);
            vga_puts("\n");
            continue;
        }

        if (c == '\b')
        {
            if (username_length > 0)
            {
                username_length--;
                username[username_length] = '\0';
                clear_login_input(username_length);
                redraw_login_input(username);
            }
            continue;
        }

        if (username_length < (int)sizeof(username) - 1)
        {
            username[username_length++] = c;
            username[username_length] = '\0';
            redraw_login_input(username);
        }

        update_user_panel(username);
    }
}
