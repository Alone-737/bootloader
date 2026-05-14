#include "kernel.h"
#include "shell.h"
#include "idt.h"
#include "pmm.h"

#define DEFAULT_MEM_SIZE (32 * 1024 * 1024)

void shoot_on_your_own_foot()
{
    idt_init();
    vga_init();
    pmm_init(DEFAULT_MEM_SIZE);

    shell_banner();
    shell_run();
}