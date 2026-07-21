#include "syscall.h"
#include "vga.h"

extern uint32_t ring3_return_esp;
extern uint32_t ring3_return_ebp;

typedef struct
{
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
} syscall_regs_t;

#define USER_ADDR_MIN 0x100000U
#define USER_ADDR_MAX 0x10000000U

static int valid_user_ptr(uint32_t addr, uint32_t len)
{
    if (addr < USER_ADDR_MIN)
        return 0;
    if (addr > USER_ADDR_MAX || len > USER_ADDR_MAX - addr)
        return 0;
    return 1;
}

static uint32_t sys_write(syscall_regs_t *r)
{
    uint32_t ptr = r->ecx;
    uint32_t len = r->edx;

    if (!ptr || !len)
        return 0;

    if (!valid_user_ptr(ptr, len))
    {
        vga_puts("[syscall] sys_write: invalid user pointer\n");
        return (uint32_t)-1;
    }

    for (uint32_t i = 0; i < len; i++)
    {
        char c = *((char *)ptr + i);
        vga_putchar(c);
    }
    return len;
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
    vga_puts("\n[user] exit called, returning to shell\n");

    if (ring3_return_esp && ring3_return_ebp)
    {
        __asm__ volatile(
            "mov %0, %%ebp\n"
            "mov %1, %%esp\n"
            "ret\n"
            : : "r"(ring3_return_ebp), "r"(ring3_return_esp)
        );
    }

    vga_puts("[syscall] sys_exit: no return context, halting\n");
    for (;;) __asm__ volatile("cli; hlt");
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