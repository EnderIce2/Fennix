ENTRY(_start)

SECTIONS
{
    . = 0x80000;
    _kernel_start = .;
    .text :
    {
        KEEP(*(.text.boot))
        *(.text .text.*)
    }
    . = ALIGN(4096);
    _kernel_text_end = .;

    .data :
    {
        *(.data .data.*)
    }
    . = ALIGN(4096);
    _kernel_data_end = .;

    .rodata :
    {
        *(.rodata .rodata.*)
    }
    . = ALIGN(4096);
    _kernel_rodata_end = .;

    .bss :
    {
        *(.bss .bss.*)
    }
    . = ALIGN(4096);
    _kernel_end = .;
    _bss_size = _kernel_end - _kernel_rodata_end;

    /DISCARD/ :
    {
        *(.eh_frame)
        *(.note .note.*)
    }
}
