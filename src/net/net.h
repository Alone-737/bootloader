#ifndef NET_H
#define NET_H

#include <stdint.h>

int net_init(void);

/* Sends `count` echo requests to `dst` (a.b.c.d) and prints the result.
   Returns the number of replies received, -1 if the host is unreachable. */
int net_ping(const uint8_t dst[4], int count);

#endif
