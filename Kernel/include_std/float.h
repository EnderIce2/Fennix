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

#pragma once

/* Stubs */
#define FLT_RADIX 2
#if a64
#define DBL_MANT_DIG 53
#define DBL_MAX_10_EXP 308
#define DBL_MAX 1.7976931348623157e+308
#elif a32
#define DBL_MANT_DIG 24
#define DBL_MAX_10_EXP 38
#define DBL_MAX 3.4028234663852886e+38
#endif
