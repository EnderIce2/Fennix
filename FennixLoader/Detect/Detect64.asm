[bits 32]
global Detect64Bit
Detect64Bit:
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .NoLongMode
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .NoLongMode
    mov eax, 1
    ret
.NoLongMode:
    xor eax, eax
    ret
