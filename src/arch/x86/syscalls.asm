[BITS 64]

extern syscall_impl

section .text
global _syscall_ret
global _syscall_entrypoint:function (_syscall_ret.end - _syscall_entrypoint)
_syscall_entrypoint:

    mov r15, 51
    cmp rdi, r15
    jne .not_fork

    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15

.not_fork:
    mov [0x10016], rsp    ; TASK_POINTER->ret_sp_val
    mov [0x10024], r11    ; TASK_POINTER->ret_flags
    mov [0x10032], rcx    ; TASK_POINTER->ret_ip
    mov rsp, [0x10008]    ; TASK_POINTER->entry_ksp_val
    mov rcx, rax ; FIXME: Not needed anymore

    sti
    ; Do very complicated stuff here
    call syscall_impl

_syscall_ret:

    cli
    mov r11, [0x10024]    ; TASK_POINTER->ret_flags
    mov rcx, [0x10032]    ; TASK_POINTER->ret_ip
    o64 sysret
.end:
