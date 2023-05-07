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

OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

KERNEL_VMA = 0xFFFFFFFF80000000;

ENTRY(_start)

SECTIONS
{
    . = 0x100000;

    _bootstrap_start = .;
    .bootstrap :
    {
        *(.multiboot)
        *(.multiboot2)
        *(.bootstrap .bootstrap.*)
    }
    . += CONSTANT(MAXPAGESIZE);
    _bootstrap_end = ALIGN(CONSTANT(MAXPAGESIZE));

    . += KERNEL_VMA;
    . += CONSTANT(MAXPAGESIZE);
    _kernel_start = ALIGN(CONSTANT(MAXPAGESIZE));
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
    _kernel_rodata_end = ALIGN(CONSTANT(MAXPAGESIZE));
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
