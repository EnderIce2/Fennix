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

PF_R = 0x4;
PF_W = 0x2;
PF_X = 0x1;

PHDRS
{
	bootstrap   PT_LOAD    FLAGS( PF_R | PF_W /*| PF_X*/ );
	text        PT_LOAD    FLAGS( PF_R | PF_X );
	data        PT_LOAD    FLAGS( PF_R | PF_W );
	rodata      PT_LOAD    FLAGS( PF_R );
	bss         PT_LOAD    FLAGS( PF_R | PF_W );
}

SECTIONS
{
	. = 0x80000;
	_bootstrap_start = .;
	.bootstrap :
	{
		*(.bootstrap.boot .bootstrap.boot.*)
		*(.bootstrap .bootstrap.*)
	} :bootstrap
	_bootstrap_end = .;

	_kernel_start = .;
	_kernel_text_start = .;
	.text :
	{
		*(.text .text.*)
	} :text
	_kernel_text_end = .;

	_kernel_data_start = .;
	.data :
	{
		*(.data .data.*)
	} :data

	.eh_frame :
	{
		KEEP (*(.eh_frame .eh_frame.*))
	} :data

	.gcc_except_table :
	{
		KEEP (*(.gcc_except_table .gcc_except_table.*))
	} :data
	_kernel_data_end = .;

	_kernel_rodata_start = .;
	.rodata :
	{
		*(.rodata .rodata.*)
	} :rodata

	.init_array :
	{
		PROVIDE_HIDDEN(__init_array_start = .);
		KEEP(*(.init_array .ctors))
		KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
		PROVIDE_HIDDEN (__init_array_end = .);
	} :rodata

	.fini_array :
	{
		PROVIDE_HIDDEN(__fini_array_start = .);
		KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
		KEEP(*(.fini_array .dtors))
		PROVIDE_HIDDEN (__fini_array_end = .);
	} :rodata

	.eh_frame_hdr :
	{
		*(.eh_frame_hdr .eh_frame_hdr.*)
	} :rodata

	.eh_frame :
	{
		KEEP (*(.eh_frame .eh_frame.*))
	} :rodata

	.gcc_except_table :
	{
		KEEP (*(.gcc_except_table .gcc_except_table.*))
	} :rodata
	_kernel_rodata_end = .;

	_kernel_bss_start = .;
	.bss :
	{
		*(COMMON)
		*(.bss .bss.*)
	} :bss
	_kernel_bss_end = .;
	_kernel_bss_size = SIZEOF(.bss);
	_kernel_end = .;

	/DISCARD/ :
	{
		*(.comment*)
	}
}
