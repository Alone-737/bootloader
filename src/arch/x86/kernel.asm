[BITS 32]

section .text.startup

extern shoot_on_your_own_foot

global _kernel_entry

_kernel_entry:
  call shoot_on_your_own_foot

  cli
  hlt
  jmp $

<<<<<<< HEAD
times 512-($ - $$) db 0
=======
times 512-($ - $$) db 0  ; pad to 512 bytes so boot.asm can load exactly N sectors
>>>>>>> f7fec4a (added new things and bug fixes)
