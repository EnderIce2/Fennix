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

#ifdef FENNIX_DYNAMIC_LOADER
__attribute__((naked, used, no_stack_protector)) void _dl_start()
{
#if defined(__amd64__)
	__asm__(
		"xorq %rbp, %rbp\n" /* Clear rbp */

		"push %rdi\n"
		"push %rsi\n"
		"push %rdx\n"
		"push %rcx\n"
		"push %r8\n"
		"push %r9\n"

		"call __init_print_buffer\n" /* Call __init_print_buffer */
		"call _dl_preload\n"		 /* Call _dl_preload */
		"movl %eax, %edi\n"			 /* Move return value to edi */
		"cmp $0, %edi\n"			 /* Check if return value is 0 */
		"jne _exit\n"				 /* If not, jump to _exit */

		"pop %r9\n"
		"pop %r8\n"
		"pop %rcx\n"
		"pop %rdx\n"
		"pop %rsi\n"
		"pop %rdi\n"

		"call main\n"		/* Call _dl_main */
		"movl %eax, %edi\n" /* Move return value to edi */
		"call _exit\n");	/* Call _exit */
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
#endif
