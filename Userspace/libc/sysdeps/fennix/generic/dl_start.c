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
#define _s __asm__ __volatile__
__attribute__((naked, used, no_stack_protector)) void _dl_start()
{
#if defined(__amd64__)
	_s("xorq %rbp, %rbp\n"); /* Clear rbp */

	_s("pushq %rdi\n");
	_s("pushq %rsi\n");
	_s("pushq %rdx\n");
	_s("pushq %rcx\n");
	_s("pushq %r8\n");
	_s("pushq %r9\n");

	_s("call __init_print_buffer\n"); /* Call __init_print_buffer */
	_s("call _dl_preload\n");		  /* Call _dl_preload */
	_s("movl %eax, %edi\n");		  /* Move return value to edi */
	_s("cmp $0, %edi\n");			  /* Check if return value is 0 */
	_s("jne _exit\n");				  /* If not, jump to _exit */

	_s("popq %r9\n");
	_s("popq %r8\n");
	_s("popq %rcx\n");
	_s("popq %rdx\n");
	_s("popq %rsi\n");
	_s("popq %rdi\n");

	_s("call main\n");		 /* Call _dl_main */
	_s("movl %eax, %edi\n"); /* Move return value to edi */
	_s("call _exit\n");		 /* Call _exit */
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
#undef _s
#endif
