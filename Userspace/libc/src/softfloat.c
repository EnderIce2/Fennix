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

float __addsf3(float a, float b);
double __adddf3(double a, double b);
long double __addtf3(long double a, long double b);
long double __addxf3(long double a, long double b);
float __subsf3(float a, float b);
double __subdf3(double a, double b);
long double __subtf3(long double a, long double b);
long double __subxf3(long double a, long double b);
float __mulsf3(float a, float b);
double __muldf3(double a, double b);
long double __multf3(long double a, long double b);
long double __mulxf3(long double a, long double b);
float __divsf3(float a, float b);
double __divdf3(double a, double b);
long double __divtf3(long double a, long double b);
long double __divxf3(long double a, long double b);
float __negsf2(float a);
double __negdf2(double a);
long double __negtf2(long double a);
long double __negxf2(long double a);
double __extendsfdf2(float a);
long double __extendsftf2(float a);
long double __extendsfxf2(float a);
long double __extenddftf2(double a);
long double __extenddfxf2(double a);
double __truncxfdf2(long double a);
double __trunctfdf2(long double a) { return (double)a; }
float __truncxfsf2(long double a);
float __trunctfsf2(long double a);
float __truncdfsf2(double a);
int __fixsfsi(float a);
int __fixdfsi(double a);
int __fixtfsi(long double a);
int __fixxfsi(long double a);
long __fixsfdi(float a);
long __fixdfdi(double a);
long __fixtfdi(long double a);
long __fixxfdi(long double a);
long long __fixsfti(float a);
long long __fixdfti(double a);
long long __fixtfti(long double a);
long long __fixxfti(long double a);
unsigned int __fixunssfsi(float a);
unsigned int __fixunsdfsi(double a);
unsigned int __fixunstfsi(long double a);
unsigned int __fixunsxfsi(long double a);
unsigned long __fixunssfdi(float a);
unsigned long __fixunsdfdi(double a);
unsigned long __fixunstfdi(long double a);
unsigned long __fixunsxfdi(long double a);
unsigned long long __fixunssfti(float a);
unsigned long long __fixunsdfti(double a);
unsigned long long __fixunstfti(long double a);
unsigned long long __fixunsxfti(long double a);
float __floatsisf(int i);
double __floatsidf(int i);
long double __floatsitf(int i);
long double __floatsixf(int i);
float __floatdisf(long i);
double __floatdidf(long i);
long double __floatditf(long i);
long double __floatdixf(long i);
float __floattisf(long long i);
double __floattidf(long long i);
long double __floattitf(long long i);
long double __floattixf(long long i);
float __floatunsisf(unsigned int i);
double __floatunsidf(unsigned int i);
long double __floatunsitf(unsigned int i);
long double __floatunsixf(unsigned int i);
float __floatundisf(unsigned long i);
double __floatundidf(unsigned long i);
long double __floatunditf(unsigned long i);
long double __floatundixf(unsigned long i);
float __floatuntisf(unsigned long long i);
double __floatuntidf(unsigned long long i);
long double __floatuntitf(unsigned long long i);
long double __floatuntixf(unsigned long long i);
// void __fixsfbitint(UBILtype *r, int32_t rprec, float a);
// void __fixdfbitint(UBILtype *r, int32_t rprec, double a);
// void __fixxfbitint(UBILtype *r, int32_t rprec, __float80 a);
// void __fixtfbitint(UBILtype *r, int32_t rprec, _Float128 a);
// float __floatbitintsf(UBILtype *i, int32_t iprec);
// double __floatbitintdf(UBILtype *i, int32_t iprec);
// __float80 __floatbitintxf(UBILtype *i, int32_t iprec);
// _Float128 __floatbitinttf(UBILtype *i, int32_t iprec);
// _Float16 __floatbitinthf(UBILtype *i, int32_t iprec);
// __bf16 __floatbitintbf(UBILtype *i, int32_t iprec);
int __cmpsf2(float a, float b);
int __cmpdf2(double a, double b);
int __cmptf2(long double a, long double b);
int __unordsf2(float a, float b);
int __unorddf2(double a, double b);
int __unordtf2(long double a, long double b);
int __eqsf2(float a, float b);
int __eqdf2(double a, double b);
int __eqtf2(long double a, long double b);
int __nesf2(float a, float b);
int __nedf2(double a, double b);
int __netf2(long double a, long double b);
int __gesf2(float a, float b);
int __gedf2(double a, double b);
int __getf2(long double a, long double b);
int __ltsf2(float a, float b);
int __ltdf2(double a, double b);
int __lttf2(long double a, long double b) { return a < b; }
int __lesf2(float a, float b);
int __ledf2(double a, double b);
int __letf2(long double a, long double b) { return a > b; }
int __gtsf2(float a, float b);
int __gtdf2(double a, double b);
int __gttf2(long double a, long double b) { return a > b; }
float __powisf2(float a, int b);
double __powidf2(double a, int b);
long double __powitf2(long double a, int b);
long double __powixf2(long double a, int b);
// complex float __mulsc3(float a, float b, float c, float d);
// complex double __muldc3(double a, double b, double c, double d);
// complex long double __multc3(long double a, long double b, long double c, long double d);
// complex long double __mulxc3(long double a, long double b, long double c, long double d);
// complex float __divsc3(float a, float b, float c, float d);
// complex double __divdc3(double a, double b, double c, double d);
// complex long double __divtc3(long double a, long double b, long double c, long double d);
// complex long double __divxc3(long double a, long double b, long double c, long double d);

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

int __aeabi_dcmpun(aeabi_double_t a, aeabi_double_t b)
{
	return __unorddf2(a.d, b.d);
}
