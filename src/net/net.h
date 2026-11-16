#ifndef NET_H
#define NET_H

#include <stdint.h>

int net_init(void);

/* Sends `count` echo requests to `dst` (a.b.c.d) and prints the result.
   Returns the number of replies received, -1 if the host is unreachable. */
int net_ping(const uint8_t dst[4], int count);

/* Resolves a hostname to an IPv4 address via DNS. Returns 0 on success. */
int net_resolve(const char *name, uint8_t out_ip[4]);

#endif
