[BITS 64]

section .text
global _tlb_flush:function (_tlb_flush.end - _tlb_flush)
_tlb_flush:
    mov rax, cr3
    mov cr3, rax
    ret
.end: