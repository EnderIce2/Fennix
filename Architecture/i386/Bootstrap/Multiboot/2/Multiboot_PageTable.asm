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

KERNEL_VIRTUAL_BASE equ 0xC0000000 ; 3GB or 0xC0000000
KERNEL_PAGE_NUMBER equ (KERNEL_VIRTUAL_BASE >> 22) ; 768

section .bootstrap.data
align 0x1000
global BootPageTable
BootPageTable:
    dd 0x00000083
    dd 0x00000083
    times (KERNEL_PAGE_NUMBER - 2) dd 0
    dd 0x00000083
    dd 0x00000083
    times (1024 - KERNEL_PAGE_NUMBER - 2) dd 0
