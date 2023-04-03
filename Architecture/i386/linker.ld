/*
   This file is part of Fennix Kernel.

   Fennix Kernel is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of
   the License, or (at your option) any later version.

   Fennix Kernel is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)

ENTRY(MB2_start)

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
    . += CONSTANT(MAXPAGESIZE);

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
