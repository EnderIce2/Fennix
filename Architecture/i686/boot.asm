; Inspired From: https://github.com/MQuy/mos/blob/master/src/kernel/boot.asm
section .multiboot2
align 4096
HEADER_START:
    dd 0xE85250D6
    dd 0
    dd (HEADER_END - HEADER_START)
    dd 0x100000000 - (HEADER_END - HEADER_START) - 0 - 0xE85250D6
align 8
MB2_TAG_START:
    dw 0
    dw 0
    dd MB2_TAG_END - MB2_TAG_START
MB2_TAG_END:
HEADER_END:

KERNEL_VIRTUAL_BASE equ 0xC0000000 ; 3GB
KERNEL_PAGE_NUMBER equ (KERNEL_VIRTUAL_BASE >> 22) ; 768
KERNEL_STACK_SIZE equ 0x4000 ; 16KB

extern x32Entry
global _start

section .data
align 0x1000
BootPageTable:
    dd 0x00000083
    times (KERNEL_PAGE_NUMBER - 1) dd 0
    dd 0x00000083
    times (1024 - KERNEL_PAGE_NUMBER - 1) dd 0

section .text
_start:
	mov ecx, (BootPageTable - KERNEL_VIRTUAL_BASE)
	mov cr3, ecx
	mov ecx, cr4
	or ecx, 0x00000010 ; Set PSE in CR4
	mov cr4, ecx
	mov ecx, cr0
	or ecx, 0x80000000 ; Set PG in CR0
	mov cr0, ecx
	lea ecx, [HigherHalfStart]
	jmp ecx

HigherHalfStart:
	mov esp, KernelStack + KERNEL_STACK_SIZE

	push eax ; Multiboot2 Magic
	add ebx, KERNEL_VIRTUAL_BASE
	push ebx ; Multiboot2 Header
	call x32Entry
Loop:
	hlt
    jmp Loop


section .bss
align 16
KernelStack : 
	resb KERNEL_STACK_SIZE
