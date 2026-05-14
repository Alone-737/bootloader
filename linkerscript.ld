ENTRY(_kernel_entry)
OUTPUT_FORMAT(elf32-i386)
SECTIONS{
  . = 0x10000;
  .text : ALIGN(4096)
  {
    *(.text.startup)
    *(.text)
  }
  .rodata : ALIGN(4096)
  {
    *(.rodata)
  }
  .data : ALIGN(4096)
  {
    *(.data)
  }
  .bss : ALIGN(4096)
  {
    *(COMMON)
    *(.bss)
  }
}
