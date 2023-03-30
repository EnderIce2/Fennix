; https://wiki.osdev.org/Creating_a_64-bit_kernel
; https://wiki.osdev.org/Entering_Long_Mode_Directly

KERNEL_VIRTUAL_BASE equ 0xFFFFFFFF80000000 ; 512GB
KERNEL_LMA equ 0x1000000 ; 16MB
KERNEL_STACK_SIZE equ 0x4000 ; 16KB

extern Multiboot2Entry
extern BootPageTable
extern UpdatePageTable
extern UpdatePageTable64
extern DetectCPUID
extern Detect64Bit
extern LoadGDT32
global MB2_start
extern MB2_start_c

[bits 32]

section .bootstrap.data
global MB2_HeaderMagic
MB2_HeaderMagic:
    times (0x64) dq 0

global MB2_HeaderInfo
MB2_HeaderInfo:
	times (0x64) dq 0

section .bootstrap.text
MB2_start:
	cli
	mov word [0xb8F00], 0x072E ; .
    mov [MB2_HeaderMagic], eax
    mov [MB2_HeaderInfo], ebx

	; We need to check if the CPU supports 64-bit mode
	call DetectCPUID
	call Detect64Bit

	mov word [0xb8F02], 0x072E ; .

	mov ecx, cr0
	and ecx, 0x7fffffff ; Clear PG
	mov cr0, ecx

	mov ecx, cr4
	or ecx, 0x10 ; Set PSE
	or ecx, 0x20 ; Set PAE
	mov cr4, ecx

	; Load the GDT and update the page table
	call LoadGDT32
	call UpdatePageTable

	; Load the new page table
	mov edi, BootPageTable
	mov cr3, edi

	mov word [0xb8F04], 0x072E ; .

	; Enable long mode
	mov ecx, 0xC0000080 ; EFER
	rdmsr
	or eax, 0x800 | 0x100 | 0x1 ; Set LME, LMA, SCE
	wrmsr

	mov ecx, cr0
	or ecx, (0x80000000 | 0x1) ; Set PG and PE
	mov cr0, ecx


	lgdt [GDT64.Ptr]
    ; xor eax, eax
    ; sgdt [eax]
    ; test eax, eax
    ; jz .InvalidGDT

	; .InvalidGDT:
	; 	mov word [0xb8F07], 0x4 ; Red
	; 	hlt

	jmp GDT64.code:HigherHalfStart

[bits 64]
HigherHalfStart:
	cli
	mov word [0xb8F06], 0x072E ; .
	call UpdatePageTable64

	; Load the new page table
	mov rdi, BootPageTable
	mov cr3, rdi

	mov ax, GDT64.data
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	mov word [0xb8F08], 0x072E ; .
	mov rsp, (KernelStack + KERNEL_STACK_SIZE)
	mov rbp, (KernelStack + KERNEL_STACK_SIZE)

	cld
	cli

	call Multiboot2Entry
	.Loop:
		hlt
		jmp .Loop

section .bootstrap.bss
align 16
KernelStack: 
	resb KERNEL_STACK_SIZE

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
