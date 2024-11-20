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

; This has to be the same as enum SMPTrampolineAddress.
TRAMPOLINE_PAGE_TABLE equ 0x500
TRAMPOLINE_START_ADDR equ 0x520
TRAMPOLINE_STACK equ 0x570
TRAMPOLINE_GDT equ 0x580
TRAMPOLINE_IDT equ 0x590
TRAMPOLINE_CORE equ 0x600
TRAMPOLINE_START equ 0x2000

[bits 16]

extern StartCPU
global _trampoline_start
_trampoline_start:
    cli
    mov ax, 0x0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    o32 lgdt [ProtectedMode_gdtr - _trampoline_start + TRAMPOLINE_START]
    mov eax, cr0
    or al, 0x1
    mov cr0, eax
    jmp 0x8:(Trampoline32 - _trampoline_start + TRAMPOLINE_START)

[bits 32]
section .text
Trampoline32:
    mov bx, 0x10
    mov ds, bx
    mov es, bx
    mov ss, bx
    mov eax, dword [TRAMPOLINE_PAGE_TABLE]
    mov cr3, eax
    mov eax, cr4
    or eax, 1 << 5 ; Set the PAE-bit, which is the 6th bit (bit 5).
    or eax, 1 << 7
    mov cr4, eax
    mov ecx, 0xc0000080
    rdmsr
    or eax,1 << 8 ; LME
    wrmsr
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax
    lgdt [LongMode_gdtr - _trampoline_start + TRAMPOLINE_START]
    jmp 0x8:(Trampoline64 - _trampoline_start + TRAMPOLINE_START)

[bits 64]
Trampoline64:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov ax, 0x0
    mov fs, ax
    mov gs, ax
    lgdt [TRAMPOLINE_GDT]
    lidt [TRAMPOLINE_IDT]
    mov rsp, [TRAMPOLINE_STACK]
    mov rbp, 0x0 ; Terminate stack traces here.
    ; Reset RFLAGS.
    push 0x0
    popf
    mov rax, qword vcode64
    call vcode64

vcode64:
    push rbp
    ; Set up SSE
    mov rax, cr0
    ; btr eax, 2
    ; bts eax, 1
    ; mov cr0, rax
    mov rax, cr4
    bts eax, 9
    bts eax, 10
    mov cr4, rax
    mov rax, qword TrampolineExit
    call rax

align 16
LongMode_gdtr:
    dw LongModeGDTEnd - LongModeGDTStart - 1
    dq LongModeGDTStart - _trampoline_start + TRAMPOLINE_START

align 16
LongModeGDTStart:
    dq 0 ; NULL segment
    dq 0x00AF98000000FFFF ; Code segment
    dq 0x00CF92000000FFFF ; Data segment
LongModeGDTEnd:

align 16
ProtectedMode_gdtr:
    dw ProtectedModeGDTEnd - ProtectedModeGDTStart - 1
    dd ProtectedModeGDTStart - _trampoline_start + TRAMPOLINE_START

align 16
ProtectedModeGDTStart:
    dq 0 ; NULL segment
    dq 0x00CF9A000000FFFF ; Code segment
    dq 0x00CF92000000FFFF ; Data segment
ProtectedModeGDTEnd:

align 16
ProtectedMode_idtr:
    dw 0
    dd 0
    dd 0
    align 16

global _trampoline_end
_trampoline_end:

TrampolineExit:
    call StartCPU

times 512 - ($-$$) db 0
