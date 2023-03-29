; https://wiki.osdev.org/Higher_Half_x86_Bare_Bones
; https://wiki.osdev.org/Higher_Half_x86_Bare_Bones_(Backup)

KERNEL_VIRTUAL_BASE equ 0xC0000000 ; 3GB
KERNEL_STACK_SIZE equ 0x4000 ; 16KB

extern Multiboot2Entry
extern BootPageTable
global MB2_start

section .text
MB2_start:
	cli
    mov word [0xb8F00], 0x072E ; .

	mov ecx, (BootPageTable - KERNEL_VIRTUAL_BASE)
	mov cr3, ecx

    mov word [0xb8F02], 0x072E ; .

	mov ecx, cr4
	or ecx, 0x00000010 ; Set PSE in CR4
	mov cr4, ecx

    mov word [0xb8F04], 0x072E ; .

	mov ecx, cr0
	or ecx, 0x80000000 ; Set PG in CR0
	mov cr0, ecx

    mov word [0xb8F06], 0x072E ; .

	lea ecx, [HigherHalfStart]
	jmp ecx

HigherHalfStart:
    mov word [0xb8F08], 0x072E ; .
    mov dword [BootPageTable], 0
    invlpg [0]

	mov esp, KernelStack + KERNEL_STACK_SIZE

	push eax ; Multiboot2 Magic
	add ebx, KERNEL_VIRTUAL_BASE
	push ebx ; Multiboot2 Header
	call Multiboot2Entry
Loop:
	hlt
    jmp Loop

section .bss
align 16
KernelStack: 
	resb KERNEL_STACK_SIZE
