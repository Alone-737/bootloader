#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PDE_PRESENT 0x1
#define PDE_RW      0x2
#define PDE_USER    0x4
#define PDE_PS      0x80

void paging_init(void);

#endif
