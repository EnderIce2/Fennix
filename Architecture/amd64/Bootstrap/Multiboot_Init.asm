[bits 32]
KERNEL_STACK_SIZE equ 0x4000 ; 16KB

extern multiboot_main
extern LoadGDT32
extern BootPageTable
extern UpdatePageTable

section .bootstrap.data

MB_HeaderMagic:
	dq 0

MB_HeaderInfo:
	dq 0

section .bootstrap.text

global _start
_start:
	cli

	mov [MB_HeaderMagic], eax
	mov [MB_HeaderInfo], ebx

	mov ecx, cr4
	or ecx, 0x00000010 ; Set PSE in CR4
	or ecx, 0x00000020 ; Set PAE in CR4
	mov cr4, ecx

	call LoadGDT32
	call UpdatePageTable

	mov ecx, BootPageTable
	mov cr3, ecx

	mov ecx, 0xC0000080 ; EFER
	rdmsr
	or eax, 0x800 | 0x100 | 0x1 ; Set LME, LMA, SCE
	wrmsr

	mov ecx, cr0
	or ecx, 0x80000000 | 0x1 ; Set PG and PE in CR0
	mov cr0, ecx

	lgdt [GDT64.Ptr]

	jmp GDT64.code:HigherHalfStart

extern UpdatePageTable64

[bits 64]
HigherHalfStart:
	mov ax, GDT64.data
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	call UpdatePageTable64

	mov rsp, KernelStack + KERNEL_STACK_SIZE
	mov rdi, [MB_HeaderMagic]
	mov rsi, [MB_HeaderInfo]
	push rsi
	push rdi
	call multiboot_main
.Hang:
	hlt
	jmp .Hang



; Access bits
PRESENT equ 1 << 7
NOT_SYS equ 1 << 4
EXEC equ 1 << 3
DC equ 1 << 2
RW equ 1 << 1
ACCESSED equ 1 << 0
 
; Flags bits
GRAN_4K equ 1 << 7
SZ_32 equ 1 << 6
LONG_MODE equ 1 << 5

section .bootstrap.data
GDT64:
    .null: equ $ - GDT64
        dq 0
    .code: equ $ - GDT64
        dd 0xFFFF
        db 0
        db PRESENT | NOT_SYS | EXEC | RW
        db GRAN_4K | LONG_MODE | 0xF
        db 0
    .data: equ $ - GDT64
        dd 0xFFFF
        db 0
        db PRESENT | NOT_SYS | RW
        db GRAN_4K | SZ_32 | 0xF
        db 0
    .tss: equ $ - GDT64
        dd 0x00000068
        dd 0x00CF8900
	.Ptr:
		dw $ - GDT64 - 1
		dq GDT64

section .bootstrap.bss
align 16
KernelStack:
	resb KERNEL_STACK_SIZE
