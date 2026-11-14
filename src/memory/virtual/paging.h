#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PDE_PRESENT 0x1
#define PDE_RW      0x2
#define PDE_USER    0x4
#define PDE_PS      0x80

/* Ring-3 window: one 4 MiB identity-mapped PDE (PDE 1). */
#define USER_BASE  0x400000u
#define USER_LIMIT (USER_BASE + 0x400000u)

void paging_init(void);

#endif
