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
	mov edi, (BootPageTable + 0x0000)

	mov eax, (BootPageTable + 0x1000)
	or eax, 0x3
	mov dword [edi], eax

	mov ecx, PAGE_TABLE_SIZE
	mov edi, (BootPageTable + 0x1000)
	mov eax, (BootPageTable + 0x2000)
	or eax, 0x3
	mov ebx, 0x0

	.FillPageTableLevel3:
		mov dword [edi], eax
		mov dword [edi + 4], ebx
		add eax, 1 << 12
		adc ebx, 0
		add edi, 8
		loop .FillPageTableLevel3

	mov ecx, (512 * PAGE_TABLE_SIZE)
	mov edi, (BootPageTable + 0x2000)
	mov eax, 0x0 | 0x3 | 1 << 7
	mov ebx, 0x0

	.FillPageTableLevel2:
		mov dword [edi], eax
		mov dword [edi + 4], ebx
		add eax, 1 << 21
		adc ebx, 0
		add edi, 8
		loop .FillPageTableLevel2

	ret

[bits 64]
section .bootstrap.text
global UpdatePageTable64
UpdatePageTable64:
	mov rdi, (BootPageTable + 0x0000)

	mov rax, (BootPageTable + 0x1000)
	or rax, 0x3
	mov [rdi], rax

	mov rcx, PAGE_TABLE_SIZE
	mov rdi, (BootPageTable + 0x1000)
	mov rax, (BootPageTable + 0x2000)
	or rax, 0x3
	mov rbx, 0x0

	.FillPageTableLevel3:
		mov [rdi], rax
		mov [rdi + 4], rbx
		add rax, 1 << 12
		adc rbx, 0
		add rdi, 8
		loop .FillPageTableLevel3

	mov rcx, (512 * PAGE_TABLE_SIZE)
	mov rdi, (BootPageTable + 0x2000)
	mov rax, 0x0 | 0x3 | 1 << 7
	mov rbx, 0x0

	.FillPageTableLevel2:
		mov [rdi], rax
		mov [rdi + 4], rbx
		add rax, 1 << 21
		adc rbx, 0
		add rdi, 8
		loop .FillPageTableLevel2

	ret
