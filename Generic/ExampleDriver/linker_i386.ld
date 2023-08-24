OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)

ENTRY(DriverEntry)

SECTIONS
{
    .header :
    {
        *(.header .header.*)
        *(.extended .extended.*)
    }

    .text :
    {
        *(.text .text.*)
    }

    .data :
    {
        *(.data .data.*)
        PROVIDE(_GLOBAL_OFFSET_TABLE_ = .);
    }

    .rodata :
    {
        *(.rodata .rodata.*)
    }

    .bss :
    {
        *(COMMON)
        *(.bss .bss.*)
    }

    /DISCARD/ :
    {
        *(.eh_frame)
        *(.note .note.*)
    }
}
