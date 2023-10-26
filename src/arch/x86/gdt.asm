[BITS 64]

; Access bits
PRESENT        equ 1 << 7
NOT_SYS        equ 1 << 4
EXEC           equ 1 << 3
DC             equ 1 << 2
RW             equ 1 << 1
ACCESSED       equ 1 << 0
USER           equ 1 << 6 | 1 << 5

; Flags bits
GRAN_4K       equ 1 << 7
SZ_32         equ 1 << 6
LONG_MODE     equ 1 << 5
 
section .gdt
global gdt_null:data
gdt_null:
    dq 0
global gdt_code_16:data
gdt_code_16:
    dd 0xFFFF                                   ; Limit & Base (low, bits 0-15)
    db 0                                        ; Base (mid, bits 16-23)
    db PRESENT | NOT_SYS | EXEC | RW            ; Access
    db GRAN_4K | 0xF                            ; Flags & Limit (high, bits 16-19)
    db 0                                        ; Base (high, bits 24-31)
global gdt_data_16:data
gdt_data_16:
    dd 0xFFFF                                   ; Limit & Base (low, bits 0-15)
    db 0                                        ; Base (mid, bits 16-23)
    db PRESENT | NOT_SYS | RW                   ; Access
    db GRAN_4K | 0xF                            ; Flags & Limit (high, bits 16-19)
    db 0                                        ; Base (high, bits 24-31)
global gdt_code_32:data
gdt_code_32:
    dd 0xFFFF                                   ; Limit & Base (low, bits 0-15)
    db 0                                        ; Base (mid, bits 16-23)
    db PRESENT | NOT_SYS | EXEC | RW            ; Access
    db GRAN_4K | SZ_32 | 0xF                    ; Flags & Limit (high, bits 16-19)
    db 0                                        ; Base (high, bits 24-31)
global gdt_data_32:data
gdt_data_32:
    dd 0xFFFF                                   ; Limit & Base (low, bits 0-15)
    db 0                                        ; Base (mid, bits 16-23)
    db PRESENT | NOT_SYS | RW                   ; Access
    db GRAN_4K | SZ_32 | 0xF                    ; Flags & Limit (high, bits 16-19)
    db 0                                        ; Base (high, bits 24-31)
global gdt_code:data
gdt_code:
    dd 0xFFFF                                   ; Limit & Base (low, bits 0-15)
    db 0                                        ; Base (mid, bits 16-23)
    db PRESENT | NOT_SYS | EXEC | RW            ; Access
    db GRAN_4K | LONG_MODE | 0xF                ; Flags & Limit (high, bits 16-19)
    db 0                                        ; Base (high, bits 24-31)
global gdt_data:data
gdt_data:
    dd 0xFFFF                                   ; Limit & Base (low, bits 0-15)
    db 0                                        ; Base (mid, bits 16-23)
    db PRESENT | NOT_SYS | RW                   ; Access
    db GRAN_4K | SZ_32 | 0xF                    ; Flags & Limit (high, bits 16-19)
    db 0                                        ; Base (high, bits 24-31)
global gdt_data_user:data
gdt_data_user:
    dd 0xFFFF                                   ; Limit & Base (low, bits 0-15)
    db 0                                        ; Base (mid, bits 16-23)
    db PRESENT | USER | NOT_SYS | RW            ; Access
    db GRAN_4K | SZ_32 | 0xF                    ; Flags & Limit (high, bits 16-19)
    db 0                                        ; Base (high, bits 24-31)
global gdt_code_user:data
gdt_code_user:
    dd 0xFFFF                                   ; Limit & Base (low, bits 0-15)
    db 0                                        ; Base (mid, bits 16-23)
    db PRESENT | USER | NOT_SYS | EXEC | RW     ; Access
    db GRAN_4K | LONG_MODE | 0xF                ; Flags & Limit (high, bits 16-19)
    db 0                                        ; Base (high, bits 24-31)
global gdt_tss:data
gdt_tss:
    dq 0x00000000 ;TODO
    dq 0x00000000
global gdt_tss_user:data
gdt_tss_user:
    dq 0x00000000 ;TODO
    dq 0x00000000
global gdt_end:data
gdt_end:
global gdtr:data
gdtr:
    dw gdt_end - gdt_null - 1
    dq gdt_null

section .text
global _gdt_setup:function (_gdt_setup.end - _gdt_setup)
_gdt_setup:
    LGDT [gdtr]
    ; Reload CS register:
    PUSH (gdt_code - gdt_null); Push code segment to stack, 0x08 is a stand-in for your code segment
    LEA RAX, [rel .flush]     ; Load address of .reload_CS into RAX
    PUSH RAX                  ; Push this value to the stack
    RETFQ                     ; Perform a far return, RETFQ or LRETQ depending on syntax
.flush:
    ; Reload data segment registers
    MOV   AX, (gdt_data - gdt_null) ; 0x10 is a stand-in for your data segment
    MOV   DS, AX
    MOV   ES, AX
    MOV   FS, AX
    MOV   GS, AX
    MOV   SS, AX
    MOV   AX, (gdt_tss - gdt_null)
    ltr AX
    RET
.end:
