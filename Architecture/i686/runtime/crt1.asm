; https://wiki.osdev.org/Higher_Half_x86_Bare_Bones
; https://wiki.osdev.org/Higher_Half_x86_Bare_Bones_(Backup)
section .multiboot2
align 4096
HEADER_START:
    dd 0xE85250D6
    dd 0
    dd (HEADER_END - HEADER_START)
    dd 0x100000000 - (HEADER_END - HEADER_START) - 0 - 0xE85250D6
align 8
MB2_INFO_REQUEST_TAG_START:
    dw 1
    dw 0
    dd MB2_INFO_REQUEST_TAG_END - MB2_INFO_REQUEST_TAG_START
    dd 1 ; Command Line
    dd 2 ; Boot Loader Name
    dd 3 ; Module
    dd 4 ; Basic Memory Information
    dd 5 ; BIOS Boot Device
    dd 6 ; Memory Map
    dd 7 ; VBE
    dd 8 ; Framebuffer
    dd 9 ; ELF Sections
    dd 10 ; APM Table
    dd 11 ; EFI 32-bit System Table Pointer
    dd 12 ; EFI 64-bit System Table Pointer
    ; dd 13 ; SMBIOS
    dd 14 ; ACPI Old
    dd 15 ; ACPI New
    dd 16 ; Network
    dd 17 ; EFI Memory Map
    dd 18 ; EFI Boot Services Notifier
    dd 19 ; EFI 32-bit Image Handle Pointer
    dd 20 ; EFI 64-bit Image Handle Pointer
    dd 21 ; Load Base Address
MB2_INFO_REQUEST_TAG_END:
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

extern x32Multiboot2Entry
global _start

section .data
align 0x1000
BootPageTable:
    dd 0x00000083
    dd 0x00000083
    times (KERNEL_PAGE_NUMBER - 2) dd 0
    dd 0x00000083
    dd 0x00000083
    times (1024 - KERNEL_PAGE_NUMBER - 2) dd 0

section .text
_start:
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
	call x32Multiboot2Entry
Loop:
	hlt
    jmp Loop

section .bss
align 16
KernelStack : 
	resb KERNEL_STACK_SIZE
