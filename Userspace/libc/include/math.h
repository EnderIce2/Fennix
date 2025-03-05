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

#ifndef _MATH_H
#define _MATH_H

#include <float.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#if FLT_EVAL_METHOD == 0
	typedef float float_t;
	typedef double double_t;
#elif FLT_EVAL_METHOD == 1
typedef float float_t;
typedef float double_t;
#elif FLT_EVAL_METHOD == 2
typedef long double float_t;
typedef long double double_t;
#else
#error "Unsupported FLT_EVAL_METHOD"
#endif

#define fpclassify(x) __builtin_fpclassify(FP_NAN, FP_INFINITE, FP_NORMAL, FP_SUBNORMAL, FP_ZERO, (x))
#define isfinite(x) __builtin_isfinite(x)
#define isgreater(x, y) __builtin_isgreater(x, y)
#define isgreaterequal(x, y) __builtin_isgreaterequal(x, y)
#define isinf(x) __builtin_isinf(x)
#define isless(x, y) __builtin_isless(x, y)
#define islessequal(x, y) __builtin_islessequal(x, y)
#define islessgreater(x, y) __builtin_islessgreater(x, y)
#define isnan(x) __builtin_isnan(x)
#define isnormal(x) __builtin_isnormal(x)
#define isunordered(x, y) __builtin_isunordered(x, y)
#define signbit(x) __builtin_signbit(x)

#define M_E 2.71828182845904523536
#define M_El 2.71828182845904523536028747135266249775724709369995L
#define M_EGAMMA 0.57721566490153286060
#define M_EGAMMAl 0.57721566490153286060651209008240243104215933593992L
#define M_LOG2E 1.44269504088896340736
#define M_LOG2El 1.44269504088896340735992468100189213742664595415299L
#define M_LOG10E 0.43429448190325182765
#define M_LOG10El 0.43429448190325182765112891891660508229439700580366L
#define M_LN2 0.69314718055994530942
#define M_LN2l 0.69314718055994530941723212145817656807550013436026L
#define M_LN10 2.30258509299404568402
#define M_LN10l 2.30258509299404568401799145468436420760110148862877L
#define M_PHI 1.61803398874989484820
#define M_PHIl 1.61803398874989484820458683436563811772030917980576L
#define M_PI 3.14159265358979323846
#define M_PIl 3.14159265358979323846264338327950288419716939937510L
#define M_PI_2 1.57079632679489661923
#define M_PI_2l 1.57079632679489661923132169163975144209858469968755L
#define M_PI_4 0.78539816339744830962
#define M_PI_4l 0.78539816339744830961566084581987572104929234984378L
#define M_1_PI 0.31830988618379067154
#define M_1_PIl 0.31830988618379067153776752674502872406891929148091L
#define M_1_SQRTPI 0.56418958354775628695
#define M_1_SQRTPIl 0.56418958354775628694807945156077258584405062932899L
#define M_2_PI 0.63661977236758134308
#define M_2_PIl 0.63661977236758134307553505349005744813783858296182L
#define M_2_SQRTPI 1.12837916709551257390
#define M_2_SQRTPIl 1.12837916709551257389615890312154517168810125865798L
#define M_SQRT2 1.41421356237309504880
#define M_SQRT2l 1.41421356237309504880168872420969807856967187537694L
#define M_SQRT3 1.73205080756887729352
#define M_SQRT3l 1.73205080756887729352744634150587236694280525381038L
#define M_SQRT1_2 0.70710678118654752440
#define M_SQRT1_2l 0.70710678118654752440084436210484903928483593768847L
#define M_SQRT1_3 0.57735026918962576450
#define M_SQRT1_3l 0.57735026918962576450914878050195745564760175127013L

#define HUGE_VAL (__builtin_huge_val())
#define HUGE_VALF (__builtin_huge_valf())
#define HUGE_VALL (__builtin_huge_vall())

#define FP_INFINITE 1
#define FP_NAN 2
#define FP_NORMAL 3
#define FP_SUBNORMAL 4
#define FP_ZERO 5

#define FP_ILOGB0 (-2147483647 - 1)
#define FP_ILOGBNAN 2147483647

#define MATH_ERRNO 1
#define MATH_ERREXCEPT 2

