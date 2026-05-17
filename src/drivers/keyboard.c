#include "keyboard.h"

void keyboard_handler(void)
{
    unsigned char scancode;
    __asm__ volatile("inb %1, %0" : "=a"(scancode) : "Nd"(0x60));
    (void)scancode;  /* discarded shell and editor poll port 0x60 directly */
}