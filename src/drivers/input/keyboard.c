#include "keyboard.h"
#include "io.h"

#define BUF_SIZE 256

static volatile unsigned char buf[BUF_SIZE];
static volatile int head = 0;
static volatile int tail = 0;

const signed char scancode_map[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0,
};

static const signed char scancode_map_shift[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0,
};

static int shift_pressed = 0;

void keyboard_init(void)
{
    head = 0;
    tail = 0;
    shift_pressed = 0;

    inb(0x60);
}

void keyboard_handler(void)
{
    unsigned char scancode = inb(0x60);

    int next = (head + 1) % BUF_SIZE;
    if (next != tail)
    {
        buf[head] = scancode;
        head = next;
    }
}

int keyboard_data_available(void)
{
    return head != tail;
}

static char translate_scancode(unsigned char scancode)
{
    if (scancode == 0x2A || scancode == 0x36)
    {
        shift_pressed = 1;
        return 0;
    }
    if (scancode == 0xAA || scancode == 0xB6)
    {
        shift_pressed = 0;
        return 0;
    }

    if (scancode & 0x80)
        return 0;

    if (scancode >= 128)
        return 0;

    if (shift_pressed)
        return scancode_map_shift[scancode];

    return scancode_map[scancode];
}

char keyboard_getchar(void)
{
    for (;;)
    {
        while (head == tail)
        {
            __asm__ volatile("sti; hlt");
        }

        unsigned char sc = buf[tail];
        tail = (tail + 1) % BUF_SIZE;

        char c = translate_scancode(sc);
        if (c != 0)
            return c;
    }
}

unsigned char keyboard_read_raw(void)
{
    while (head == tail)
    {
        __asm__ volatile("sti; hlt");
    }

    unsigned char sc = buf[tail];
    tail = (tail + 1) % BUF_SIZE;
    return sc;
}
