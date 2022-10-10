.section ".text.boot"

.global _start
    .org 0x80000
_start:
    ldr     x5, =_start
    mov     sp, x5
    ldr     x5, =_kernel_rodata_end
    ldr     w6, =_bss_size
1:  cbz     w6, 2f
    str     xzr, [x5], #8
    sub     w6, w6, #1
    cbnz    w6, 1b
2:  bl      aarch64Entry
Halt:
    wfe
    b Halt
