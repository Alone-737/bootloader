#ifndef KEYBOARD_H
#define KEYBOARD_H

void keyboard_init(void);
void keyboard_handler(void);
/* Returned by keyboard_translate for E0-prefixed arrow keys; the map
   never emits these values. */
#define KEY_UP   0x01
#define KEY_DOWN 0x02

char keyboard_getchar(void);
char keyboard_translate(unsigned char scancode);
unsigned char keyboard_read_raw(void);
int keyboard_data_available(void);

extern const signed char scancode_map[128];

#endif
