[BITS 16]
[ORG 0x7c00]

CODE_OFFSET equ 0x8
DATA_OFFSET equ 0x10

KERNEL_LOAD_SEG equ 0x1000
KERNEL_LOAD_OFFSET equ 0x0000
KERNEL_START_ADDR equ 0x10000
%ifndef KERNEL_SECTORS
%define KERNEL_SECTORS 57
%endif

start:
  cli
  mov ax, 0x00 
  mov ds , ax
  mov es , ax
  mov ss , ax
  mov sp , 0x7c00
  mov [boot_drive], dl
  sti

  ; LOAD KERNEL. BIOS int 0x13/ah=2 cannot transfer more than 127 sectors in
  ; one call. Read one sector at a time so a larger kernel still boots.
  mov bx, KERNEL_LOAD_SEG
  mov es, bx
  mov bx, KERNEL_LOAD_OFFSET
  mov byte [read_sector], 0x02
  mov byte [read_head], 0x00
  mov word [read_remaining], KERNEL_SECTORS

.read_loop:
  xor cx, cx
  mov cl, [read_sector]
  mov dh, [read_head]
  mov dl, [boot_drive]
  mov ah, 0x02
  mov al, 0x01
  int 0x13
  jc disk_read_error

  mov ax, es
  add ax, 0x20          ; next sector = 512 bytes = 32 paragraphs
  mov es, ax

  inc byte [read_sector]
  cmp byte [read_sector], 64
  jb .next_sector
  mov byte [read_sector], 1
  inc byte [read_head]
.next_sector:
  dec word [read_remaining]
  jnz .read_loop

  mov ax, 0x0013
  int 0x10

load_PM:
  cli
  lgdt[gdt_descriptor]
  mov eax, cr0
  or al, 1
  mov cr0, eax
  jmp CODE_OFFSET:PModeMain

disk_read_error:
  hlt
  jmp disk_read_error

[BITS 32]
PModeMain:
  mov ax, DATA_OFFSET
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov ss, ax
  mov gs, ax
  mov ebp, 0x9C00
  mov esp, ebp
  
  in al, 0x92       ; enable A20 gate via fast method
  or al, 2
  out 0x92, al

  jmp CODE_OFFSET:KERNEL_START_ADDR

boot_drive db 0
read_sector db 0
read_head db 0
read_remaining dw 0

%include "gdt.asm"

times 510 - ($ - $$) db 0

dw 0xAA55