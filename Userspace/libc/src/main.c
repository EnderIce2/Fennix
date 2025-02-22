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

#include "../runtime/crt1.c"

const char __interp[] __attribute__((section(".interp"))) = "/lib/ld.so";

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

const struct
{
	Elf_Nhdr header;
	char name[4];
	__UINT8_TYPE__ desc[20];
} __build_id __attribute__((aligned(4), section(".note.build-id"))) = {
	.header = {
		.n_namesz = 4,							 /* "FNX" + '\0' */
		.n_descsz = sizeof(__UINT8_TYPE__) * 20, /* Description Size */
		.n_type = NT_FNX_BUILD_ID,				 /* Type */
	},
	.name = "FNX",
	.desc = HASH_BYTES(LIBC_GIT_COMMIT),
};

int main(int argc, char *argv[], char *envp[])
{
	(void)argc;
	(void)argv;
	(void)envp;
	/* FIXME: show printf license notice and some help and commands? */
	return -1;
}
