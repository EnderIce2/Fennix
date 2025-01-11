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

OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)

ENTRY(_start)

SECTIONS
{
	_bootstrap_start = .;
	.text.boot :
	{
		*(.text.boot)
		. += CONSTANT(MAXPAGESIZE);
		_bss_start = .;
		*(.text.bss)
		_bss_end = .;
	}
	_bootstrap_end = .;

	_kernel_start = .;
	_kernel_text_start = .;
	.text :
	{
		KEEP(*(.text.boot))
		*(.text .text.*)
	}
	. = ALIGN(4096);
	_kernel_text_end = .;

	_kernel_data_start = .;
	.data :
	{
		*(.data .data.*)
	}
	. = ALIGN(4096);
	_kernel_data_end = .;

	_kernel_rodata_start = .;
	.rodata :
	{
		*(.rodata .rodata.*)
	}
	. = ALIGN(4096);

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
	_kernel_rodata_end = .;

	_kernel_bss_start = .;
	.bss :
	{
		*(.bss .bss.*)
	}
	. = ALIGN(4096);
	_kernel_bss_end = .;
	_kernel_end = .;
	_bss_size = _kernel_end - _kernel_rodata_end;

	/DISCARD/ :
	{
		*(.comment*)
	}
}
