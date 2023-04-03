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

[BITS 64]

%macro PushAllSC 0
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro PopAllSC 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
%endmacro

ALIGN 4096
extern SystemCallsHandler
global SystemCallHandlerStub
SystemCallHandlerStub:
    swapgs                  ; Swap gs and kernelgs
    mov [gs:0x8], rsp       ; CPUData->TempStack
    mov rsp, [gs:0x0]       ; CPUData->SystemCallStack
    push qword 0x1b         ; User data segment
    push qword [gs:0x8]     ; Saved stack
    push r11                ; Saved rflags
    push qword 0x23         ; User code segment
    push rcx                ; Current instruction pointer
    cld                     ; Clear direction flag
    PushAllSC               ; Push all registers
    mov rdi, rsp            ; Pass pointer to registers
    mov rbp, 0              ; Pass 0 as return address
    call SystemCallsHandler ; Call system call handler
    PopAllSC                ; Pop all registers except rax
    mov rsp, [gs:0x8]       ; Restore stack
    swapgs                  ; Swap back gs and kernelgs
    sti                     ; Enable interrupts
    o64 sysret              ; Return to user mode
