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
export float atanf(float);
export double atanh(double);
export float atanhf(float);
export long double atanhl(long double);
export long double atanl(long double);
export double cbrt(double);
export float cbrtf(float);
export long double cbrtl(long double);
export double ceil(double);
export float ceilf(float);
export long double ceill(long double);
export double copysign(double, double);
export float copysignf(float, float);
export long double copysignl(long double, long double);
export double cos(double x);
export float cosf(float x);
export double cosh(double x);
export float coshf(float x);
export long double coshl(long double x);
export long double cosl(long double x);
export double erf(double);
export double erfc(double);
export float erfcf(float);
export long double erfcl(long double);
export float erff(float);
export long double erfl(long double);
export double exp(double);
export double exp2(double);
export float exp2f(float);
export long double exp2l(long double);
export float expf(float);
export long double expl(long double);
export double expm1(double);
export float expm1f(float);
export long double expm1l(long double);
export double fabs(double);
export float fabsf(float);
export long double fabsl(long double);
export double fdim(double, double);
export float fdimf(float, float);
export long double fdiml(long double, long double);
export double floor(double);
export float floorf(float);
export long double floorl(long double);
export double fma(double, double, double);
export float fmaf(float, float, float);
export long double fmal(long double, long double, long double);
export double fmax(double, double);
export float fmaxf(float, float);
export long double fmaxl(long double, long double);
export double fmin(double, double);
export float fminf(float, float);
export long double fminl(long double, long double);
export double fmod(double, double);
export float fmodf(float, float);
export long double fmodl(long double, long double);
export double frexp(double, int *);
export float frexpf(float value, int *);
export long double frexpl(long double value, int *);
export double hypot(double, double);
export float hypotf(float, float);
export long double hypotl(long double, long double);
export int ilogb(double);
export int ilogbf(float);
export int ilogbl(long double);
export double j0(double);
export double j1(double);
export double jn(int, double);
export double ldexp(double, int);
export float ldexpf(float, int);
export long double ldexpl(long double, int);
export double lgamma(double);
export float lgammaf(float);
export long double lgammal(long double);
export long long llrint(double);
export long long llrintf(float);
export long long llrintl(long double);
export long long llround(double);
export long long llroundf(float);
export long long llroundl(long double);
export double log(double);
export double log10(double);
export float log10f(float);
export long double log10l(long double);
export double log1p(double);
export float log1pf(float);
export long double log1pl(long double);
export double log2(double);
export float log2f(float);
export long double log2l(long double);
export double logb(double);
export float logbf(float);
export long double logbl(long double);
export float logf(float);
export long double logl(long double);
export long lrint(double);
export long lrintf(float);
export long lrintl(long double);
export long lround(double);
export long lroundf(float);
export long lroundl(long double);
export double modf(double, double *);
export float modff(float, float *);
export long double modfl(long double, long double *);
export double nan(const char *);
export float nanf(const char *);
export long double nanl(const char *);
export double nearbyint(double);
export float nearbyintf(float);
export long double nearbyintl(long double);
export double nextafter(double, double);
export float nextafterf(float, float);
export long double nextafterl(long double, long double);
export double nexttoward(double, long double);
export float nexttowardf(float, long double);
export long double nexttowardl(long double, long double);
export double pow(double, double);
export float powf(float, float);
export long double powl(long double, long double);
export double remainder(double, double);
export float remainderf(float, float);
export long double remainderl(long double, long double);
export double remquo(double, double, int *);
export float remquof(float, float, int *);
export long double remquol(long double, long double, int *);
export double rint(double);
export float rintf(float);
export long double rintl(long double);
export double round(double);
export float roundf(float);
export long double roundl(long double);
export double scalb(double, double);
export double scalbln(double, long);
export float scalblnf(float, long);
export long double scalblnl(long double, long);
export double scalbn(double, int);
export float scalbnf(float, int);
export long double scalbnl(long double, int);
export double sin(double x);
export float sinf(float x);
export double sinh(double x);
export float sinhf(float x);
export long double sinhl(long double x);
export long double sinl(long double x);
export double sqrt(double);
export float sqrtf(float);
export long double sqrtl(long double);
export double tan(double);
export float tanf(float);
export double tanh(double);
export float tanhf(float);
export long double tanhl(long double);
export long double tanl(long double);
export double tgamma(double);
export float tgammaf(float);
export long double tgammal(long double);
export double trunc(double);
export float truncf(float);
export long double truncl(long double);
export double y0(double);
export double y1(double);
export double yn(int, double);
