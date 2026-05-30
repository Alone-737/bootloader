#include "paging.h"
#include "pmm.h"
#include "errors.h"

typedef struct
{
    uint32_t entries[1024];
} __attribute__((packed)) page_directory_t;

void paging_init(void)
{
    uint32_t cr4;
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= 0x10;
    __asm__ volatile("mov %0, %%cr4" : : "r"(cr4));

    page_directory_t *pd = (page_directory_t *)pmm_allocate_block();
    if (!pd)
        panic("pmm_allocate_block failed");

    for (int i = 0; i < 1024; i++)
        pd->entries[i] = 0;

    for (int i = 0; i < 8; i++)
        pd->entries[i] = (i * 0x400000) | PDE_PRESENT | PDE_RW | PDE_PS;

    __asm__ volatile("mov %0, %%cr3" : : "r"(pd));

    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}
