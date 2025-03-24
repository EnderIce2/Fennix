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

#include <bits/libc.h>

// const char __interp[] __attribute__((section(".interp"))) = "/boot/fennix.elf";

#ifndef LIBC_GIT_COMMIT
#define LIBC_GIT_COMMIT "0000000000000000000000000000000000000000"
#endif

#define HEX_DIGIT(c) (((c) >= '0' && (c) <= '9') ? ((c) - '0') : ((c) - 'a' + 10))
#define CONVERT_TO_BYTE(h, l) ((HEX_DIGIT(h) << 4) | HEX_DIGIT(l))
#define HASH_BYTES(hex)                 \
	{CONVERT_TO_BYTE(hex[0], hex[1]),   \
	 CONVERT_TO_BYTE(hex[2], hex[3]),   \
	 CONVERT_TO_BYTE(hex[4], hex[5]),   \
	 CONVERT_TO_BYTE(hex[6], hex[7]),   \
	 CONVERT_TO_BYTE(hex[8], hex[9]),   \
	 CONVERT_TO_BYTE(hex[10], hex[11]), \
	 CONVERT_TO_BYTE(hex[12], hex[13]), \
	 CONVERT_TO_BYTE(hex[14], hex[15]), \
	 CONVERT_TO_BYTE(hex[16], hex[17]), \
	 CONVERT_TO_BYTE(hex[18], hex[19]), \
	 CONVERT_TO_BYTE(hex[20], hex[21]), \
	 CONVERT_TO_BYTE(hex[22], hex[23]), \
	 CONVERT_TO_BYTE(hex[24], hex[25]), \
	 CONVERT_TO_BYTE(hex[26], hex[27]), \
	 CONVERT_TO_BYTE(hex[28], hex[29]), \
	 CONVERT_TO_BYTE(hex[30], hex[31]), \
	 CONVERT_TO_BYTE(hex[32], hex[33]), \
	 CONVERT_TO_BYTE(hex[34], hex[35]), \
	 CONVERT_TO_BYTE(hex[36], hex[37]), \
	 CONVERT_TO_BYTE(hex[38], hex[39])}

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

const struct
{
	Elf_Nhdr header;
	char name[4];
	__UINT8_TYPE__ desc[20];
} __build_id __attribute__((used, aligned(4), section(".note.build-id"))) = {
	.header = {
		.n_namesz = 4,							 /* "FNX" + '\0' */
		.n_descsz = sizeof(__UINT8_TYPE__) * 20, /* Description Size */
		.n_type = NT_FNX_BUILD_ID,				 /* Type */
	},
	.name = "FNX",
	.desc = HASH_BYTES(LIBC_GIT_COMMIT),
};

void __init_print_buffer();
void __fini_print_buffer();

__attribute__((no_stack_protector)) _Noreturn void _exit(int status)
{
	__fini_print_buffer();
	sysdep(Exit)(status);
	/* At this point, the program *SHOULD* have exited. */
	__builtin_unreachable();
}

__attribute__((no_stack_protector)) _Noreturn void _Exit(int status) { _exit(status); }
