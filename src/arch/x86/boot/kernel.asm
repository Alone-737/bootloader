[BITS 32]

section .text.startup

extern kernel_start

global _kernel_entry

_kernel_entry:
  call kernel_start

  cli
  hlt
  jmp $

times 512-($ - $$) db 0  ; pad to 512 bytes so boot.asm can load exactly N sectors
