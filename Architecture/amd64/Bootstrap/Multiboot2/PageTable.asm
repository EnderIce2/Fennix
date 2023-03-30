PAGE_TABLE_SIZE equ 0x4 ; 1GB
[bits 32]

section .bootstrap.data
align 0x1000
global BootPageTable
BootPageTable:
    times (0x10000) dq 0 ; 0x4000

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
	mov eax, 11b | 10000000b ; Present, Write, Large Page

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
