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

#ifndef __FENNIX_API_SYSCALLS_LIST_H__
#define __FENNIX_API_SYSCALLS_LIST_H__

#ifndef syscall0
static inline long syscall0(long syscall)
{
	long ret;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall)
						 : "rcx", "r11", "memory");
	return ret;
}
#endif

#ifndef syscall1
static inline long syscall1(long syscall, long arg1)
{
	long ret;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1)
						 : "rcx", "r11", "memory");
	return ret;
}
#endif

#ifndef syscall2
static inline long syscall2(long syscall, long arg1, long arg2)
{
	long ret;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2)
						 : "rcx", "r11", "memory");
	return ret;
}
#endif

#ifndef syscall3
static inline long syscall3(long syscall, long arg1, long arg2, long arg3)
{
	long ret;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3)
						 : "rcx", "r11", "memory");
	return ret;
}
#endif

#ifndef syscall4
static inline long syscall4(long syscall, long arg1, long arg2, long arg3, long arg4)
{
	long ret;
	register long r10 __asm__("r10") = arg4;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10)
						 : "rcx", "r11", "memory");
	return ret;
}
#endif

#ifndef syscall5
static inline long syscall5(long syscall, long arg1, long arg2, long arg3, long arg4, long arg5)
{
	long ret;
	register long r10 __asm__("r10") = arg4;
	register long r8 __asm__("r8") = arg5;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8)
						 : "rcx", "r11", "memory");
	return ret;
}
#endif

#ifndef syscall6
static inline long syscall6(long syscall, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6)
{
	long ret;
	register long r10 __asm__("r10") = arg4;
	register long r8 __asm__("r8") = arg5;
	register long r9 __asm__("r9") = arg6;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8), "r"(r9)
						 : "rcx", "r11", "memory");
	return ret;
}
#endif

#endif // !__FENNIX_API_SYSCALLS_LIST_H__
