/* Based on this tutorial:
   https://github.com/s-matyukevich/raspberry-pi-os */

.section ".text.boot"

.extern _bss_start
.extern _bss_end

.extern arm64Entry
memzero:
    str xzr, [x0], #8
    subs x1, x1, #8
    b.gt memzero
    ret

.global _start
_start:
    mrs x0, mpidr_el1
    and x0, x0, #0xFF
    cbz x0, _start2
    b CPU_Loop

_start2:
    adr x0, _bss_start
    adr x1, _bss_end
    sub x1, x1, x0
    bl memzero
    mov sp, #0x200000
    bl arm64Entry

Halt:
    wfe
    b Halt

CPU_Loop:
    b CPU_Loop
