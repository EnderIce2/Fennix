[bits 32]
global DetectCPUID
DetectCPUID:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    xor eax, ecx
    jz .NoCPUID
    mov eax, 1
    ret
.NoCPUID:
    xor eax, eax
    ret
