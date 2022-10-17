OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)

ENTRY(_start)

KERNEL_VIRTUAL_BASE = 0xC0000000;

SECTIONS
{
    . = 1M;

    .boot :
    {
        *(.multiboot2 .multiboot2.*)
    }

    .mb2bootcode.text :
    {
        *(.mb2bootcode.text)
    }

    .mb2bootcode.data :
    {
        *(.mb2bootcode.data)
    }

    .mb2bootcode.bss :
    {
        *(.mb2bootcode.bss)
    }

    . += ALIGN(4096);
    . += KERNEL_VIRTUAL_BASE;
    _kernel_start = .;

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
