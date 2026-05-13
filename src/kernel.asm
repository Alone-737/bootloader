[BITS 32]

extern kernel_main

global _kernel_entry

_kernel_entry:
  call kernel_main

  cli
  hlt
  jmp $

times 512-($ - $$) db 0