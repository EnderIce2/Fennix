[bits 32]
section .bootstrap.text

align 32
global gdtr
gdtr:
    dw GDT32_END - GDT32 - 1
    dd GDT32

align 32
GDT32:
    dq 0x0

    dw 0xffff
    dw 0x0000
    db 0x00
    dw 0xcf9a
    db 0x00

    dw 0xffff
    dw 0x0000
    db 0x00
    dw 0xcf92
    db 0x00

    dw 0x0100
    dw 0x1000
    db 0x00
    dw 0x4092
    db 0x00
GDT32_END:

global LoadGDT32
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
