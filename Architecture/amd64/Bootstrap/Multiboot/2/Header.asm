[bits 32]
extern Multiboot2_start

; https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html
section .multiboot2
align 4096
MULTIBOOT2_HEADER_START:
    dd 0xE85250D6
    dd 0
    dd (MULTIBOOT2_HEADER_END - MULTIBOOT2_HEADER_START)
    dd 0x100000000 - (MULTIBOOT2_HEADER_END - MULTIBOOT2_HEADER_START) - 0 - 0xE85250D6
align 8
InfoRequestTag_Start:
    dw 1
    dw 0
    dd InfoRequestTag_End - InfoRequestTag_Start
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
InfoRequestTag_End:
align 8
FramebufferTag_Start:
    dw 5
    dw 1
    dd FramebufferTag_End - FramebufferTag_Start
    dd 0
    dd 0
    dd 32
FramebufferTag_End:
align 8
EGATextSupportTag_Start:
    dw 4
    dw 0
    dd EGATextSupportTag_End - EGATextSupportTag_Start
    dd 1 ; https://www.gnu.org/software/grub/manual/multiboot2/html_node/Console-header-tags.html
EGATextSupportTag_End:
align 8
AlignedModulesTag_Start:
    dw 6
    dw 0
    dd AlignedModulesTag_End - AlignedModulesTag_Start
AlignedModulesTag_End:
align 8
EntryAddressTag_Start:
    dw 9
    dw 0
    dd EntryAddressTag_End - EntryAddressTag_Start
    dd Multiboot2_start
EntryAddressTag_End:
align 8
EndTag_Start:
    dw 0
    dw 0
    dd EndTag_End - EndTag_Start
EndTag_End:
MULTIBOOT2_HEADER_END:
