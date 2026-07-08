#include "kernel.h"
#include "shell.h"
#include "idt.h"
#include "gdt.h"
#include "pmm.h"
#include "paging.h"
#include "kheap.h"
#include "vfs.h"
#include "romfs.h"
#include "keyboard.h"
#include "timer.h"

extern char _bss_start;
extern char _bss_end;

#define DEFAULT_MEM_SIZE (256 * 1024 * 1024)

void shoot_on_your_own_foot()
{
    char *bss = &_bss_start;
    while (bss < &_bss_end)
        *bss++ = 0;

    idt_init();
    gdt_init();
    __asm__ volatile("sti");
    vga_init();
    pmm_init(DEFAULT_MEM_SIZE);
    kheap_init();
    paging_init();
    keyboard_init();
    timer_init();
    vfs_init();
    romfs_init();
    vga_clear();
    vga_set_cursor(0, 0);

    shell_banner();
    shell_run();
}
