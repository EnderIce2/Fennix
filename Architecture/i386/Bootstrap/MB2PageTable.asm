KERNEL_VIRTUAL_BASE equ 0xC0000000 ; 3GB
KERNEL_PAGE_NUMBER equ (KERNEL_VIRTUAL_BASE >> 22) ; 768

section .data
global BootPageTable
align 0x1000
BootPageTable:
    dd 0x00000083
    dd 0x00000083
    times (KERNEL_PAGE_NUMBER - 2) dd 0
    dd 0x00000083
    dd 0x00000083
    times (1024 - KERNEL_PAGE_NUMBER - 2) dd 0
