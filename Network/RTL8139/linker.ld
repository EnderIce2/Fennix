OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386:x86-64)

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
