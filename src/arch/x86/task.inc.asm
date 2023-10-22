[BITS 64]

; =!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!
; FIXME: CMake doesn't detect changes to this file!
; OBJECT_DEPENDS also doesn't seem to work...
; =!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!

extern temp_fxsave

; TODO: This is probably not enough
%macro pushaq 0
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    fxsave64 [temp_fxsave]

    mov rdi, 0xdeadbe3fdeadb3ef ; IDT_GUARD
    push rdi ; IDT_GUARD

%endmacro
%macro popaq 0
    add rsp, 8 ; remove IDT_GUARD

    fxrstor64 [temp_fxsave]

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
%endmacro