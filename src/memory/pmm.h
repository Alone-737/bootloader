#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

void pmm_init(uint32_t mem_size);
void *pmm_allocate_block();
void pmm_free_block(void *ptr);

#endif