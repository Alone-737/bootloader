#include <stdint.h>
#include <stddef.h>
#include "gdt.h"

extern void *memset(void *, int, size_t);
extern void gdt_flush(uint32_t);
extern void tss_flush(void);

static struct gdt_entry gdt[6];
static struct gdt_ptr gp;
static struct tss_entry tss;

static void gdt_set_entry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt[num].base_low     = base & 0xFFFF;
    gdt[num].base_middle  = (base >> 16) & 0xFF;
    gdt[num].base_high    = (base >> 24) & 0xFF;
    gdt[num].limit_low    = limit & 0xFFFF;
    gdt[num].granularity  = (limit >> 16) & 0x0F;
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access       = access;
}

void gdt_init(void)
{
    gp.limit = sizeof(gdt) - 1;
    gp.base  = (uint32_t)&gdt;

    gdt_set_entry(0, 0, 0, 0, 0);
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xCF);
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xCF);
    gdt_set_entry(3, 0, 0xFFFFF, 0xFA, 0xCF);
    gdt_set_entry(4, 0, 0xFFFFF, 0xF2, 0xCF);

    uint32_t tss_base = (uint32_t)&tss;
    uint32_t tss_limit = sizeof(tss) - 1;
    memset(&tss, 0, sizeof(tss));
    tss.ss0 = GDT_KERNEL_DATA;
    tss.esp0 = 0x9C00;
    tss.iopb_offset = sizeof(tss);

    gdt_set_entry(5, tss_base, tss_limit, 0x89, 0x00);

    gdt_flush((uint32_t)&gp);
    tss_flush();
}
