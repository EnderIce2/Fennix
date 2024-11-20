[bits 32]
section .bootstrap.text
global DetectCPUID
DetectCPUID:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    xor eax, ecx
    jz .NoCPUID
    ret
.NoCPUID:
    ; mov word [0xb8F00], 0xF00F ; .
.Loop:
    cli
    hlt
    jmp .Loop

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
    ret
.NoLongMode:
    ; mov word [0xb8F00], 0xF00A ; .
.Loop:
    cli
    hlt
    jmp .Loop
