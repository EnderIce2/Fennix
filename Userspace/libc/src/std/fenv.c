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
#include <sys/types.h>
#include <fenv.h>

export int feclearexcept(int excepts)
{
#if defined(__amd64__) || defined(__i386__)
	unsigned int status;
	__asm__ __volatile__("fnstsw %0" : "=m"(*&status));
	status &= ~excepts;
	__asm__ __volatile__("fldcw %0" : : "m"(*&status));
	return 0;
#elif defined(__arm__)
	return 0;
#elif defined(__aarch64__)
	return 0;
#else
#error "Unsupported architecture"
#endif
}

export int fegetenv(fenv_t *envp)
{
#if defined(__amd64__) || defined(__i386__)
	__asm__ __volatile__("fnstenv %0" : "=m"(*envp));
	return 0;
#elif defined(__arm__)
	return 0;
#elif defined(__aarch64__)
	return 0;
#else
#error "Unsupported architecture"
#endif
}

export int fegetexceptflag(fexcept_t *flagp, int excepts)
{
#if defined(__amd64__) || defined(__i386__)
	unsigned int status;
	__asm__ __volatile__("fnstsw %0" : "=m"(*&status));
	*flagp = status & excepts;
	return 0;
#elif defined(__arm__)
	return 0;
#elif defined(__aarch64__)
	return 0;
#else
#error "Unsupported architecture"
#endif
}

export int fegetround(void)
{
#if defined(__amd64__) || defined(__i386__)
	unsigned int cw;
	__asm__ __volatile__("fnstcw %0" : "=m"(*&cw));
	return cw & 0x0C00;
#elif defined(__arm__)
	return FE_TONEAREST;
#elif defined(__aarch64__)
	return FE_TONEAREST;
#else
#error "Unsupported architecture"
#endif
}

export int feholdexcept(fenv_t *envp)
{
#if defined(__amd64__) || defined(__i386__)
	__asm__ __volatile__("fnstenv %0" : "=m"(*envp));
	unsigned int cw;
	__asm__ __volatile__("fnclex");
	__asm__ __volatile__("fnstcw %0" : "=m"(*&cw));
	cw |= 0x3F; /* Set non-stop mode for all exceptions */
	__asm__ __volatile__("fldcw %0" : : "m"(*&cw));
	return 0;
#elif defined(__arm__)
	return 0;
#elif defined(__aarch64__)
	return 0;
#else
#error "Unsupported architecture"
#endif
}

export int feraiseexcept(int excepts)
{
#if defined(__amd64__) || defined(__i386__)
	unsigned int status;
	__asm__ __volatile__("fnstsw %0" : "=m"(*&status));
	status |= excepts;
	__asm__ __volatile__("fldcw %0" : : "m"(*&status));
	return 0;
#elif defined(__arm__)
	return 0;
#elif defined(__aarch64__)
	return 0;
#else
#error "Unsupported architecture"
#endif
}

export int fesetenv(const fenv_t *envp)
{
#if defined(__amd64__) || defined(__i386__)
	__asm__ __volatile__("fldenv %0" : : "m"(*envp));
	return 0;
#elif defined(__arm__)
	return 0;
#elif defined(__aarch64__)
	return 0;
#else
#error "Unsupported architecture"
#endif
}

export int fesetexceptflag(const fexcept_t *flagp, int excepts)
{
#if defined(__amd64__) || defined(__i386__)
	unsigned int status;
	__asm__ __volatile__("fnstsw %0" : "=m"(*&status));
	status &= ~excepts;
	status |= *flagp & excepts;
	__asm__ __volatile__("fldcw %0" : : "m"(*&status));
	return 0;
#elif defined(__arm__)
	return 0;
#elif defined(__aarch64__)
	return 0;
#else
#error "Unsupported architecture"
#endif
}

export int fesetround(int round)
{
#if defined(__amd64__) || defined(__i386__)
	unsigned int cw;
	if (round != FE_TONEAREST && round != FE_DOWNWARD && round != FE_UPWARD && round != FE_TOWARDZERO)
		return -1;
	__asm__ __volatile__("fnstcw %0" : "=m"(*&cw));
	cw &= ~0x0C00;
	cw |= round;
	__asm__ __volatile__("fldcw %0" : : "m"(*&cw));
	return 0;
#elif defined(__arm__)
	return 0;
#elif defined(__aarch64__)
	return 0;
#else
#error "Unsupported architecture"
#endif
}

export int fetestexcept(int excepts)
{
#if defined(__amd64__) || defined(__i386__)
	unsigned int status;
	__asm__ __volatile__("fnstsw %0" : "=m"(*&status));
	return status & excepts;
#elif defined(__arm__)
	return 0;
#elif defined(__aarch64__)
	return 0;
#else
#error "Unsupported architecture"
#endif
}

export int feupdateenv(const fenv_t *envp)
{
#if defined(__amd64__) || defined(__i386__)
	unsigned int status;
	__asm__ __volatile__("fnstsw %0" : "=m"(*&status));
	__asm__ __volatile__("fldenv %0" : : "m"(*envp));
	__asm__ __volatile__("fldcw %0" : : "m"(*&status));
	return 0;
#elif defined(__arm__)
	return 0;
#elif defined(__aarch64__)
	return 0;
#else
#error "Unsupported architecture"
#endif
}
