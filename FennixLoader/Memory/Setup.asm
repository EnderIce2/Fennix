[bits 32]
section .text
global EnablePaging
EnablePaging:
    mov ecx, cr0
    or ecx, 0x80000000 ; Set PG in CR0
    mov cr0, ecx
    ret
