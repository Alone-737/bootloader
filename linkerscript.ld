ENTRY(_kernel_entry)
OUTPUT_FORMAT(elf32-i386)
SECTIONS{
  . = 0x10000;
  .text : ALIGN(4096)
  {
    *(.text.startup)
    *(.text)
    *(.text.*)
  }
  .usertext : ALIGN(16)
  {
    __usertext_start = .;
    *(.usertext)
    __usertext_end = .;
  }
  .rodata : ALIGN(4096)
  {
    *(.rodata)
    *(.rodata.*)
  }
  .data : ALIGN(4096)
  {
    *(.data)
  }
  .bss : ALIGN(4096)
  {
    _bss_start = .;
    *(COMMON)
    *(.bss)
    _bss_end = .;
  }
}
