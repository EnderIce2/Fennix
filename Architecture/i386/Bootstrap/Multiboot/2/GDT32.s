/*
   This file is part of Fennix Kernel.

   Fennix Kernel is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of
   the License, or (at your option) any later version.

   Fennix Kernel is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

.intel_syntax noprefix

.code32
.section .bootstrap.text

.align 32
.global gdtr
gdtr:
	.word GDT32_END - GDT32 - 1
	.long GDT32

.align 32
GDT32:
	.quad 0x0

	.word 0xFFFF
	.word 0x0000
	.byte 0x00
	.word 0xCF9A
	.byte 0x00

	.word 0xFFFF
	.word 0x0000
	.byte 0x00
	.word 0xCF92
	.byte 0x00

	.word 0x0100
	.word 0x1000
	.byte 0x00
	.word 0x4092
	.byte 0x00
GDT32_END:

.global LoadGDT32
LoadGDT32:
	lgdt [gdtr]

	jmp 0x8:ActivateGDT
	ActivateGDT:
		mov cx, 0x10
		mov ss, cx
		mov ds, cx
		mov es, cx
		mov fs, cx
		mov cx, 0x18
		mov gs, cx

	ret
