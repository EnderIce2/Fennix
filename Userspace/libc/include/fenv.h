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

#ifndef _FENV_H
#define _FENV_H

typedef int fenv_t;
typedef int fexcept_t;

#define FE_DIVBYZERO 0x04
#define FE_INEXACT 0x20
#define FE_INVALID 0x01
#define FE_OVERFLOW 0x08
#define FE_UNDERFLOW 0x10

#define FE_ALL_EXCEPT (FE_DIVBYZERO | FE_INEXACT | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW)

#define FE_DOWNWARD 0x400
#define FE_TONEAREST 0x000
#define FE_TOWARDZERO 0xC00
#define FE_UPWARD 0x800

#define FE_DFL_ENV ((const fenv_t *)-1)

int feclearexcept(int excepts);
int fegetenv(fenv_t *envp);
int fegetexceptflag(fexcept_t *flagp, int excepts);
int fegetround(void);
int feholdexcept(fenv_t *envp);
int feraiseexcept(int excepts);
int fesetenv(const fenv_t *envp);
int fesetexceptflag(const fexcept_t *flagp, int excepts);
int fesetround(int round);
int fetestexcept(int excepts);
int feupdateenv(const fenv_t *envp);

#endif // _FENV_H
