[bits 32]
section .multiboot2
align 4096
HEADER_START:
    dd 0xE85250D6
    dd 0
    dd (HEADER_END - HEADER_START)
    dd 0x100000000 - (HEADER_END - HEADER_START) - 0 - 0xE85250D6
align 8
MB2_INFO_REQUEST_TAG_START:
    dw 1
    dw 0
    dd MB2_INFO_REQUEST_TAG_END - MB2_INFO_REQUEST_TAG_START
    dd 1 ; Command Line
    dd 2 ; Boot Loader Name
    dd 3 ; Module
    dd 4 ; Basic Memory Information
    dd 5 ; BIOS Boot Device
    dd 6 ; Memory Map
    dd 7 ; VBE
    dd 8 ; Framebuffer
    dd 9 ; ELF Sections
    dd 10 ; APM Table
    dd 11 ; EFI 32-bit System Table Pointer
    dd 12 ; EFI 64-bit System Table Pointer
    ; dd 13 ; SMBIOS
    dd 14 ; ACPI Old
    dd 15 ; ACPI New
    dd 16 ; Network
    dd 17 ; EFI Memory Map
    dd 18 ; EFI Boot Services Notifier
    dd 19 ; EFI 32-bit Image Handle Pointer
    dd 20 ; EFI 64-bit Image Handle Pointer
    dd 21 ; Load Base Address
MB2_INFO_REQUEST_TAG_END:
align 8
MB2_TAG_START:
    dw 0
    dw 0
    dd MB2_TAG_END - MB2_TAG_START
MB2_TAG_END:
HEADER_END:
