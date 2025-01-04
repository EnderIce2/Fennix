/*
	This file is part of Fennix Userspace.

	Fennix Userspace is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Userspace is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Userspace. If not, see <https://www.gnu.org/licenses/>.
*/

typedef void (*fct)(void);
#define asm __asm__ __volatile__
__attribute__((__visibility__("hidden"))) extern void (*__preinit_array_start[])(void) __attribute__((weak));
__attribute__((__visibility__("hidden"))) extern void (*__preinit_array_end[])(void) __attribute__((weak));
__attribute__((__visibility__("hidden"))) extern void (*__init_array_start[])(void) __attribute__((weak));
__attribute__((__visibility__("hidden"))) extern void (*__init_array_end[])(void) __attribute__((weak));
__attribute__((__visibility__("hidden"))) extern void (*__fini_array_start[])(void) __attribute__((weak));
__attribute__((__visibility__("hidden"))) extern void (*__fini_array_end[])(void) __attribute__((weak));
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
} __abi_tag __attribute__((aligned(4), section(".note.ABI-tag"))) = {
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
} __build_id __attribute__((aligned(4), section(".note.build-id"))) = {
	.header = {
		.n_namesz = 4,							 /* "FNX" + '\0' */
		.n_descsz = sizeof(__UINT8_TYPE__) * 20, /* Description Size */
		.n_type = NT_FNX_BUILD_ID,				 /* Type */
	},
	.name = "FNX",
	.desc = HASH_BYTES(LIBC_GIT_COMMIT),
};

void __crt_init_array(void)
{
	for (fct *func = __init_array_start; func != __init_array_end; func++)
		(*func)();
}

void __crt_fini_array(void)
{
	for (fct *func = __fini_array_start; func != __fini_array_end; func++)
		(*func)();
}

__attribute__((naked, used, no_stack_protector, section(".text"))) void _start()
{
	asm("movq $0, %rbp\n"
		"pushq %rbp\n"
		"pushq %rbp\n"
		"movq %rsp, %rbp\n"

		"pushq %rcx\n"
		"pushq %rdx\n"
		"pushq %rsi\n"
		"pushq %rdi\n"

		"call __libc_init\n"
		"call __crt_init_array\n"

		"popq %rdi\n"
		"popq %rsi\n"
		"popq %rdx\n"
		"popq %rcx\n"

		"call main\n"

		"pushq %rax\n"
		"call __crt_fini_array\n"
		"popq %rax\n"

		"movl %eax, %edi\n"
		"call _exit\n");
}

int main(int argc, char *argv[], char *envp[])
{
	(void)argc;
	(void)argv;
	(void)envp;
	/* FIXME: show printf license notice and some help and commands? */
	return -1;
}
