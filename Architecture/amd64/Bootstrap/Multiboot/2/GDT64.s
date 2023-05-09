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

.code64
.section .bootstrap.data

/* Access bits */
.equ A, 1 << 0
.equ RW, 1 << 1
.equ DC, 1 << 2
.equ E, 1 << 3
.equ S, 1 << 4
.equ DPL0, 0 << 5
.equ DPL1, 1 << 5
.equ P, 1 << 7

/* Flags bits */
.equ LONG_MODE, 1 << 5
.equ SZ_32, 1 << 6
.equ GRAN_4K, 1 << 7

.global GDT64.Null
.global GDT64.Code
.global GDT64.Data
.global GDT64.Tss
.global GDT64.Ptr
GDT64:
.equ GDT64.Null, $ - GDT64
	.quad 0
.equ GDT64.Code, $ - GDT64
	.long 0xFFFF
	.byte 0
	.byte P | S | E | RW
	.byte GRAN_4K | LONG_MODE | 0xF
	.byte 0
.equ GDT64.Data, $ - GDT64
	.long 0xFFFF
	.byte 0
	.byte P | S | RW
	.byte GRAN_4K | SZ_32 | 0xF
	.byte 0
.equ GDT64.Tss, $ - GDT64
	.long 0x00000068
	.long 0x00CF8900
GDT64.Ptr:
	.word $ - GDT64 - 1
	.quad GDT64
