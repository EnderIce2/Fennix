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

#include <math.h>
#include <sys/types.h>
#include <float.h>
#include <errno.h>

export int signgam;

export double acos(double x);
export float acosf(float x);
export double acosh(double x);
export float acoshf(float x);
export long double acoshl(long double x);
export long double acosl(long double x);
export double asin(double x);
export float asinf(float x);
export double asinh(double x);
export float asinhf(float x);
export long double asinhl(long double x);
export long double asinl(long double x);
export double atan(double x);
export double atan2(double y, double x);
export float atan2f(float y, float x);
export long double atan2l(long double y, long double x);
export float atanf(float x);
export double atanh(double x);
export float atanhf(float x);
export long double atanhl(long double x);
export long double atanl(long double x);
export double cbrt(double x);
export float cbrtf(float x);
export long double cbrtl(long double x);
export double ceil(double x);
export float ceilf(float x);
export long double ceill(long double x);
export double copysign(double x, double y);
export float copysignf(float x, float y);
export long double copysignl(long double x, long double y);
export double cos(double x);
export float cosf(float x);
export double cosh(double x);
export float coshf(float x);
export long double coshl(long double x);
export long double cosl(long double x);
export double erf(double x);
export double erfc(double x);
export float erfcf(float x);
export long double erfcl(long double x);
export float erff(float x);
export long double erfl(long double x);
export double exp(double x);
export double exp2(double x);
export float exp2f(float x);
export long double exp2l(long double x);
export float expf(float x);
export long double expl(long double x);
export double expm1(double x);
export float expm1f(float x);
export long double expm1l(long double x);
export double fabs(double x);
export float fabsf(float x);
export long double fabsl(long double x);
export double fdim(double x, double y);
export float fdimf(float x, float y);
export long double fdiml(long double x, long double y);
export double floor(double x);
export float floorf(float x);
export long double floorl(long double x);
export double fma(double x, double y, double z);
export float fmaf(float x, float y, float z);
export long double fmal(long double x, long double y, long double z);
export double fmax(double x, double y);
export float fmaxf(float x, float y);
export long double fmaxl(long double x, long double y);
export double fmin(double x, double y);
export float fminf(float x, float y);
export long double fminl(long double x, long double y);
export double fmod(double x, double y);
export float fmodf(float x, float y);
export long double fmodl(long double x, long double y);
export double frexp(double num, int *exp);
export float frexpf(float num, int *exp);
export long double frexpl(long double num, int *exp);
export double hypot(double x, double y);
export float hypotf(float x, float y);
export long double hypotl(long double x, long double y);
export int ilogb(double x);
export int ilogbf(float x);
export int ilogbl(long double x);
export double j0(double x);
export double j1(double x);
export double jn(int n, double x);
export double ldexp(double x, int exp);
export float ldexpf(float x, int exp);
export long double ldexpl(long double x, int exp);
export double lgamma(double x);
export float lgammaf(float x);
export long double lgammal(long double x);
export long long llrint(double x);
export long long llrintf(float x);
export long long llrintl(long double x);
export long long llround(double x);
export long long llroundf(float x);
export long long llroundl(long double x);
export double log(double x);
export double log10(double x);
export float log10f(float x);
export long double log10l(long double x);
export double log1p(double x);
export float log1pf(float x);
export long double log1pl(long double x);
export double log2(double x);
export float log2f(float x);
export long double log2l(long double x);
export double logb(double x);
export float logbf(float x);
export long double logbl(long double x);
export float logf(float x);
export long double logl(long double x);
export long lrint(double x);
export long lrintf(float x);
export long lrintl(long double x);
export long lround(double x);
export long lroundf(float x);
export long lroundl(long double x);
export double modf(double x, double *iptr);
export float modff(float value, float *iptr);
export long double modfl(long double x, long double *iptr);
export double nan(const char *tagp);
export float nanf(const char *tagp);
export long double nanl(const char *tagp);
export double nearbyint(double x);
export float nearbyintf(float x);
export long double nearbyintl(long double x);
export double nextafter(double x, double y);
export float nextafterf(float x, float y);
export long double nextafterl(long double x, long double y);
export double nexttoward(double x, long double y);
export float nexttowardf(float x, long double y);
export long double nexttowardl(long double x, long double y);
export double pow(double x, double y);
export float powf(float x, float y);
export long double powl(long double x, long double y);
export double remainder(double x, double y);
export float remainderf(float x, float y);
export long double remainderl(long double x, long double y);
export double remquo(double x, double y, int *quo);
export float remquof(float x, float y, int *quo);
export long double remquol(long double x, long double y, int *quo);
export double rint(double x);
export float rintf(float x);
export long double rintl(long double x);
export double round(double x);
export float roundf(float x);
export long double roundl(long double x);
export double scalb(double x, double y);
export double scalbln(double x, long n);
export float scalblnf(float x, long n);
export long double scalblnl(long double x, long n);
export double scalbn(double x, int n);
export float scalbnf(float x, int n);
export long double scalbnl(long double x, int n);
export double sin(double x);
export float sinf(float x);
export double sinh(double x);
export float sinhf(float x);
export long double sinhl(long double x);
export long double sinl(long double x);
export double sqrt(double x);
export float sqrtf(float x);
export long double sqrtl(long double x);
export double tan(double x);
export float tanf(float x);
export double tanh(double x);
export float tanhf(float x);
export long double tanhl(long double x);
export long double tanl(long double x);
export double tgamma(double x);
export float tgammaf(float x);
export long double tgammal(long double x);
export double trunc(double x);
export float truncf(float x);
export long double truncl(long double x);
export double y0(double x);
export double y1(double x);
export double yn(int n, double x);
