[bits 32]
KERNEL_VIRTUAL_BASE equ 0xC0000000 ; 3GB
KERNEL_STACK_SIZE equ 0x4000 ; 16KB

extern multiboot_main
extern BootPageTable
global _start

section .text

MB_HeaderMagic:
	dq 0

MB_HeaderInfo:
	dq 0

_start:
	cli
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
	mov [MB_HeaderMagic], eax
	mov [MB_HeaderInfo], ebx
	mov esp, KernelStack + KERNEL_STACK_SIZE
	mov eax, [MB_HeaderMagic]
	mov ebx, [MB_HeaderInfo]
	push ebx ; Multiboot2 Header
	add ebx, KERNEL_VIRTUAL_BASE
	push eax ; Multiboot2 Magic
	call multiboot_main
.Hang:
	hlt
	jmp .Hang

section .bss
align 16
KernelStack:
	resb KERNEL_STACK_SIZE
