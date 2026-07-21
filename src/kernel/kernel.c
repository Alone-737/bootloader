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
#include "syscall.h"

extern char _bss_start;
extern char _bss_end;

#define DEFAULT_MEM_SIZE (256 * 1024 * 1024)



void ring3_test_entry(void)
{
    vga_puts("Ring 3: Hello from user mode!\n");
    
    int ret = 0;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(0), "b"("Ring 3: syscall write works!\n"), "c"(25)
        : "memory"
    );
    vga_printf("Ring 3: sys_write returned %d\n", ret);
    
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(1)
        : "memory"
    );
    vga_printf("Ring 3: sys_getpid returned %d\n", ret);
    
    vga_puts("Ring 3: exiting via sys_exit\n");
    __asm__ volatile(
        "int $0x80"
        : : "a"(4)
        : "memory"
    );
}

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
    syscall_init();
    vga_clear();
    vga_set_cursor(0, 0);

    shell_banner();
    shell_run();
}