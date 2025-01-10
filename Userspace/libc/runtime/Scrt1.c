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

extern void (*__preinit_array_start[])(void) __attribute__((weak));
extern void (*__preinit_array_end[])(void) __attribute__((weak));
extern void (*__init_array_start[])(void) __attribute__((weak));
extern void (*__init_array_end[])(void) __attribute__((weak));
extern void (*__fini_array_start[])(void) __attribute__((weak));
extern void (*__fini_array_end[])(void) __attribute__((weak));

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
#if defined(__amd64__)
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
#elif defined(__i386__)
#warning "i386 _start not implemented"
#elif defined(__arm__)
#warning "arm _start not implemented"
#elif defined(__aarch64__)
#warning "aarch64 _start not implemented"
#else
#error "Unsupported architecture"
#endif
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
} __abi_tag __attribute__((aligned(4), section(".note.ABI-tag"))) = {
	.header = {
		.n_namesz = 4,							 /* "FNX" + '\0' */
		.n_descsz = sizeof(__UINT32_TYPE__) * 4, /* Description Size */
		.n_type = NT_FNX_ABI_TAG,				 /* Type */
	},
	.name = "FNX",
	.desc = {0, 0, 0, 0},
};
