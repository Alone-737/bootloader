[BITS 16]
[ORG 0x7c00]

CODE_OFFSET equ 0x8
DATA_OFFSET equ 0x10

KERNEL_LOAD_SEG equ 0x1000
KERNEL_LOAD_OFFSET equ 0x0000
KERNEL_START_ADDR equ 0x10000
KERNEL_SECTORS equ 25

start:
  cli ;Clear interrupts
  mov ax, 0x00 
  mov ds , ax
  mov es , ax
  mov ss , ax
  mov sp , 0x7c00
  mov [boot_drive], dl
  sti ; Enable interrupts

  ;LOAD KERNEL

  mov bx, KERNEL_LOAD_SEG
  mov es, bx
  mov bx, KERNEL_LOAD_OFFSET
  mov dh, 0x00
  mov dl, [boot_drive]
  mov cl, 0x02
  mov ch, 0x00
  mov ah, 0x02
  mov al, KERNEL_SECTORS
  int 0x13

  jc disk_read_error

load_PM:
  cli 
  lgdt[gdt_descriptor]
  mov eax, cr0
  or al, 1
  mov cr0, eax
  jmp CODE_OFFSET:PModeMain

disk_read_error:
  hlt


;GDT Implemetation
gdt_start:
  dd 0x00000000
  dd 0x00000000

  ;Code Segment descriptor
  dw 0xFFFF
  dw 0x0000
  db 0x00
  db 10011010b
  db 11001111b
  db 0x00

  ;Data Segment descriptor
  dw 0xFFFF
  dw 0x0000
  db 0x00
  db 10010010b
  db 11001111b
  db 0x00

gdt_end:
  
gdt_descriptor:
  dw gdt_end - gdt_start - 1
  dd gdt_start

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
  
  in al, 0x92
  or al, 2
  out 0x92, al

  jmp CODE_OFFSET:KERNEL_START_ADDR

boot_drive db 0

times 510 - ($ - $$) db 0

dw 0xAA55
