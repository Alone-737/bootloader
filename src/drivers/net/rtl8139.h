#ifndef RTL8139_H
#define RTL8139_H

#include <stdint.h>

/* 0 on success, -1 when no card was found. */
int rtl8139_init(void);
void rtl8139_mac(uint8_t out[6]);

/* 0 on success, -1 on timeout / not initialised. */
int rtl8139_send(const void *frame, uint16_t len);

/* Length of the next received frame, 0 when the buffer is empty. */
int rtl8139_recv(void *buf, int max);

#endif
