[BITS 32]

global idt_load
idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret

extern exception_handler

%macro EXCEPTION_NOERR 1
global exception_handler_%1
exception_handler_%1:
    push dword 0
    push dword %1
    jmp exception_common_stub
%endmacro

%macro EXCEPTION_ERR 1
global exception_handler_%1
exception_handler_%1:
    push dword %1
    jmp exception_common_stub
%endmacro

EXCEPTION_NOERR 0
EXCEPTION_NOERR 1
EXCEPTION_NOERR 2
EXCEPTION_NOERR 3
EXCEPTION_NOERR 4
EXCEPTION_NOERR 5
EXCEPTION_NOERR 6
EXCEPTION_NOERR 7
EXCEPTION_ERR   8
EXCEPTION_NOERR 9
EXCEPTION_ERR   10
EXCEPTION_ERR   11
EXCEPTION_ERR   12
EXCEPTION_ERR   13
EXCEPTION_ERR   14
EXCEPTION_NOERR 15
EXCEPTION_NOERR 16
EXCEPTION_NOERR 17
EXCEPTION_NOERR 18
EXCEPTION_NOERR 19
EXCEPTION_NOERR 20
EXCEPTION_NOERR 21
EXCEPTION_NOERR 22
EXCEPTION_NOERR 23
EXCEPTION_NOERR 24
EXCEPTION_NOERR 25
EXCEPTION_NOERR 26
EXCEPTION_NOERR 27
EXCEPTION_NOERR 28
EXCEPTION_NOERR 29
EXCEPTION_NOERR 30
EXCEPTION_NOERR 31

exception_common_stub:
    pushad
    cld
    
    push dword [esp + 36]  ; error code
    push dword [esp + 36]  ; exception number
    call exception_handler
    add esp, 8
    
    popad
    add esp, 8       ; pop exception number and error code pushed by macro
    iretd

global keyboard_handler_asm
extern keyboard_handler
keyboard_handler_asm:
    pushad
    cld
    call keyboard_handler
    mov al, 0x20
    out 0x20, al
    popad
    iretd

global timer_handler_asm
extern timer_handler
timer_handler_asm:
    pushad
    cld
    call timer_handler
    mov al, 0x20
    out 0x20, al
    popad
    iretd

global gdt_flush
gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.flush
.flush:
    ret

global tss_flush
tss_flush:
    mov ax, 0x28
    ltr ax
    ret

extern syscall_handler
global syscall_handler_asm
syscall_handler_asm:
    pushad
    cld
    push esp
    call syscall_handler
    add esp, 4
    popad
    iretd

global enter_ring3
enter_ring3:
    mov eax, [esp + 4]
    mov ebx, [esp + 8]
    mov ecx, [esp + 12]
    cli
    mov esp, ebx
    push dword 0x23
    push dword ebx
    push dword ecx
    push dword 0x1B
    push dword eax
    iretd

global ring3_switch
ring3_switch:
    ; args: [esp+4] = user_eip, [esp+8] = user_esp
    mov eax, [esp + 4]   ; user_eip
    mov ebx, [esp + 8]   ; user_esp
    
    cli
    ; switch to user stack and build iret frame
    mov esp, ebx
    push dword 0x23      ; user SS
    push ebx             ; user ESP
    pushfd
    pop ecx
    or ecx, 0x3000       ; IF=1, IOPL=3
    push ecx             ; EFLAGS
    push dword 0x1B      ; user CS
    push eax             ; user EIP
    iretd