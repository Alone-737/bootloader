#include "kernel.h"
#include "shell.h"
#include "idt.h"
#include "pmm.h"
#include "kheap.h"
#include "vfs.h"

extern char _bss_start;
extern char _bss_end;

#define DEFAULT_MEM_SIZE (32 * 1024 * 1024)

void shoot_on_your_own_foot()
{
    char *bss = &_bss_start;
    while (bss < &_bss_end)
        *bss++ = 0;

    idt_init();
    __asm__ volatile("sti"); 
    vga_init();
    pmm_init(DEFAULT_MEM_SIZE);
    kheap_init();
    vfs_init();

    shell_banner();
    shell_run();
}
