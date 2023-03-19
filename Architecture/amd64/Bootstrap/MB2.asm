; https://wiki.osdev.org/Creating_a_64-bit_kernel
; https://wiki.osdev.org/Entering_Long_Mode_Directly

KERNEL_VIRTUAL_BASE equ 0xFFFFFFFF80000000 ; 512GB
KERNEL_LMA equ 0x1000000 ; 16MB
KERNEL_STACK_SIZE equ 0x4000 ; 16KB

extern Multiboot2Entry
extern BootPageTable
extern DetectCPUID
extern Detect64Bit
global MB2_start

[bits 32]
section .text
MB2_start:
	cli

    mov word [0xb8F00], 0x072E ; .

	call DetectCPUID
	call Detect64Bit

	mov ecx, cr4
	or ecx, 0x00000010 ; Set PSE in CR4
	mov cr4, ecx

    mov word [0xb8F02], 0x072E ; .

	mov ecx, (BootPageTable - KERNEL_VIRTUAL_BASE)
	mov cr3, ecx

	mov ecx, cr0
	or ecx, 0x80000000 ; Set PG in CR0
	mov cr0, ecx

	cli
	hlt

    mov word [0xb8F04], 0x072E ; .


    mov word [0xb8F06], 0x072E ; .

	cli
	hlt

	; lea ecx, [HigherHalfStart]
	; jmp ecx

[bits 64]
HigherHalfStart:
    mov word [0xb8F08], 0x072E ; .
    mov dword [BootPageTable], 0
    invlpg [0]

	mov rsp, KernelStack + KERNEL_STACK_SIZE

	push rax ; Multiboot2 Magic
	add rbx, KERNEL_VIRTUAL_BASE
	push rbx ; Multiboot2 Header
	call Multiboot2Entry
Loop:
	hlt
    jmp Loop

section .bss
align 16
KernelStack: 
	resb KERNEL_STACK_SIZE
