%define KERNEL_OFFSET   0xFFFFFFFF80000000
%define V2P(a) ((a) - KERNEL_OFFSET)

%define PAGE_PRESENT    0x001
%define PAGE_WRITE      0x002
%define PAGE_GLOBAL     0x100

%define PAGE_SIZE       0x1000
%define ENTRIES_PER_PT  512

section .data
align PAGE_SIZE
global BootPageTable
BootPageTable:
