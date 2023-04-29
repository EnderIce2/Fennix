ENTRY(_start)

SECTIONS
{
    . = 0x100000;
    _kernel_start = .;

    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)
        *(.multiboot2)
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    .init_array :
    {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(.init_array .ctors))
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        PROVIDE_HIDDEN (__init_array_end = .);
    }

    .fini_array :
    {
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
        KEEP(*(.fini_array .dtors))
        PROVIDE_HIDDEN (__fini_array_end = .);
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }

    /DISCARD/ :
    {
        *(.comment*)
        *(.note*)
    }
    . += CONSTANT(MAXPAGESIZE);
    _kernel_end = ALIGN(CONSTANT(MAXPAGESIZE));
}
