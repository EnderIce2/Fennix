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

.extern Multiboot1_start
.extern Multiboot2_start

.code32
.section .bootstrap.text, "a"

.global _start
_start:
	cmp eax, 0x36D76289
	je .Multiboot2
	cmp eax, 0x1BADB002
	je .Multiboot1
	int3

.Multiboot1:
	call Multiboot1_start
	jmp .

.Multiboot2:
	call Multiboot2_start
	jmp .
