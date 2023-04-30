KERNEL_PAGE_NUMBER equ 768 ; 0xC0000000

section .data
global BootPageTable
align 0x1000
BootPageTable:
    dd 0x00000083
    times (KERNEL_PAGE_NUMBER - 1) dd 0
    dd 0x00000083
    times (1024 - KERNEL_PAGE_NUMBER - 1) dd 0