#define math_errhandling (MATH_ERRNO | MATH_ERREXCEPT)

	extern int signgam;

	double acos(double x);
	float acosf(float x);
	double acosh(double x);
	float acoshf(float x);
	long double acoshl(long double x);
	long double acosl(long double x);
	double asin(double x);
	float asinf(float x);
	double asinh(double x);
	float asinhf(float x);
	long double asinhl(long double x);
	long double asinl(long double x);
	double atan(double x);
	double atan2(double y, double x);
	float atan2f(float y, float x);
	long double atan2l(long double y, long double x);
	float atanf(float x);
	double atanh(double x);
	float atanhf(float x);
	long double atanhl(long double x);
	long double atanl(long double x);
	double cbrt(double x);
	float cbrtf(float x);
	long double cbrtl(long double x);
	double ceil(double x);
	float ceilf(float x);
	long double ceill(long double x);
	double copysign(double x, double y);
	float copysignf(float x, float y);
	long double copysignl(long double x, long double y);
	double cos(double x);
	float cosf(float x);
	double cosh(double x);
	float coshf(float x);
	long double coshl(long double x);
	long double cosl(long double x);
	double erf(double x);
	double erfc(double x);
	float erfcf(float x);
	long double erfcl(long double x);
	float erff(float x);
	long double erfl(long double x);
	double exp(double x);
	double exp2(double x);
	float exp2f(float x);
	long double exp2l(long double x);
	float expf(float x);
	long double expl(long double x);
	double expm1(double x);
	float expm1f(float x);
	long double expm1l(long double x);
	double fabs(double x);
	float fabsf(float x);
	long double fabsl(long double x);
	double fdim(double x, double y);
	float fdimf(float x, float y);
	long double fdiml(long double x, long double y);
	double floor(double x);
	float floorf(float x);
	long double floorl(long double x);
	double fma(double x, double y, double z);
	float fmaf(float x, float y, float z);
	long double fmal(long double x, long double y, long double z);
	double fmax(double x, double y);
	float fmaxf(float x, float y);
	long double fmaxl(long double x, long double y);
	double fmin(double x, double y);
	float fminf(float x, float y);
	long double fminl(long double x, long double y);
	double fmod(double x, double y);
	float fmodf(float x, float y);
	long double fmodl(long double x, long double y);
	double frexp(double num, int *exp);
	float frexpf(float num, int *exp);
	long double frexpl(long double num, int *exp);
	double hypot(double x, double y);
	float hypotf(float x, float y);
	long double hypotl(long double x, long double y);
	int ilogb(double x);
	int ilogbf(float x);
	int ilogbl(long double x);
	double j0(double x);
	double j1(double x);
	double jn(int n, double x);
	double ldexp(double x, int exp);
	float ldexpf(float x, int exp);
	long double ldexpl(long double x, int exp);
	double lgamma(double x);
	float lgammaf(float x);
	long double lgammal(long double x);
	long long llrint(double x);
	long long llrintf(float x);
	long long llrintl(long double x);
	long long llround(double x);
	long long llroundf(float x);
	long long llroundl(long double x);
	double log(double x);
	double log10(double x);
	float log10f(float x);
	long double log10l(long double x);
	double log1p(double x);
	float log1pf(float x);
	long double log1pl(long double x);
	double log2(double x);
	float log2f(float x);
	long double log2l(long double x);
	double logb(double x);
	float logbf(float x);
	long double logbl(long double x);
	float logf(float x);
	long double logl(long double x);
	long lrint(double x);
	long lrintf(float x);
	long lrintl(long double x);
	long lround(double x);
	long lroundf(float x);
	long lroundl(long double x);
	double modf(double x, double *iptr);
	float modff(float value, float *iptr);
	long double modfl(long double x, long double *iptr);
	double nan(const char *tagp);
	float nanf(const char *tagp);
	long double nanl(const char *tagp);
	double nearbyint(double x);
	float nearbyintf(float x);
	long double nearbyintl(long double x);
	double nextafter(double x, double y);
	float nextafterf(float x, float y);
	long double nextafterl(long double x, long double y);
	double nexttoward(double x, long double y);
	float nexttowardf(float x, long double y);
	long double nexttowardl(long double x, long double y);
	double pow(double x, double y);
	float powf(float x, float y);
	long double powl(long double x, long double y);
	double remainder(double x, double y);
	float remainderf(float x, float y);
	long double remainderl(long double x, long double y);
	double remquo(double x, double y, int *quo);
	float remquof(float x, float y, int *quo);
	long double remquol(long double x, long double y, int *quo);
	double rint(double x);
	float rintf(float x);
	long double rintl(long double x);
	double round(double x);
	float roundf(float x);
	long double roundl(long double x);
	double scalb(double x, double n);
	double scalbln(double x, long n);
	float scalblnf(float x, long n);
	long double scalblnl(long double x, long n);
	double scalbn(double x, int n);
	float scalbnf(float x, int n);
	long double scalbnl(long double x, int n);
	double sin(double x);
	float sinf(float x);
	double sinh(double x);
	float sinhf(float x);
	long double sinhl(long double x);
	long double sinl(long double x);
	double sqrt(double x);
	float sqrtf(float x);
	long double sqrtl(long double x);
	double tan(double x);
	float tanf(float x);
	double tanh(double x);
	float tanhf(float x);
	long double tanhl(long double x);
	long double tanl(long double x);
	double tgamma(double x);
	float tgammaf(float x);
	long double tgammal(long double x);
	double trunc(double x);
	float truncf(float x);
	long double truncl(long double x);
	double y0(double x);
	double y1(double x);
	double yn(int n, double x);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_MATH_H
