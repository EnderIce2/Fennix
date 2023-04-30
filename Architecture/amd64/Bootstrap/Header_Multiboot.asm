section .multiboot
align 4
    dd 0x1BADB002
    dd 1 << 0 | 1 << 1
    dd -(0x1BADB002 + (1 << 0 | 1 << 1))
