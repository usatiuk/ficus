[BITS 64]

%include "task.inc.asm"

extern switch_task
extern gdt_code
extern gdt_null
extern gdt_data

; FIXME: 75% chance this leaks stack or something
section .text
global _yield_self_kern:function (_yield_self_kern.end - _yield_self_kern)
_yield_self_kern:
    pop rsi ; save the return ip
    mov r8, rsp ; save cur sp

    mov r10, gdt_null
    mov r9, gdt_data
    mov r11, gdt_code

    sub r9, r10
    sub r11, r10

    PUSH r9; Push data segment
    push r8 ; current sp
    pushf ; eflags
    PUSH r11; Push code segment
    push rsi ; instruction address to return to
    pushaq

    mov rdi, 0xdeadbe3fdeadb3ef ; IDT_GUARD
    push rdi ; IDT_GUARD

    ; pass the "pointer" to the stack as pointer to the interrupt_frame argument,
    ; the stack and the struct must match!
    mov rdi, rsp

    call switch_task
    add rsp, 8 ; remove IDT_GUARD
    popaq
    iretq
.end:
