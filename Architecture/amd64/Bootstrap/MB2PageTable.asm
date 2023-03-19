section .data
align 0x1000
global BootPageTable
BootPageTable:
    dq 0x0000000000000083
    dq 0x0000000000000083
    TIMES (512-2) dq 0
    dq 0x0000000000000083
    dq 0x0000000000000083
    TIMES (512-2) dq 0