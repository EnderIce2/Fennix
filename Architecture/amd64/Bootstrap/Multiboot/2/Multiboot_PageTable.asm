;   This file is part of Fennix Kernel.
;
;   Fennix Kernel is free software: you can redistribute it and/or
;   modify it under the terms of the GNU General Public License as
;   published by the Free Software Foundation, either version 3 of
;   the License, or (at your option) any later version.
;
;   Fennix Kernel is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;   GNU General Public License for more details.
;
;   You should have received a copy of the GNU General Public License
;   along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.

[bits 32]

PAGE_TABLE_SIZE equ 0x4

section .bootstrap.data
align 0x1000
global BootPageTable
BootPageTable:
    times (0x10000) dq 0 ; 0x4000 bytes will be used in UpdatePageTable

section .bootstrap.text
global UpdatePageTable
UpdatePageTable:
	mov edi, (BootPageTable + 0x0000) ; First PML4E
	mov eax, (BootPageTable + 0x1000) ; First PDPTE
	or eax, 11b ; Bitwise OR on rax (PDPTE) with 11b (Present, Write)
	mov dword [edi], eax ; Write 11b to PML4E

	mov edi, (BootPageTable + 0x1000) ; First PDPTE
	mov eax, (BootPageTable + 0x2000) ; First PDE
	or eax, 11b ; Bitwise OR on rax (PDE) with 11b (Present, Write)

	mov ecx, PAGE_TABLE_SIZE ; For loop instruction
	mov ebx, 0x0 ; Value to store in the next 4 bytes
	.FillPageTableLevel3:
		mov dword [edi], eax ; Store modified PDE in PDPTE
		mov dword [edi + 4], ebx ; Store the rbx value in the next 4 bytes
		add eax, 0x1000 ; Increment (page size)
		adc ebx, 0 ; Add 0 to carry flag
		add edi, 8 ; Add 8 to rdi (next PDE)
		loop .FillPageTableLevel3 ; Loop until rcx is 0

	mov edi, (BootPageTable + 0x2000) ; First PDE
	mov eax, 10000011b ; Present, Write, Large Page

	mov ecx, (512 * PAGE_TABLE_SIZE) ; For loop instruction
	mov ebx, 0x0 ; Value to store in the next 4 bytes
	.FillPageTableLevel2:
		mov dword [edi], eax ; Store modified PDE in PDPTE
		mov dword [edi + 4], ebx ; Store the rbx value in the next 4 bytes
		add eax, 1 << 21 ; Increment (page size)
		adc ebx, 0 ; Add 0 (carry flag) to rbx to increment if there was a carry
		add edi, 8 ; Add 8 to rdi (next PDE)
		loop .FillPageTableLevel2 ; Loop until rcx is 0

	ret
