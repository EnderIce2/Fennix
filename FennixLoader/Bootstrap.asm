[bits 32]

extern loader_main

section .text

MB_HeaderMagic:
    dq 0

MB_HeaderInfo:
	dq 0

global _start
_start:
	cli
    mov [MB_HeaderMagic], eax
    mov [MB_HeaderInfo], ebx
	mov esp, KernelStack
	mov eax, [MB_HeaderMagic]
	mov ebx, [MB_HeaderInfo]
	push ebx
	push eax
	call loader_main
.hang:
	cli
	hlt
	jmp .hang

STACK_SIZE equ 0x4000 ; 16KB

section .bss
align 16
KernelStack: 
	resb STACK_SIZE
