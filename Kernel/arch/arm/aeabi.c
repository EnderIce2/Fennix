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

unsigned __aeabi_unwind_cpp_pr1(void) { return 0; }
unsigned __aeabi_atexit(void) { return 0; }
unsigned __cxa_end_cleanup(void) { return 0; }
unsigned __aeabi_unwind_cpp_pr0(void) { return 0; }

int __aeabi_dcmple(long double a, long double b)
{
	return a <= b;
}

long long __aeabi_d2lz(double a)
{
	return (long long)a;
}

int __aeabi_dcmplt(long double a, long double b)
{
	return a < b;
}

typedef union
{
	double d;
	struct
	{
		__UINT64_TYPE__ mantissa : 52;
		__UINT64_TYPE__ exponent : 11;
		__UINT64_TYPE__ sign : 1;
	} parts;
} aeabi_double_t;

typedef union
{
	float f;
	struct
	{
		__UINT32_TYPE__ mantissa : 23;
		__UINT32_TYPE__ exponent : 8;
		__UINT32_TYPE__ sign : 1;
	} parts;
} aeabi_float_t;

aeabi_double_t __aeabi_ddiv(aeabi_double_t a, aeabi_double_t b)
{
	aeabi_double_t result;
	result.d = a.d / b.d;
	return result;
}

aeabi_double_t __aeabi_dmul(aeabi_double_t a, aeabi_double_t b)
{
	aeabi_double_t result;
	result.d = a.d * b.d;
	return result;
}

aeabi_double_t __aeabi_dadd(aeabi_double_t a, aeabi_double_t b)
{
	aeabi_double_t result;
	result.d = a.d + b.d;
	return result;
}

int __aeabi_dcmpgt(aeabi_double_t a, aeabi_double_t b)
{
	return a.d > b.d;
}

int __aeabi_dcmpge(aeabi_double_t a, aeabi_double_t b)
{
	return a.d >= b.d;
}

aeabi_double_t __aeabi_dsub(aeabi_double_t a, aeabi_double_t b)
{
	aeabi_double_t result;
	result.d = a.d - b.d;
	return result;
}

aeabi_double_t __aeabi_i2d(int a)
{
	aeabi_double_t result;
	result.d = (double)a;
	return result;
}

aeabi_double_t __aeabi_l2d(long long a)
{
	aeabi_double_t result;
	result.d = (double)a;
	return result;
}

int __aeabi_dcmpeq(aeabi_double_t a, aeabi_double_t b)
{
	return a.d == b.d;
}

int __aeabi_d2iz(aeabi_double_t a)
{
	return (int)a.d;
}

struct ldivmod_result
{
	long quot;
	long rem;
};

struct ldivmod_result __aeabi_ldivmod(long numerator, long denominator)
{
	struct ldivmod_result result;
	result.quot = numerator / denominator;
	result.rem = numerator % denominator;
	return result;
}

signed __aeabi_idiv(signed numerator, signed denominator)
{
	return numerator / denominator;
}

signed __aeabi_idivmod(signed numerator, signed denominator)
{
	signed quotient = numerator / denominator;
	signed remainder = numerator % denominator;
	return (quotient << 16) | remainder;
}

unsigned __aeabi_uidiv(unsigned numerator, unsigned denominator)
{
	return numerator / denominator;
}

unsigned __aeabi_uidivmod(unsigned numerator, unsigned denominator)
{
	unsigned quotient = numerator / denominator;
	unsigned remainder = numerator % denominator;
	return (quotient << 16) | remainder;
}

__UINT64_TYPE__ __udivmoddi4(__UINT64_TYPE__ numerator, __UINT64_TYPE__ denominator, __UINT64_TYPE__ *remainder)
{
	__UINT64_TYPE__ quotient = 0;
	__UINT64_TYPE__ bit = 1;

	if (denominator == 0)
	{
		*remainder = numerator;
		return ~0ULL;
	}

	while (denominator < numerator && (denominator & (1ULL << 63)) == 0)
	{
		denominator <<= 1;
		bit <<= 1;
	}

	while (bit)
	{
		if (numerator >= denominator)
		{
			numerator -= denominator;
			quotient |= bit;
		}
		denominator >>= 1;
		bit >>= 1;
	}

	if (remainder)
		*remainder = numerator;

	return quotient;
}

struct udivmod_result
{
	__UINT64_TYPE__ quot;
	__UINT64_TYPE__ rem;
};

struct udivmod_result __aeabi_uldivmod(__UINT64_TYPE__ numerator, __UINT64_TYPE__ denominator)
{
	struct udivmod_result result;
	result.quot = __udivmoddi4(numerator, denominator, &result.rem);
	return result;
}

int __aeabi_fcmpgt(aeabi_float_t a, aeabi_float_t b)
{
	return a.f > b.f;
}

aeabi_float_t __aeabi_fmul(aeabi_float_t a, aeabi_float_t b)
{
	aeabi_float_t result;
	result.f = a.f * b.f;
	return result;
}

aeabi_float_t __aeabi_ui2f(unsigned a)
{
	aeabi_float_t result;
	result.f = (float)a;
	return result;
}

unsigned __aeabi_f2uiz(aeabi_float_t a)
{
	return (unsigned)a.f;
}

aeabi_float_t __aeabi_d2f(aeabi_double_t a)
{
	aeabi_float_t result;
	result.f = (float)a.d;
	return result;
}

aeabi_float_t __aeabi_fadd(aeabi_float_t a, aeabi_float_t b)
{
	aeabi_float_t result;
	result.f = a.f + b.f;
	return result;
}

aeabi_float_t __aeabi_fdiv(aeabi_float_t a, aeabi_float_t b)
{
	aeabi_float_t result;
	result.f = a.f / b.f;
	return result;
}

aeabi_double_t __aeabi_f2d(aeabi_float_t a)
{
	aeabi_double_t result;
	result.d = (double)a.f;
	return result;
}

aeabi_float_t __aeabi_fsub(aeabi_float_t a, aeabi_float_t b)
{
	aeabi_float_t result;
	result.f = a.f - b.f;
	return result;
}

int __aeabi_fcmplt(aeabi_float_t a, aeabi_float_t b)
{
	return a.f < b.f;
}

int __aeabi_fcmpeq(aeabi_float_t a, aeabi_float_t b)
{
	return a.f == b.f;
}
