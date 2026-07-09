#include "syscall.h"
#include "vga.h"

typedef struct
{
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
} syscall_regs_t;

static uint32_t sys_write(syscall_regs_t *r)
{
    (void)r;
    if (r->ecx && r->edx)
    {
        for (uint32_t i = 0; i < r->edx; i++)
        {
            char c = *((char *)r->ecx + i);
            if (c == '\0') break;
            vga_putchar(c);
        }
    }
    return r->edx;
}

static uint32_t sys_read(syscall_regs_t *r)
{
    (void)r;
    return 0;
}

static uint32_t sys_getpid(syscall_regs_t *r)
{
    (void)r;
    return 1;
}

static uint32_t sys_yield(syscall_regs_t *r)
{
    (void)r;
    return 0;
}

static uint32_t sys_exit(syscall_regs_t *r)
{
    (void)r;
    vga_puts("\n[user] exit called\n");
    for (;;) __asm__ volatile("hlt");
    return 0;
}

typedef uint32_t (*syscall_fn_t)(syscall_regs_t *);

static syscall_fn_t syscall_table[32] = {0};

void syscall_init(void)
{
    syscall_table[SYS_WRITE]  = sys_write;
    syscall_table[SYS_READ]   = sys_read;
    syscall_table[SYS_GETPID] = sys_getpid;
    syscall_table[SYS_YIELD]  = sys_yield;
    syscall_table[SYS_EXIT]   = sys_exit;
}

void syscall_handler(syscall_regs_t *regs)
{
    if (regs->eax >= 32 || syscall_table[regs->eax] == 0)
    {
        vga_puts("[syscall] invalid number: ");
        vga_printf("%d\n", regs->eax);
        regs->eax = 0xFFFFFFFF;
        return;
    }

    regs->eax = syscall_table[regs->eax](regs);
}