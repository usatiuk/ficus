[BITS 64]

%include "task.inc.asm"

extern switch_task

; FIXME: 75% chance this leaks stack or something
section .text
global _yield_self_kern:function (_yield_self_kern.end - _yield_self_kern)
_yield_self_kern:
    pop rsi ; save the return ip
    mov r8, rsp ; save cur sp

    mov r9, ss
    mov r11, cs

    PUSH r9; Push data segment
    push r8 ; current sp
    pushf ; eflags
    PUSH r11; Push code segment
    push rsi ; instruction address to return to


    pushaq

    ; pass the "pointer" to the stack as pointer to the interrupt_frame argument,
    ; the stack and the struct must match!
    mov rdi, rsp

    call switch_task

    popaq
    iretq
.end:
