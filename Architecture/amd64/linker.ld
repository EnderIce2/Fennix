OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

KERNEL_LMA = 16M;
KERNEL_VMA = 0xFFFFFFFF80000000;

ENTRY(MB2_start)

SECTIONS
{
    . = KERNEL_LMA;

    .multiboot2 :
    {
        *(.multiboot2 .multiboot2.*)
    }

    . += KERNEL_VMA;

    _kernel_start = .;
    .text : AT(ADDR(.text) - KERNEL_VMA)
    {
        *(.text .text.*)
    }
    _kernel_text_end = ALIGN(CONSTANT(MAXPAGESIZE));
    . += CONSTANT(MAXPAGESIZE);

    .data : AT(ADDR(.data) - KERNEL_VMA)
    {
        *(.data .data.*)
    }
    _kernel_data_end = ALIGN(CONSTANT(MAXPAGESIZE));
    . += CONSTANT(MAXPAGESIZE);

    .rodata : AT(ADDR(.rodata) - KERNEL_VMA)
    {
        *(.rodata .rodata.*)
    }
    _kernel_rodata_end = ALIGN(CONSTANT(MAXPAGESIZE));
    . += CONSTANT(MAXPAGESIZE);

    .init_array : AT(ADDR(.init_array) - KERNEL_VMA)
    {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(.init_array .ctors))
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        PROVIDE_HIDDEN (__init_array_end = .);
    }

    .fini_array : AT(ADDR(.fini_array) - KERNEL_VMA)
    {
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
        KEEP(*(.fini_array .dtors))
        PROVIDE_HIDDEN (__fini_array_end = .);
    }
    . += CONSTANT(MAXPAGESIZE);

    .bss : AT(ADDR(.bss) - KERNEL_VMA)
    {
        *(COMMON)
        *(.bss .bss.*)
    }
    . += CONSTANT(MAXPAGESIZE);
    _kernel_end = ALIGN(CONSTANT(MAXPAGESIZE));

    /DISCARD/ :
    {
        *(.comment*)
        *(.note*)
    }
}
