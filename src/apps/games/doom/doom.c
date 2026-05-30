#include "doom.h"
#include "keyboard.h"
#include "keys.h"
#include "timer.h"
#include "vga13h.h"
#include "io.h"

#include <stdint.h>

static inline int _nearest6(uint8_t c) {
  if (c < 26)
    return 0;
  if (c < 77)
    return 1;
  if (c < 128)
    return 2;
  if (c < 179)
    return 3;
  if (c < 230)
    return 4;
  return 5;
}

static inline uint8_t _rgb_to_vga(uint32_t argb) {
  uint8_t r = (argb >> 16) & 0xFF;
  uint8_t g = (argb >> 8) & 0xFF;
  uint8_t b = argb & 0xFF;
  return (uint8_t)(16 + 36 * _nearest6(r) + 6 * _nearest6(g) + _nearest6(b));
}

static void _setup_palette(void) {
  outb(0x3C8, 16);
  for (int r = 0; r < 6; r++)
    for (int g = 0; g < 6; g++)
      for (int b = 0; b < 6; b++) {
        outb(0x3C9, (uint8_t)(r * 51 / 4));
        outb(0x3C9, (uint8_t)(g * 51 / 4));
        outb(0x3C9, (uint8_t)(b * 51 / 4));
      }
}

#define KEYQUEUE_SIZE 32
static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWrite = 0;
static unsigned int s_KeyQueueRead = 0;

static unsigned char _scancode_to_doomkey(unsigned char sc) {
  switch (sc) {
  case 0x48:
    return DK_UPARROW;
  case 0x50:
    return DK_DOWNARROW;
  case 0x4B:
    return DK_LEFTARROW;
  case 0x4D:
    return DK_RIGHTARROW;
  case 0x01:
    return DK_ESCAPE;
  case 0x1C:
    return DK_ENTER;
  case 0x0F:
    return DK_TAB;
  case 0x39:
    return DK_USE;
  case 0x1D:
    return DK_FIRE;
  case 0x2A:
    return DK_RSHIFT;
  case 0x36:
    return DK_RSHIFT;
  case 0x38:
    return DK_RALT;
  case 0x3B:
    return DK_F1;
  case 0x3C:
    return DK_F2;
  case 0x3D:
    return DK_F3;
  case 0x3E:
    return DK_F4;
  case 0x3F:
    return DK_F5;
  case 0x40:
    return DK_F6;
  case 0x41:
    return DK_F7;
  case 0x42:
    return DK_F8;
  case 0x43:
    return DK_F9;
  case 0x44:
    return DK_F10;
  case 0x57:
    return DK_F11;
  case 0x58:
    return DK_F12;
  case 0x02:
    return '1';
  case 0x03:
    return '2';
  case 0x04:
    return '3';
  case 0x05:
    return '4';
  case 0x06:
    return '5';
  case 0x07:
    return '6';
  case 0x08:
    return '7';
  case 0x09:
    return '8';
  case 0x0A:
    return '9';
  case 0x0B:
    return '0';
  case 0x10:
    return 'q';
  case 0x11:
    return 'w';
  case 0x12:
    return 'e';
  case 0x13:
    return 'r';
  case 0x14:
    return 't';
  case 0x15:
    return 'y';
  case 0x16:
    return 'u';
  case 0x17:
    return 'i';
  case 0x18:
    return 'o';
  case 0x19:
    return 'p';
  case 0x1E:
    return 'a';
  case 0x1F:
    return 's';
  case 0x20:
    return 'd';
  case 0x21:
    return 'f';
  case 0x22:
    return 'g';
  case 0x23:
    return 'h';
  case 0x24:
    return 'j';
  case 0x25:
    return 'k';
  case 0x26:
    return 'l';
  case 0x2C:
    return 'z';
  case 0x2D:
    return 'x';
  case 0x2E:
    return 'c';
  case 0x2F:
    return 'v';
  case 0x30:
    return 'b';
  case 0x31:
    return 'n';
  case 0x32:
    return 'm';
  case 0x0C:
    return DK_MINUS;
  case 0x0D:
    return DK_EQUALS;
  default:
    return 0;
  }
}

static void _enqueue_key(int pressed, unsigned char doomkey) {
  if (doomkey == 0)
    return;
  s_KeyQueue[s_KeyQueueWrite] = (unsigned short)((pressed << 8) | doomkey);
  s_KeyQueueWrite = (s_KeyQueueWrite + 1) % KEYQUEUE_SIZE;
}

static void _poll_keyboard(void) {
  while (keyboard_data_available()) {
    unsigned char sc = keyboard_read_raw();
    int is_break = (sc & 0x80) != 0;
    _enqueue_key(is_break ? 0 : 1, _scancode_to_doomkey(sc & 0x7F));
  }
}

void DG_Init(void) {
  vga13h_init();
  _setup_palette();
}

typedef uint32_t pixel_t;
pixel_t *DG_ScreenBuffer = 0;

void DG_DrawFrame(void) {
  uint8_t *fb = (uint8_t *)VGA13H_FB;
  pixel_t *src = DG_ScreenBuffer;
  for (int i = 0; i < DOOMGENERIC_RESX * DOOMGENERIC_RESY; i++)
    fb[i] = _rgb_to_vga(src[i]);
  _poll_keyboard();
}

void DG_SleepMs(uint32_t ms) {
  uint32_t start = timer_get_ticks();
  uint32_t need = (ms + 9) / 10;
  while ((timer_get_ticks() - start) < need)
    __asm__ volatile("hlt");
}

uint32_t DG_GetTicksMs(void) { return timer_get_ticks() * 10; }

int DG_GetKey(int *pressed, unsigned char *doomKey) {
  if (s_KeyQueueRead == s_KeyQueueWrite)
    return 0;
  unsigned short data = s_KeyQueue[s_KeyQueueRead];
  s_KeyQueueRead = (s_KeyQueueRead + 1) % KEYQUEUE_SIZE;
  *pressed = (data >> 8) & 0xFF;
  *doomKey = data & 0xFF;
  return 1;
}

void DG_SetWindowTitle(const char *title) { (void)title; }

void doom_run(void) {
  // static char *argv[] = { "doom", 0 };
  // doomgeneric_Create(1, argv);
  // for (;;)
  //     doomgeneric_Tick();
  vga13h_init();
  vga13h_clear(0);
  // Print a small message or just halt
  for (;;)
    ;
}
