#include "idt.h"
#include "io.h"

struct idt_entry idt[256];
struct idt_ptr idtp;

extern void idt_load(uint32_t);

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1


void idt_set_gate(int n, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt[n].base_low = base & 0xFFFF;
    idt[n].base_high = (base >> 16) & 0xFFFF;
    idt[n].selector = sel;
    idt[n].always0 = 0;
    idt[n].flags = flags;
}

void pic_remap()
{
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    outb(PIC1_DATA, 0xFC);
    outb(PIC2_DATA, 0xFF);
}

void idt_init()
{
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    for (int i = 0; i < 256; i++)
    {
        idt_set_gate(i, 0, 0, 0);
    }

    extern void exception_handler_0();
    extern void exception_handler_1();
    extern void exception_handler_2();
    extern void exception_handler_3();
    extern void exception_handler_4();
    extern void exception_handler_5();
    extern void exception_handler_6();
    extern void exception_handler_7();
    extern void exception_handler_8();
    extern void exception_handler_9();
    extern void exception_handler_10();
    extern void exception_handler_11();
    extern void exception_handler_12();
    extern void exception_handler_13();
    extern void exception_handler_14();
    extern void exception_handler_15();
    extern void exception_handler_16();
    extern void exception_handler_17();
    extern void exception_handler_18();
    extern void exception_handler_19();
    extern void exception_handler_20();
    extern void exception_handler_21();
    extern void exception_handler_22();
    extern void exception_handler_23();
    extern void exception_handler_24();
    extern void exception_handler_25();
    extern void exception_handler_26();
    extern void exception_handler_27();
    extern void exception_handler_28();
    extern void exception_handler_29();
    extern void exception_handler_30();
    extern void exception_handler_31();

    idt_set_gate(0, (uint32_t)exception_handler_0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)exception_handler_1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t)exception_handler_2, 0x08, 0x8E);
    idt_set_gate(3, (uint32_t)exception_handler_3, 0x08, 0x8E);
    idt_set_gate(4, (uint32_t)exception_handler_4, 0x08, 0x8E);
    idt_set_gate(5, (uint32_t)exception_handler_5, 0x08, 0x8E);
    idt_set_gate(6, (uint32_t)exception_handler_6, 0x08, 0x8E);
    idt_set_gate(7, (uint32_t)exception_handler_7, 0x08, 0x8E);
    idt_set_gate(8, (uint32_t)exception_handler_8, 0x08, 0x8E);
    idt_set_gate(9, (uint32_t)exception_handler_9, 0x08, 0x8E);
    idt_set_gate(10, (uint32_t)exception_handler_10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)exception_handler_11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)exception_handler_12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)exception_handler_13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)exception_handler_14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)exception_handler_15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)exception_handler_16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)exception_handler_17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)exception_handler_18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)exception_handler_19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)exception_handler_20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)exception_handler_21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)exception_handler_22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)exception_handler_23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)exception_handler_24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)exception_handler_25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)exception_handler_26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)exception_handler_27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)exception_handler_28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)exception_handler_29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)exception_handler_30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)exception_handler_31, 0x08, 0x8E);

    pic_remap();

    extern void keyboard_handler_asm();
    idt_set_gate(0x21, (uint32_t)keyboard_handler_asm, 0x08, 0x8E);

    extern void timer_handler_asm();
    idt_set_gate(0x20, (uint32_t)timer_handler_asm, 0x08, 0x8E);

    idt_load((uint32_t)&idtp);
}