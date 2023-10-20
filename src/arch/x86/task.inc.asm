[BITS 64]

; =!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!
; FIXME: CMake doesn't detect changes to this file!
; OBJECT_DEPENDS also doesn't seem to work...
; =!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!

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

    ; Ensure 16-byte alignment
    ; This works as last bunch of bits in fxsave state aren't used
    ; Fxsaved memory needs to be zeroed before!
    mov rsi, 0x0000000000000000
    times 64 push rsi

    mov rsi, rsp
    add rsi, 32
    mov rdi, 0xFFFFFFFFFFFFFFF0
    and rsi, rdi

    fxsave64 [rsi]

%endmacro
%macro popaq 0

    ; Ensure 16-byte alignment
    ; This works as last bunch of bits in fxsave state aren't used
    mov rsi, rsp
    add rsi, 32
    mov rdi, 0xFFFFFFFFFFFFFFF0
    and rsi, rdi

    fxrstor64 [rsi]
    add rsp, 512

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