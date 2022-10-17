OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)

ENTRY(_start)

SECTIONS
{
    . = 0xC0100000;
    _kernel_start = .;

    .text ALIGN(4096) : AT(ADDR(.text) - 0xC0000000)
    {
        *(.multiboot2)
        *(.text .text.*)
    }
    _kernel_text_end = .;

    .data ALIGN (4096) : AT(ADDR(.data) - 0xC0000000)
    {
        *(.data .data.*)
    }
    _kernel_data_end = .;

    .rodata ALIGN (4096) : AT(ADDR(.rodata) - 0xC0000000)
    {
        *(.rodata .rodata.*)
    }
    _kernel_rodata_end = .;

    .bss ALIGN (4096) : AT(ADDR(.bss) - 0xC0000000)
    {
        *(COMMON)
        *(.bss .bss.*)
    }
    _kernel_end = .;

    /DISCARD/ :
    {
        *(.comment*)
        *(.note*)
    }
}
