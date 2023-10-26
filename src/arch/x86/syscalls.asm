[BITS 64]

extern syscall_impl

section .text
global _syscall_entrypoint:function (_syscall_entrypoint.end - _syscall_entrypoint)
_syscall_entrypoint:
    ; TODO: make it synced somehow
    mov r11, 0x10016 ; TASK_POINTER->ret_sp_val
    mov [r11], rsp
    mov r11, 0x10008 ; TASK_POINTER->entry_ksp_val
    mov rsp, [r11]
    mov r15, rcx

    ; Do very complicated stuff here
    call syscall_impl

    mov r11, 0x10016 ; TASK_POINTER->entry_ksp_val
    mov rsp, [r11]
    mov rcx, r15
    o64 sysret
.end:
