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
extern _kernel_start
extern _kernel_end
global _start

_start:
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
