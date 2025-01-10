/* Based on this tutorial:
   https://github.com/s-matyukevich/raspberry-pi-os */

.section ".text.boot", "a"

.extern _bss_start
.extern _bss_end

.global _start
_start:
	b CPU_Loop

Halt:
	wfe
	b Halt

CPU_Loop:
	b CPU_Loop
