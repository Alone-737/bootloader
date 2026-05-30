#ifndef KEYBOARD_H
#define KEYBOARD_H

void keyboard_init(void);
void keyboard_handler(void);
char keyboard_getchar(void);
unsigned char keyboard_read_raw(void);
int keyboard_data_available(void);

extern const signed char scancode_map[128];

#endif
