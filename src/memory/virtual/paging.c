#include "paging.h"
#include "pmm.h"
#include "errors.h"

typedef struct
{
    uint32_t entries[1024];
} __attribute__((packed)) page_directory_t;

extern char _bss_end;

void paging_init(void)
{
    if ((uint32_t)&_bss_end > USER_BASE)
        panic("paging: kernel overlaps the user window");

    uint32_t cr4;
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= 0x10;
    __asm__ volatile("mov %0, %%cr4" : : "r"(cr4));

    page_directory_t *pd = (page_directory_t *)pmm_allocate_block();
    if (!pd)
        panic("pmm_allocate_block failed");

    for (int i = 0; i < 1024; i++)
        pd->entries[i] = 0;

    pd->entries[0] = PDE_PRESENT | PDE_RW | PDE_PS;
    for (int i = 1; i < 64; i++)
        pd->entries[i] = (i * 0x400000) | PDE_PRESENT | PDE_RW | PDE_PS;

    /* shortcut: exactly one 4 MiB user PDE; add page tables if the window grows. */
    pd->entries[USER_BASE >> 22] |= PDE_USER;
    for (uint32_t a = USER_BASE; a < USER_LIMIT; a += PAGE_SIZE)
        pmm_mark_used((void *)a);

    __asm__ volatile("mov %0, %%cr3" : : "r"(pd));

    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}
