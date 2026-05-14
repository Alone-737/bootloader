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

static int str_eq(const char *a, const char *b)
{
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0')
    {
        if (a[i] != b[i])
        {
            return 0;
        }
        i++;
    }
    return a[i] == b[i];
}

static int str_starts_with(const char *s, const char *prefix)
{
    int i = 0;
    while (prefix[i] != '\0')
    {
        if (s[i] != prefix[i])
        {
            return 0;
        }
        i++;
    }
    return 1;
}

static void print_prompt(void)
{
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("bootsh$ ");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
}

static void shell_banner(void)
{
    vga_init();
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("Bootloader OS shell\n");
    vga_puts("Type 'help' for commands.\n\n");
}

static void shell_exec(const char *line)
{
    int i = 0;
    while (line[i] == ' ' || line[i] == '\t')
    {
        i++;
    }

    const char *cmd = &line[i];
    if (cmd[0] == '\0')
    {
        return;
    }

    if (str_eq(cmd, "help"))
    {
        vga_puts("help  clear  echo  uname  whoami\n");
        return;
    }

    if (str_eq(cmd, "clear"))
    {
        vga_clear();
        return;
    }

    if (str_eq(cmd, "uname"))
    {
        vga_puts("BootloaderOS 0.1 i386\n");
        return;
    }

    if (str_eq(cmd, "whoami"))
    {
        vga_puts("root\n");
        return;
    }

    if (str_starts_with(cmd, "echo "))
    {
        vga_puts(cmd + 5);
        vga_puts("\n");
        return;
    }

    if (str_eq(cmd, "echo"))
    {
        vga_puts("\n");
        return;
    }

    vga_puts("command not found: ");
    vga_puts(cmd);
    vga_puts("\n");
}

void shoot_on_your_own_foot()
{
    shell_banner();

    char line[80];
    int len = 0;
    line[0] = '\0';

    print_prompt();

    for (;;)
    {
        char c = keyboard_getchar();

        if (c == '\n')
        {
            vga_putchar('\n');
            line[len] = '\0';
            shell_exec(line);
            len = 0;
            line[0] = '\0';
            print_prompt();
            continue;
        }

        if (c == '\b')
        {
            if (len > 0)
            {
                len--;
                line[len] = '\0';
                vga_putchar('\b');
                vga_putchar(' ');
                vga_putchar('\b');
            }
            continue;
        }

        if (len < (int)sizeof(line) - 1)
        {
            line[len++] = c;
            line[len] = '\0';
            vga_putchar(c);
        }
    }
}
