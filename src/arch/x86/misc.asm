[BITS 64]

section .text
global _sse_setup:function (_sse_setup.end - _sse_setup)
_sse_setup:
    mov eax, 0x1
    cpuid
    test edx, 1<<25
    jz .noSSE
    ;SSE is available
    ;now enable SSE and the like
    mov rax, cr0
    and ax, 0xFFFB		;clear coprocessor emulation CR0.EM
    or ax, 0x2			;set coprocessor monitoring  CR0.MP

    ; TODO: set this up properly, and the FPU
    or ax, 1<<5			;set native exceptions  CR0.NE

    mov cr0, rax
    mov rax, cr4
    or ax, 3 << 9		;set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
    mov cr4, rax
    ret
.noSSE:
    hlt
    jmp .noSSE
.end:
