/*
	This file is part of Fennix Userspace.

	Fennix Userspace is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Userspace is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Userspace. If not, see <https://www.gnu.org/licenses/>.
*/

#include <math.h>
#include <stddef.h>
#include <stdio.h>

int test_acos()
{
	double result = acos(0.5);
	double expected_result = 1.0471975511965976;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_acosf()
{
	float result = acosf(0.5f);
	float expected_result = 1.0471975f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_acosh()
{
	double result = acosh(1.5);
	double expected_result = 0.9624236501192069;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_acoshf()
{
	float result = acoshf(1.5f);
	float expected_result = 0.9624237f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_acoshl()
{
	long double result = acoshl(1.5L);
	long double expected_result = 0.9624236501192069L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_acosl()
{
	long double result = acosl(0.5L);
	long double expected_result = 1.0471975511965976L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_asin()
{
	double result = asin(0.5);
	double expected_result = 0.5235987755982989;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_asinf()
{
	float result = asinf(0.5f);
	float expected_result = 0.5235988f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_asinh()
{
	double result = asinh(0.5);
	double expected_result = 0.48121182505960347;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_asinhf()
{
	float result = asinhf(0.5f);
	float expected_result = 0.4812118f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_asinhl()
{
	long double result = asinhl(0.5L);
	long double expected_result = 0.48121182505960347L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_asinl()
{
	long double result = asinl(0.5L);
	long double expected_result = 0.5235987755982989L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_atan()
{
	double result = atan(1.0);
	double expected_result = 0.7853981633974483;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_atan2()
{
	double result = atan2(1.0, 1.0);
	double expected_result = 0.7853981633974483;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_atan2f()
{
	float result = atan2f(1.0f, 1.0f);
	float expected_result = 0.7853982f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_atan2l()
{
	long double result = atan2l(1.0L, 1.0L);
	long double expected_result = 0.7853981633974483L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_atanf()
{
	float result = atanf(1.0f);
	float expected_result = 0.7853982f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_atanh()
{
	double result = atanh(0.5);
	double expected_result = 0.5493061443340548;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_atanhf()
{
	float result = atanhf(0.5f);
	float expected_result = 0.5493061f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_atanhl()
{
	long double result = atanhl(0.5L);
	long double expected_result = 0.5493061443340548L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_atanl()
{
	long double result = atanl(1.0L);
	long double expected_result = 0.7853981633974483L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_cbrt()
{
	double result = cbrt(27.0);
	double expected_result = 3.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_cbrtf()
{
	float result = cbrtf(27.0f);
	float expected_result = 3.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_cbrtl()
{
	long double result = cbrtl(27.0L);
	long double expected_result = 3.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_ceil()
{
	double result = ceil(2.3);
	double expected_result = 3.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_ceilf()
{
	float result = ceilf(2.3f);
	float expected_result = 3.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_ceill()
{
	long double result = ceill(2.3L);
	long double expected_result = 3.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_copysign()
{
	double result = copysign(1.0, -1.0);
	double expected_result = -1.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_copysignf()
{
	float result = copysignf(1.0f, -1.0f);
	float expected_result = -1.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_copysignl()
{
	long double result = copysignl(1.0L, -1.0L);
	long double expected_result = -1.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_cos()
{
	double result = cos(0.0);
	double expected_result = 1.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_cosf()
{
	float result = cosf(0.0f);
	float expected_result = 1.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_cosh()
{
	double result = cosh(0.0);
	double expected_result = 1.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_coshf()
{
	float result = coshf(0.0f);
	float expected_result = 1.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_coshl()
{
	long double result = coshl(0.0L);
	long double expected_result = 1.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_cosl()
{
	long double result = cosl(0.0L);
	long double expected_result = 1.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_erf()
{
	double result = erf(1.0);
	double expected_result = 0.8427007929497149;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_erfc()
{
	double result = erfc(1.0);
	double expected_result = 0.1572992070502851;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_erfcf()
{
	float result = erfcf(1.0f);
	float expected_result = 0.1572992f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_erfcl()
{
	long double result = erfcl(1.0L);
	long double expected_result = 0.1572992070502851L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_erff()
{
	float result = erff(1.0f);
	float expected_result = 0.8427008f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_erfl()
{
	long double result = erfl(1.0L);
	long double expected_result = 0.8427007929497149L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_exp()
{
	double result = exp(1.0);
	double expected_result = 2.718281828459045;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_exp2()
{
	double result = exp2(3.0);
	double expected_result = 8.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_exp2f()
{
	float result = exp2f(3.0f);
	float expected_result = 8.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_exp2l()
{
	long double result = exp2l(3.0L);
	long double expected_result = 8.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_expf()
{
	float result = expf(1.0f);
	float expected_result = 2.7182817f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_expl()
{
	long double result = expl(1.0L);
	long double expected_result = 2.718281828459045L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_expm1()
{
	double result = expm1(1.0);
	double expected_result = 1.718281828459045;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_expm1f()
{
	float result = expm1f(1.0f);
	float expected_result = 1.7182817f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_expm1l()
{
	long double result = expm1l(1.0L);
	long double expected_result = 1.718281828459045L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_fabs()
{
	double result = fabs(-5.0);
	double expected_result = 5.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_fabsf()
{
	float result = fabsf(-5.0f);
	float expected_result = 5.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_fabsl()
{
	long double result = fabsl(-5.0L);
	long double expected_result = 5.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_fdim()
{
	double result = fdim(5.0, 3.0);
	double expected_result = 2.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_fdimf()
{
	float result = fdimf(5.0f, 3.0f);
	float expected_result = 2.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_fdiml()
{
	long double result = fdiml(5.0L, 3.0L);
	long double expected_result = 2.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_floor()
{
	double result = floor(2.7);
	double expected_result = 2.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_floorf()
{
	float result = floorf(2.7f);
	float expected_result = 2.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_floorl()
{
	long double result = floorl(2.7L);
	long double expected_result = 2.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_fma()
{
	double result = fma(2.0, 3.0, 1.0);
	double expected_result = 7.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_fmaf()
{
	float result = fmaf(2.0f, 3.0f, 1.0f);
	float expected_result = 7.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_fmal()
{
	long double result = fmal(2.0L, 3.0L, 1.0L);
	long double expected_result = 7.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_fmax()
{
	double result = fmax(3.0, 5.0);
	double expected_result = 5.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_fmaxf()
{
	float result = fmaxf(3.0f, 5.0f);
	float expected_result = 5.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_fmaxl()
{
	long double result = fmaxl(3.0L, 5.0L);
	long double expected_result = 5.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_fmin()
{
	double result = fmin(3.0, 5.0);
	double expected_result = 3.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_fminf()
{
	float result = fminf(3.0f, 5.0f);
	float expected_result = 3.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_fminl()
{
	long double result = fminl(3.0L, 5.0L);
	long double expected_result = 3.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_fmod()
{
	double result = fmod(5.0, 2.0);
	double expected_result = 1.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_fmodf()
{
	float result = fmodf(5.0f, 2.0f);
	float expected_result = 1.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_fmodl()
{
	long double result = fmodl(5.0L, 2.0L);
	long double expected_result = 1.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_frexp()
{
	int exponent;
	double result = frexp(5.0, &exponent);
	double expected_result = 0.625;
	int expected_exponent = 3;

	if (fabs(result - expected_result) < 1e-6 && exponent == expected_exponent)
		return 0;
	else
		return 1;
}

int test_frexpf()
{
	int exponent;
	float result = frexpf(5.0f, &exponent);
	float expected_result = 0.625f;
	int expected_exponent = 3;

	if (fabs(result - expected_result) < 1e-6f && exponent == expected_exponent)
		return 0;
	else
		return 1;
}

int test_frexpl()
{
	int exponent;
	long double result = frexpl(5.0L, &exponent);
	long double expected_result = 0.625L;
	int expected_exponent = 3;

	if (fabsl(result - expected_result) < 1e-6L && exponent == expected_exponent)
		return 0;
	else
		return 1;
}

int test_hypot()
{
	double result = hypot(3.0, 4.0);
	double expected_result = 5.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_hypotf()
{
	float result = hypotf(3.0f, 4.0f);
	float expected_result = 5.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_hypotl()
{
	long double result = hypotl(3.0L, 4.0L);
	long double expected_result = 5.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_ilogb()
{
	double result = ilogb(32.0);
	int expected_result = 5;
	return result == expected_result ? 0 : 1;
}

int test_ilogbf()
{
	float result = ilogbf(32.0f);
	int expected_result = 5;
	return result == expected_result ? 0 : 1;
}

int test_ilogbl()
{
	long double result = ilogbl(32.0L);
	int expected_result = 5;
	return result == expected_result ? 0 : 1;
}

int test_j0()
{
	double result = j0(1.0);
	double expected_result = 0.765197686551;
	return fabs(result - expected_result) < 1e-0 ? 0 : 1;
}

int test_j1()
{
	double result = j1(1.0);
	double expected_result = 0.440050585744;
	return fabs(result - expected_result) < 1e-0 ? 0 : 1;
}

int test_jn()
{
	double result = jn(2, 1.0);
	double expected_result = -0.363268;
	return fabs(result - expected_result) < 1e-0 ? 0 : 1;
}

int test_ldexp()
{
	double result = ldexp(1.0, 2);
	double expected_result = 4.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_ldexpf()
{
	float result = ldexpf(1.0f, 2);
	float expected_result = 4.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_ldexpl()
{
	long double result = ldexpl(1.0L, 2);
	long double expected_result = 4.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_lgamma()
{
	double result = lgamma(1.0);
	double expected_result = 0.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_lgammaf()
{
	float result = lgammaf(1.0f);
	float expected_result = 0.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_lgammal()
{
	long double result = lgammal(1.0L);
	long double expected_result = 0.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_llrint()
{
	long long result = llrint(3.6);
	long long expected_result = 4;
	return result == expected_result ? 0 : 1;
}

int test_llrintf()
{
	long long result = llrintf(3.6f);
	long long expected_result = 4;
	return result == expected_result ? 0 : 1;
}

int test_llrintl()
{
	long long result = llrintl(3.6L);
	long long expected_result = 4;
	return result == expected_result ? 0 : 1;
}

int test_llround()
{
	double result = llround(2.5);
	long long expected_result = 3;
	return result == expected_result ? 0 : 1;
}

int test_llroundf()
{
	float result = llroundf(2.5f);
	long long expected_result = 3;
	return result == expected_result ? 0 : 1;
}

int test_llroundl()
{
	long double result = llroundl(2.5L);
	long long expected_result = 3;
	return result == expected_result ? 0 : 1;
}

int test_log()
{
	double result = log(1.0);
	double expected_result = 0.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_log10()
{
	double result = log10(100.0);
	double expected_result = 2.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_log10f()
{
	float result = log10f(100.0f);
	float expected_result = 2.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_log10l()
{
	long double result = log10l(100.0L);
	long double expected_result = 2.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_log1p()
{
	double result = log1p(1.0);
	double expected_result = 0.6931471805599453;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_log1pf()
{
	float result = log1pf(1.0f);
	float expected_result = 0.6931472f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_log1pl()
{
	long double result = log1pl(1.0L);
	long double expected_result = 0.6931471805599453L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_log2()
{
	double result = log2(8.0);
	double expected_result = 3.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_log2f()
{
	float result = log2f(8.0f);
	float expected_result = 3.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_log2l()
{
	long double result = log2l(8.0L);
	long double expected_result = 3.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_logb()
{
	double result = logb(8.0);
	double expected_result = 3.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_logbf()
{
	float result = logbf(8.0f);
	float expected_result = 3.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_logbl()
{
	long double result = logbl(8.0L);
	long double expected_result = 3.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_logf()
{
	float result = logf(1.0f);
	float expected_result = 0.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_logl()
{
	long double result = logl(1.0L);
	long double expected_result = 0.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_lrint()
{
	long result = lrint(2.5);
	long expected_result = 2;
	return result == expected_result ? 0 : 1;
}

int test_lrintf()
{
	long result = lrintf(2.5f);
	long expected_result = 2;
	return result == expected_result ? 0 : 1;
}

int test_lrintl()
{
	long result = lrintl(2.5L);
	long expected_result = 2;
	return result == expected_result ? 0 : 1;
}

int test_lround()
{
	double result = lround(2.5);
	long expected_result = 3;
	return result == expected_result ? 0 : 1;
}

int test_lroundf()
{
	float result = lroundf(2.5f);
	long expected_result = 3;
	return result == expected_result ? 0 : 1;
}

int test_lroundl()
{
	long double result = lroundl(2.5L);
	long expected_result = 3;
	return result == expected_result ? 0 : 1;
}

int test_modf()
{
	double integer_part;
	double result = modf(3.14, &integer_part);
	double expected_result = 0.14;
	double expected_integer_part = 3.0;

	if (fabs(result - expected_result) < 1e-6 && fabs(integer_part - expected_integer_part) < 1e-6)
		return 0;
	else
		return 1;
}

int test_modff()
{
	float integer_part;
	float result = modff(3.14f, &integer_part);
	float expected_result = 0.14f;
	float expected_integer_part = 3.0f;

	if (fabs(result - expected_result) < 1e-6f && fabs(integer_part - expected_integer_part) < 1e-6f)
		return 0;
	else
		return 1;
}

int test_modfl()
{
	long double integer_part;
	long double result = modfl(3.14L, &integer_part);
	long double expected_result = 0.14L;
	long double expected_integer_part = 3.0L;

	if (fabsl(result - expected_result) < 1e-6L && fabsl(integer_part - expected_integer_part) < 1e-6L)
		return 0;
	else
		return 1;
}

int test_nan()
{
	double result = nan("");
	return isnan(result) ? 0 : 1;
}

int test_nanf()
{
	float result = nanf("");
	return isnan(result) ? 0 : 1;
}

int test_nanl()
{
	long double result = nanl("");
	return isnan(result) ? 0 : 1;
}

int test_nearbyint()
{
	double result = nearbyint(3.6);
	double expected_result = 4.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_nearbyintf()
{
	float result = nearbyintf(3.6f);
	float expected_result = 4.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_nearbyintl()
{
	long double result = nearbyintl(3.6L);
	long double expected_result = 4.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_nextafter()
{
	double result = nextafter(1.0, 2.0);
	double expected_result = 1.0000000000000002;
	return fabs(result - expected_result) < 1e-16 ? 0 : 1;
}

int test_nextafterf()
{
	float result = nextafterf(1.0f, 2.0f);
	float expected_result = 1.0000001192092896f;
	return fabs(result - expected_result) < 1e-7f ? 0 : 1;
}

int test_nextafterl()
{
	long double result = nextafterl(1.0L, 2.0L);
	long double expected_result = 1.0000000000000002L;
	return fabsl(result - expected_result) < 1e-15L ? 0 : 1;
}

int test_nexttoward()
{
	double result = nexttoward(1.0, 2.0);
	double expected_result = 1.0000000000000002;
	return fabs(result - expected_result) < 1e-16 ? 0 : 1;
}

int test_nexttowardf()
{
	float result = nexttowardf(1.0f, 2.0f);
	float expected_result = 1.0000001192092896f;
	return fabs(result - expected_result) < 1e-7f ? 0 : 1;
}

int test_nexttowardl()
{
	long double result = nexttowardl(1.0L, 2.0L);
	long double expected_result = 1.0000000000000002L;
	return fabsl(result - expected_result) < 1e-1L ? 0 : 1;
}

int test_pow()
{
	double result = pow(2.0, 3.0);
	double expected_result = 8.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_powf()
{
	float result = powf(2.0f, 3.0f);
	float expected_result = 8.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_powl()
{
	long double result = powl(2.0L, 3.0L);
	long double expected_result = 8.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_remainder()
{
	double result = remainder(5.3, 2.0);
	double expected_result = -0.700000;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_remainderf()
{
	float result = remainderf(5.3f, 2.0f);
	float expected_result = -0.700000f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_remainderl()
{
	long double result = remainderl(5.3L, 2.0L);
	long double expected_result = -0.69999999999999999983;
	return fabs(result - expected_result) < 1e-1 ? 0 : 1;
}

int test_remquo()
{
	int quotient;
	double result = remquo(5.3, 2.0, &quotient);
	double expected_result = -0.700000;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_remquof()
{
	int quotient;
	float result = remquof(5.3f, 2.0f, &quotient);
	float expected_result = -0.700000;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_remquol()
{
	int quotient;
	long double result = remquol(5.3L, 2.0L, &quotient);
	long double expected_result = -0.69999999999999999983;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_rint()
{
	double result = rint(2.5);
	double expected_result = 2.000000;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_rintf()
{
	float result = rintf(2.5f);
	float expected_result = 2.000000f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_rintl()
{
	long double result = rintl(2.5L);
	long double expected_result = 2.00;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_round()
{
	double result = round(2.5);
	double expected_result = 3.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_roundf()
{
	float result = roundf(2.5f);
	float expected_result = 3.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_roundl()
{
	long double result = roundl(2.5L);
	long double expected_result = 3.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_scalb()
{
	double result = scalb(1.0, 2);
	double expected_result = -64.0;
	return fabs(result - expected_result) < 1e-16 ? 0 : 1;
}

int test_scalbln()
{
	double result = scalbln(1.0, 2);
	double expected_result = 4.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_scalblnf()
{
	float result = scalblnf(1.0f, 2);
	float expected_result = 4.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_scalblnl()
{
	long double result = scalblnl(1.0L, 2);
	long double expected_result = 4.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_scalbn()
{
	double result = scalbn(1.0, 2);
	double expected_result = 4.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_scalbnf()
{
	float result = scalbnf(1.0f, 2);
	float expected_result = 4.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_scalbnl()
{
	long double result = scalbnl(1.0L, 2);
	long double expected_result = 4.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_sin()
{
	double result = sin(0.0);
	double expected_result = 0.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_sinf()
{
	float result = sinf(0.0f);
	float expected_result = 0.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_sinh()
{
	double result = sinh(0.0);
	double expected_result = 0.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_sinhf()
{
	float result = sinhf(0.0f);
	float expected_result = 0.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_sinhl()
{
	long double result = sinhl(0.0L);
	long double expected_result = 0.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_sinl()
{
	long double result = sinl(0.0L);
	long double expected_result = 0.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_sqrt()
{
	double result = sqrt(16.0);
	double expected_result = 4.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_sqrtf()
{
	float result = sqrtf(16.0f);
	float expected_result = 4.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_sqrtl()
{
	long double result = sqrtl(16.0L);
	long double expected_result = 4.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_tan()
{
	double result = tan(0.0);
	double expected_result = 0.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_tanf()
{
	float result = tanf(0.0f);
	float expected_result = 0.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_tanh()
{
	double result = tanh(0.0);
	double expected_result = 0.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_tanhf()
{
	float result = tanhf(0.0f);
	float expected_result = 0.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_tanhl()
{
	long double result = tanhl(0.0L);
	long double expected_result = 0.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_tanl()
{
	long double result = tanl(0.0L);
	long double expected_result = 0.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_tgamma()
{
	double result = tgamma(4.0);
	double expected_result = 6.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_tgammaf()
{
	float result = tgammaf(4.0f);
	float expected_result = 6.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_tgammal()
{
	long double result = tgammal(4.0L);
	long double expected_result = 6.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_trunc()
{
	double result = trunc(2.9);
	double expected_result = 2.0;
	return fabs(result - expected_result) < 1e-6 ? 0 : 1;
}

int test_truncf()
{
	float result = truncf(2.9f);
	float expected_result = 2.0f;
	return fabs(result - expected_result) < 1e-6f ? 0 : 1;
}

int test_truncl()
{
	long double result = truncl(2.9L);
	long double expected_result = 2.0L;
	return fabsl(result - expected_result) < 1e-6L ? 0 : 1;
}

int test_y0()
{
	double result = y0(1.0);
	double expected_result = 0.08825696421567697;
	return fabs(result - expected_result) < 1e-1 ? 0 : 1;
}

int test_y1()
{
	double result = y1(1.0);
	double expected_result = 0.000000;
	return fabs(result - expected_result) < 1e-1 ? 0 : 1;
}

int test_yn()
{
	double result = yn(1, 1.0);
	double expected_result = 0.000000;
	return fabs(result - expected_result) < 1e-1 ? 0 : 1;
}
