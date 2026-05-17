#include "pmm.h"

#define MAX_BLOCKS 32768  
static uint32_t bitmap[MAX_BLOCKS / 32];
static uint32_t total_blocks = 0;

static void bitmap_set(uint32_t bit) {
    bitmap[bit / 32] |= (1 << (bit % 32));
}

static void bitmap_unset(uint32_t bit) {
    bitmap[bit / 32] &= ~(1 << (bit % 32));
}

static int bitmap_test(uint32_t bit) {
    return bitmap[bit / 32] & (1 << (bit % 32));
}

void pmm_init(uint32_t mem_size) {
    total_blocks = mem_size / PAGE_SIZE;
    for (uint32_t i = 0; i < MAX_BLOCKS / 32; i++) {
        bitmap[i] = 0;
    }

    for (uint32_t i = 0; i < 256; i++) {
        bitmap_set(i);  /* reserve first 1 MB (0x00000000 – 0x000FFFFF) for BIOS/kernel */
    }
}

void *pmm_allocate_block() {
    for (uint32_t i = 0; i < total_blocks; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            return (void *)(i * PAGE_SIZE);
        }
    }
    return NULL;
}

void pmm_free_block(void *ptr) {
    uint32_t block = (uint32_t)ptr / PAGE_SIZE;
    bitmap_unset(block);
}

void pmm_mark_used(void *addr) {
    uint32_t block = (uint32_t)addr / PAGE_SIZE;
    bitmap_set(block);
}

int pmm_is_used(void *addr) {
    uint32_t block = (uint32_t)addr / PAGE_SIZE;
    return bitmap_test(block);
}