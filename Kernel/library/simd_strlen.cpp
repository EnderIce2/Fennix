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

#include <convert.h>

#include <memory.hpp>
#include <limits.h>
#include <debug.h>
#include <cpu.hpp>

/*
TODO: Replace these functions with even more optimized versions.
	  The current versions are fast but not as fast as they could be and also we need implementation for avx, not only sse.
*/

long unsigned __strlen(const char s[])
{
	size_t ret = (size_t)s;

#if defined(__amd64__) || defined(__i386__)
	asmv("._strlenLoop:"
		 "cmpb $0, 0(%1)\n"
		 "jz ._strlenExit\n"
		 "cmpb $0, 1(%1)\n"
		 "jz .scmp1\n"
		 "cmpb $0, 2(%1)\n"
		 "jz .scmp2\n"
		 "cmpb $0, 3(%1)\n"
		 "jz .scmp3\n"
		 "cmpb $0, 4(%1)\n"
		 "jz .scmp4\n"
		 "cmpb $0, 5(%1)\n"
		 "jz .scmp5\n"
		 "cmpb $0, 6(%1)\n"
		 "jz .scmp6\n"
		 "cmpb $0, 7(%1)\n"
		 "jz .scmp7\n"

		 "add $8, %1\n"
		 "jmp ._strlenLoop\n"

		 ".scmp1:"
		 "inc %1\n"
		 "jmp ._strlenExit\n"
		 ".scmp2:"
		 "add $2, %1\n"
		 "jmp ._strlenExit\n"
		 ".scmp3:"
		 "add $3, %1\n"
		 "jmp ._strlenExit\n"
		 ".scmp4:"
		 "add $4, %1\n"
		 "jmp ._strlenExit\n"
		 ".scmp5:"
		 "add $5, %1\n"
		 "jmp ._strlenExit\n"
		 ".scmp6:"
		 "add $6, %1\n"
		 "jmp ._strlenExit\n"
		 ".scmp7:"
		 "add $7, %1\n"

		 "._strlenExit:"
		 "mov %1, %0\n"
		 : "=r"(ret) : "0"(ret));
#endif

	return ret - (size_t)s;
}

long unsigned strlen_sse(const char s[])
{
	return __strlen(s);
}

long unsigned strlen_sse2(const char s[])
{
	return __strlen(s);
}

long unsigned strlen_sse3(const char s[])
{
	return __strlen(s);
}

long unsigned strlen_ssse3(const char s[])
{
	return __strlen(s);
}

long unsigned strlen_sse4_1(const char s[])
{
	return __strlen(s);
}

long unsigned strlen_sse4_2(const char s[])
{
	size_t ret = 0;
#if defined(__amd64__) || defined(__i386__)
	asmv("mov $-16, %0\n"
		 "pxor %%xmm0, %%xmm0\n"
		 ".strlen42Loop:"
		 "add $16, %0\n"
		 "pcmpistri $0x08, (%0,%1), %%xmm0\n"
		 "jnz .strlen42Loop\n"
		 "add %2, %0\n"
		 : "=a"(ret)
		 : "d"((size_t)s), "c"((size_t)s));
#endif
	return ret;
}
