#ifndef KHEAP_H
#define KHEAP_H

#include <stdint.h>
#include <stddef.h>

#define KHEAP_PAGES 8
#define KHEAP_SIZE (KHEAP_PAGES * 4096)
#define KHEAP_START 0x1000000
<<<<<<< HEAD
#define KHEAP_MAGIC 0xDEADBEEF
=======
#define KHEAP_CREATE 0xDEADBEEF
>>>>>>> f7fec4a (added new things and bug fixes)

void kheap_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);

#endif
