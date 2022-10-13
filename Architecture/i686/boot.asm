KERNEL_VIRTUAL_BASE EQU 0xE0000000

section .multiboot2.data
align 8
HEADER_START:
        dd 0xE85250D6
        dd 0
        dd (HEADER_END - HEADER_START)
        dd -(0xE85250D6 + (HEADER_END - HEADER_START))

align 8
        dw 0
        dw 0
        dd 8

HEADER_END:

section .bss
STACK_BOTTOM: resb 16384
STACK_TOP:

align 4096
BOOT_PAGE_DIR0: resb 4096
BOOT_PAGE_TBL0: resb 4096
BOOT_PAGE_TBL1: resb 4096

section .multiboot2.text
extern x32Entry
extern Multiboot2Initializator
extern _kernel_start
extern _kernel_end
global _start

_start:
    mov word [0xb8000], 0x074C ; L
    mov word [0xb8002], 0x076F ; o
    mov word [0xb8004], 0x0761 ; a
    mov word [0xb8006], 0x0764 ; d
    mov word [0xb8008], 0x0769 ; i
    mov word [0xb800a], 0x076E ; n
    mov word [0xb800c], 0x0767 ; g
    mov word [0xb800e], 0x072E ; .
    mov word [0xb8010], 0x072E ; .
    mov word [0xb8012], 0x072E ; .
    mov esp, STACK_TOP - KERNEL_VIRTUAL_BASE
    mov edi, BOOT_PAGE_TBL0 - KERNEL_VIRTUAL_BASE
    mov esi, 0
    mov ecx, 2048 - 301
.PagingLoop:
    cmp esi, _kernel_start - KERNEL_VIRTUAL_BASE
    jl .LoopInside
    cmp esi, _kernel_end - KERNEL_VIRTUAL_BASE
    jge .LoopEnd
    mov eax, esi
    or eax, 3
    mov [edi], eax
.LoopInside:
    add esi, 4096
    add edi, 4
    loop .PagingLoop
.LoopEnd:
    call Multiboot2Initializator
    push ebx
    mov dword [BOOT_PAGE_DIR0 - KERNEL_VIRTUAL_BASE + (000 * 4)], (BOOT_PAGE_TBL0 - KERNEL_VIRTUAL_BASE + 3)
    mov dword [BOOT_PAGE_DIR0 - KERNEL_VIRTUAL_BASE + (001 * 4)], (BOOT_PAGE_TBL1 - KERNEL_VIRTUAL_BASE + 3)
    mov dword [BOOT_PAGE_DIR0 - KERNEL_VIRTUAL_BASE + (896 * 4)], (BOOT_PAGE_TBL0 - KERNEL_VIRTUAL_BASE + 3)
    mov dword [BOOT_PAGE_DIR0 - KERNEL_VIRTUAL_BASE + (897 * 4)], (BOOT_PAGE_TBL1 - KERNEL_VIRTUAL_BASE + 3)
    mov ecx, BOOT_PAGE_DIR0 - KERNEL_VIRTUAL_BASE
    mov cr3, ecx
    mov ecx, cr0
    or ecx, 0x80010000
    mov cr0, ecx
    add esp, KERNEL_VIRTUAL_BASE
    mov eax, CallKernelMain
    jmp eax

section .text
CallKernelMain:
    push ebx
    call x32Entry
    cli
.hang:
    hlt
    jmp .hang
