/*
	This file is part of Fennix C Library.

	Fennix C Library is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix C Library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix C Library. If not, see <https://www.gnu.org/licenses/>.
*/

__attribute__((naked, used, no_stack_protector, section(".text"))) void _start()
{
#warning "crt1.c: _start() is not implemented yet"
}

/* These are declared in GNU ld */
enum
{
	NT_FNX_ABI_TAG = 1,
	NT_FNX_VERSION = 2,
	NT_FNX_BUILD_ID = 3,
	NT_FNX_ARCH = 4
};

typedef struct Elf_Nhdr
{
	__UINT32_TYPE__ n_namesz;
	__UINT32_TYPE__ n_descsz;
	__UINT32_TYPE__ n_type;
	char n_name[];
} __attribute__((packed)) Elf_Nhdr;

const struct
{
	Elf_Nhdr header;
	char name[4];
	__UINT32_TYPE__ desc[4];
} __abi_tag __attribute__((used, aligned(4), section(".note.ABI-tag"))) = {
	.header = {
		.n_namesz = 4,							 /* "FNX" + '\0' */
		.n_descsz = sizeof(__UINT32_TYPE__) * 4, /* Description Size */
		.n_type = NT_FNX_ABI_TAG,				 /* Type */
	},
	.name = "FNX",
	.desc = {0, 0, 0, 0},
};
