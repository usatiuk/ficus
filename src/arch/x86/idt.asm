[BITS 64]

%include "task.inc.asm"

section .text
%macro isr_err_stub 1
isr_stub_%+%1:
    pop rdi ; Keep the stacktrace
    call exception_handler
    iretq
%endmacro

%macro isr_no_err_stub 1
isr_stub_%+%1:
    call exception_handler
    iretq
%endmacro

extern exception_handler
isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

section .text
%macro pic1_irq 1
extern pic1_irq_real_%+%1
global pic1_irq_%+%1
pic1_irq_%+%1:
    pushaq
    call pic1_irq_real_%+%1
    popaq
    iretq
%endmacro

%macro pic2_irq 1
extern pic2_irq_real_%+%1
global pic2_irq_%+%1
pic2_irq_%+%1:
    pushaq
    call pic2_irq_real_%+%1
    popaq
    iretq
%endmacro

extern pic1_irq_real_0
global pic1_irq_0
pic1_irq_0:
    pushaq

    ; pass the "pointer" to the stack as pointer to the interrupt_frame argument,
    ; the stack and the struct must match!
    mov rdi, rsp

    call pic1_irq_real_0

    popaq
    iretq

pic1_irq 1
pic1_irq 2
pic1_irq 3
pic1_irq 4
pic1_irq 5
pic1_irq 6
pic1_irq 7

pic2_irq 0
pic2_irq 1
pic2_irq 2
pic2_irq 3
pic2_irq 4
pic2_irq 5
pic2_irq 6
pic2_irq 7

section .data
global isr_stub_table
isr_stub_table:
%assign i 0 
%rep    32 
    dq isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1 
%endrep