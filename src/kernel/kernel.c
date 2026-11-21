#include "kernel.h"
#include "shell.h"
#include "idt.h"
#include "gdt.h"
#include "pmm.h"
#include "paging.h"
#include "kheap.h"
#include "vfs.h"
#include "romfs.h"
#include "procfs.h"
#include "devfs.h"
#include "keyboard.h"
#include "timer.h"
#include "syscall.h"
#include "net.h"

extern char _bss_start;
extern char _bss_end;

#define DEFAULT_MEM_SIZE (256 * 1024 * 1024)

void kernel_start()
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
    procfs_init();
    devfs_init();
    syscall_init();
    vga_clear();
    vga_set_cursor(0, 0);
    net_init();

    shell_banner();

    /* The shell is PID 1 now. proc_init_shell also creates PID 0 (idle) and
     * returns the prepared kernel stack for the shell task. */
    extern uint32_t proc_init_shell(uint32_t shell_entry);
    extern void sched_start(uint32_t sp);
    uint32_t sp = proc_init_shell((uint32_t)shell_run);
    sched_start(sp);
}