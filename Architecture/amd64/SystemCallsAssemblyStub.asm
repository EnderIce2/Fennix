[BITS 64]
ALIGN	4096
extern SystemCallsHandler
global SystemCallHandlerStub
SystemCallHandlerStub:
    swapgs
    mov [gs:0x8], rsp   ; CPUData->TempStack
    mov rsp, [gs:0x0]   ; CPUData->SystemCallStack

    push qword 0x1b     ; user data segment
    push qword [gs:0x8] ; saved stack
    push r11            ; saved rflags
    push qword 0x23     ; user code segment
    push rcx            ; Current RIP

    cld
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

    mov rdi, rsp
    mov rbp, 0
    call SystemCallsHandler

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

    mov rsp, [gs:0x8]
    swapgs
    sti
    o64 sysret
