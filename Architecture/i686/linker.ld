OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)

ENTRY(_start)

KERNEL_VIRTUAL_BASE = 0xE0000000;

SECTIONS
{
    . = 0x00100000;
    _kernel_start = . + KERNEL_VIRTUAL_BASE;

    .multiboot.data :
    {
        *(.multiboot2.data)
    }

    .multiboot.text :
    {
        *(.multiboot2.text)
    }

    . += KERNEL_VIRTUAL_BASE;
    .text ALIGN(4096) : AT(ADDR(.text) - KERNEL_VIRTUAL_BASE)
    {
        *(.text .text.*)
    }
    _kernel_text_end = .;

    .data ALIGN (4096) : AT(ADDR(.data) - KERNEL_VIRTUAL_BASE)
    {
        *(.data .data.*)
    }
    _kernel_data_end = .;

    .rodata ALIGN (4096) : AT(ADDR(.rodata) - KERNEL_VIRTUAL_BASE)
    {
        *(.rodata .rodata.*)
    }
    _kernel_rodata_end = .;

    .bss ALIGN (4096) : AT(ADDR(.bss) - KERNEL_VIRTUAL_BASE)
    {
        _sbss = .;
        *(COMMON)
        *(.bss .bss.*)
        _ebss = .;
    }
    _kernel_end = .;

    /DISCARD/ :
    {
        *(.comment*)
        *(.note*)
    }
}
