[BITS 64]

section .text
global _syscall_entrypoint:function (_syscall_entrypoint.end - _syscall_entrypoint)
_syscall_entrypoint:
  sysret
.end:
