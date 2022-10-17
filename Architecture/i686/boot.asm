KERNEL_VIRTUAL_BASE EQU 0xE0000000

section .multiboot2
align 8
HEADER_START:
        dd 0xE85250D6
        dd 0
        dd (HEADER_END - HEADER_START)
        dd -(0xE85250D6 + 0 + (HEADER_END - HEADER_START))
align 8
MB2_TAG_START:
        dw 0
        dw 0
        dd MB2_TAG_END - MB2_TAG_START
MB2_TAG_END:
HEADER_END:

extern Multiboot2Initializator
global _start

_start:
    cli
    mov esp, STACK_TOP
	push eax
	push ebx
    call Multiboot2Initializator
.Hang:
    hlt
    jmp .Hang

section .mb2bootcode.bss
STACK_BOTTOM: resb 16384
STACK_TOP:
