[BITS 64]

extern syscall_impl

section .text
global _syscall_entrypoint:function (_syscall_entrypoint.end - _syscall_entrypoint)
_syscall_entrypoint:
    ; TODO: make it synced somehow
    mov [0x10016], rsp    ; TASK_POINTER->ret_sp_val
    mov [0x10024], r11    ; TASK_POINTER->ret_flags
    mov rsp, [0x10008]    ; TASK_POINTER->entry_ksp_val
    mov r15, rcx ; We need to save rcx
    mov rcx, rax

    sti
    ; Do very complicated stuff here
    call syscall_impl

    cli
    mov rcx, r15
    mov r11, [0x10024]    ; TASK_POINTER->ret_flags
    o64 sysret
.end:
