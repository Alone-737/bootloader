; first ELF user program: exec hello.elf in the shell.
BITS 32
global _start

section .text
_start:
  mov eax, 4            ; SYS_WRITE
  mov ebx, msg
  mov ecx, msg.len
  int 0x80

  mov eax, 1            ; SYS_EXIT
  xor ebx, ebx
  int 0x80

section .data
msg: db "hello from ELF, ring 3", 10
  .len equ $ - msg
