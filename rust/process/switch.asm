[BITS 32]

; Start the first task. Argument on the stack is the prepared kernel stack
; pointer returned by proc_init_shell. That stack already holds a pushad frame
; and an iretd frame, so this never returns.
global sched_start
sched_start:
    mov eax, [esp + 4]
    mov esp, eax
    popad
    iretd
