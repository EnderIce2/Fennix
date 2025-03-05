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
#include <inttypes.h>
#include <fenv.h>

export int signgam;

export double acos(double x)
{
	if (x < -1.0 || x > 1.0)
	{
		errno = EDOM;
		return NAN;
	}
	return atan2(sqrt(1.0 - x * x), x);
}

export float acosf(float x)
{
	if (x < -1.0f || x > 1.0f)
	{
		errno = EDOM;
		return NAN;
	}
	return atan2f(sqrtf(1.0f - x * x), x);
}

export double acosh(double x)
{
	if (x < 1.0)
	{
		errno = EDOM;
		return NAN;
	}
	return log(x + sqrt(x * x - 1.0));
}

export float acoshf(float x)
{
	if (x < 1.0f)
	{
		errno = EDOM;
		return NAN;
	}
	return logf(x + sqrtf(x * x - 1.0f));
}

export long double acoshl(long double x)
{
	if (x < 1.0L)
	{
		errno = EDOM;
		return NAN;
	}
	return logl(x + sqrtl(x * x - 1.0L));
}

export long double acosl(long double x)
{
	if (x < -1.0L || x > 1.0L)
	{
		errno = EDOM;
		return NAN;
	}
	return atan2l(sqrtl(1.0L - x * x), x);
}

export double asin(double x)
{
	if (x < -1.0 || x > 1.0)
	{
		errno = EDOM;
		return NAN;
	}
	return atan2(x, sqrt(1.0 - x * x));
}

export float asinf(float x)
{
	if (x < -1.0f || x > 1.0f)
	{
		errno = EDOM;
		return NAN;
	}
	return atan2f(x, sqrtf(1.0f - x * x));
}

export double asinh(double x)
{
	return log(x + sqrt(x * x + 1.0));
}

export float asinhf(float x)
{
	return logf(x + sqrtf(x * x + 1.0f));
}

export long double asinhl(long double x)
{
	return logl(x + sqrtl(x * x + 1.0L));
}

export long double asinl(long double x)
{
	if (x < -1.0L || x > 1.0L)
	{
		errno = EDOM;
		return NAN;
	}
	return atan2l(x, sqrtl(1.0L - x * x));
}

export double atan(double x)
{
	return atan2(x, 1.0);
}

export double atan2(double y, double x)
{
	if (x > 0.0)
	{
		return atan(y / x);
	}
	else if (x < 0.0 && y >= 0.0)
	{
		return atan(y / x) + M_PI;
	}
	else if (x < 0.0 && y < 0.0)
	{
		return atan(y / x) - M_PI;
	}
	else if (x == 0.0 && y > 0.0)
	{
		return M_PI_2;
	}
	else if (x == 0.0 && y < 0.0)
	{
		return -M_PI_2;
	}
	else
	{
		errno = EDOM;
		return NAN;
	}
}

export float atan2f(float y, float x)
{
	if (x > 0.0f)
	{
		return atanf(y / x);
	}
	else if (x < 0.0f && y >= 0.0f)
	{
		return atanf(y / x) + (float)M_PI;
	}
	else if (x < 0.0f && y < 0.0f)
	{
		return atanf(y / x) - (float)M_PI;
	}
	else if (x == 0.0f && y > 0.0f)
	{
		return (float)M_PI_2;
	}
	else if (x == 0.0f && y < 0.0f)
	{
		return -(float)M_PI_2;
	}
	else
	{
		errno = EDOM;
		return NAN;
	}
}

export long double atan2l(long double y, long double x)
{
	if (x > 0.0L)
	{
		return atanl(y / x);
	}
	else if (x < 0.0L && y >= 0.0L)
	{
		return atanl(y / x) + M_PIl;
	}
	else if (x < 0.0L && y < 0.0L)
	{
		return atanl(y / x) - M_PIl;
	}
	else if (x == 0.0L && y > 0.0L)
	{
		return M_PI_2l;
	}
	else if (x == 0.0L && y < 0.0L)
	{
		return -M_PI_2l;
	}
	else
	{
		errno = EDOM;
		return NAN;
	}
}

export float atanf(float x)
{
	return atan2f(x, 1.0f);
}

export double atanh(double x)
{
	if (x <= -1.0 || x >= 1.0)
	{
		errno = EDOM;
		return NAN;
	}
	return 0.5 * log((1.0 + x) / (1.0 - x));
}

export float atanhf(float x)
{
	if (x <= -1.0f || x >= 1.0f)
	{
		errno = EDOM;
		return NAN;
	}
	return 0.5f * logf((1.0f + x) / (1.0f - x));
}

export long double atanhl(long double x)
{
	if (x <= -1.0L || x >= 1.0L)
	{
		errno = EDOM;
		return NAN;
	}
	return 0.5L * logl((1.0L + x) / (1.0L - x));
}

export long double atanl(long double x)
{
	return atan2l(x, 1.0L);
}

export double cbrt(double x)
{
	return pow(x, 1.0 / 3.0);
}

export float cbrtf(float x)
{
	return powf(x, 1.0f / 3.0f);
}

export long double cbrtl(long double x)
{
	return powl(x, 1.0L / 3.0L);
}

export double ceil(double x)
{
	if (x == (double)((long)x))
	{
		return x;
	}
	return (x > 0.0) ? (double)((long)x + 1) : (double)((long)x);
}

export float ceilf(float x)
{
	if (x == (float)((long)x))
	{
		return x;
	}
	return (x > 0.0f) ? (float)((long)x + 1) : (float)((long)x);
}

export long double ceill(long double x)
{
	if (x == (long double)((long)x))
	{
		return x;
	}
	return (x > 0.0L) ? (long double)((long)x + 1) : (long double)((long)x);
}

export double copysign(double x, double y)
{
	return (y < 0.0) ? -fabs(x) : fabs(x);
}

export float copysignf(float x, float y)
{
	return (y < 0.0f) ? -fabsf(x) : fabsf(x);
}

export long double copysignl(long double x, long double y)
{
	return (y < 0.0L) ? -fabsl(x) : fabsl(x);
}

export double cos(double x)
{
	return sin(x + M_PI_2);
}

export float cosf(float x)
{
	return sinf(x + (float)M_PI_2);
}

export double cosh(double x)
{
	return (exp(x) + exp(-x)) / 2.0;
}

export float coshf(float x)
{
	return (expf(x) + expf(-x)) / 2.0f;
}

export long double coshl(long double x)
{
	return (expl(x) + expl(-x)) / 2.0L;
}

export long double cosl(long double x)
{
	return sinl(x + M_PI_2l);
}

export double erf(double x)
{
	double t = 1.0 / (1.0 + 0.5 * fabs(x));
	double tau = t * exp(-x * x - 1.26551223 + t * (1.00002368 + t * (0.37409196 + t * (0.09678418 + t * (-0.18628806 + t * (0.27886807 + t * (-1.13520398 + t * (1.48851587 + t * (-0.82215223 + t * 0.17087277)))))))));
	return (x >= 0.0) ? 1.0 - tau : tau - 1.0;
}

export double erfc(double x)
{
	return 1.0 - erf(x);
}

export float erfcf(float x)
{
	return 1.0f - erff(x);
}

export long double erfcl(long double x)
{
	return 1.0L - erfl(x);
}

export float erff(float x)
{
	float t = 1.0f / (1.0f + 0.5f * fabsf(x));
	float tau = t * expf(-x * x - 1.26551223f + t * (1.00002368f + t * (0.37409196f + t * (0.09678418f + t * (-0.18628806f + t * (0.27886807f + t * (-1.13520398f + t * (1.48851587f + t * (-0.82215223f + t * 0.17087277f)))))))));
	return (x >= 0.0f) ? 1.0f - tau : tau - 1.0f;
}

export long double erfl(long double x)
{
	long double t = 1.0L / (1.0L + 0.5L * fabsl(x));
	long double tau = t * expl(-x * x - 1.26551223L + t * (1.00002368L + t * (0.37409196L + t * (0.09678418L + t * (-0.18628806L + t * (0.27886807L + t * (-1.13520398L + t * (1.48851587L + t * (-0.82215223L + t * 0.17087277L)))))))));
	return (x >= 0.0L) ? 1.0L - tau : tau - 1.0L;
}

export double exp(double x)
{
	if (x == 0.0)
	{
		return 1.0;
	}
	if (x < 0.0)
	{
		return 1.0 / exp(-x);
	}
	double result = 1.0;
	double term = 1.0;
	for (int i = 1; term > DBL_EPSILON || term < -DBL_EPSILON; i++)
	{
		term *= x / i;
		result += term;
	}
	return result;
}

export double exp2(double x)
{
	if (x == 0.0)
	{
		return 1.0;
	}
	if (x < 0.0)
	{
		return 1.0 / exp2(-x);
	}
	double result = 1.0;
	double term = 1.0;
	for (int i = 1; term > DBL_EPSILON || term < -DBL_EPSILON; i++)
	{
		term *= x * log(2.0) / i;
		result += term;
	}
	return result;
}

export float exp2f(float x)
{
	if (x == 0.0f)
	{
		return 1.0f;
	}
	if (x < 0.0f)
	{
		return 1.0f / exp2f(-x);
	}
	float result = 1.0f;
	float term = 1.0f;
	for (int i = 1; term > FLT_EPSILON || term < -FLT_EPSILON; i++)
	{
		term *= x * logf(2.0f) / i;
		result += term;
	}
	return result;
}

export long double exp2l(long double x)
{
	return powl(2.0L, x);
}

export float expf(float x)
{
	return powf(M_E, x);
}

export long double expl(long double x)
{
	return powl(M_E, x);
}

export double expm1(double x)
{
	return exp(x) - 1.0;
}

export float expm1f(float x)
{
	return expf(x) - 1.0f;
}

export long double expm1l(long double x)
{
	return expl(x) - 1.0L;
}

export double fabs(double x)
{
	return (x < 0.0) ? -x : x;
}

export float fabsf(float x)
{
	return (x < 0.0f) ? -x : x;
}

export long double fabsl(long double x)
{
	return (x < 0.0L) ? -x : x;
}

export double fdim(double x, double y)
{
	return (x > y) ? x - y : 0.0;
}

export float fdimf(float x, float y)
{
	return (x > y) ? x - y : 0.0f;
}

export long double fdiml(long double x, long double y)
{
	return (x > y) ? x - y : 0.0L;
}

export double floor(double x)
{
	if (x == (double)((long)x))
	{
		return x;
	}
	return (x > 0.0) ? (double)((long)x) : (double)((long)x - 1);
}

export float floorf(float x)
{
	if (x == (float)((long)x))
	{
		return x;
	}
	return (x > 0.0f) ? (float)((long)x) : (float)((long)x - 1);
}

export long double floorl(long double x)
{
	if (x == (long double)((long)x))
	{
		return x;
	}
	return (x > 0.0L) ? (long double)((long)x) : (long double)((long)x - 1);
}

export double fma(double x, double y, double z)
{
	return x * y + z;
}

export float fmaf(float x, float y, float z)
{
	return x * y + z;
}

export long double fmal(long double x, long double y, long double z)
{
	return x * y + z;
}

export double fmax(double x, double y)
{
	return (x > y) ? x : y;
}

export float fmaxf(float x, float y)
{
	return (x > y) ? x : y;
}

export long double fmaxl(long double x, long double y)
{
	return (x > y) ? x : y;
}

export double fmin(double x, double y)
{
	return (x < y) ? x : y;
}

export float fminf(float x, float y)
{
	return (x < y) ? x : y;
}

export long double fminl(long double x, long double y)
{
	return (x < y) ? x : y;
}

export double fmod(double x, double y)
{
	if (y == 0.0)
	{
		errno = EDOM;
		return NAN;
	}
	return x - y * floor(x / y);
}

export float fmodf(float x, float y)
{
	if (y == 0.0f)
	{
		errno = EDOM;
		return NAN;
	}
	return x - y * floorf(x / y);
}

export long double fmodl(long double x, long double y)
{
	if (y == 0.0L)
	{
		errno = EDOM;
		return NAN;
	}
	return x - y * floorl(x / y);
}

export double frexp(double num, int *exp)
{
	if (num == 0.0)
	{
		*exp = 0;
		return 0.0;
	}
	double abs_num = fabs(num);
	*exp = (int)log2(abs_num) + 1;
	double mantissa = abs_num / pow(2.0, *exp);
	if (num < 0.0)
	{
		mantissa = -mantissa;
	}
	return mantissa;
}

export float frexpf(float num, int *exp)
{
	if (num == 0.0f)
	{
		*exp = 0;
		return 0.0f;
	}
	float abs_num = fabsf(num);
	*exp = (int)log2f(abs_num) + 1;
	float mantissa = abs_num / powf(2.0f, *exp);
	if (num < 0.0f)
	{
		mantissa = -mantissa;
	}
	return mantissa;
}

export long double frexpl(long double num, int *exp)
{
	if (num == 0.0L)
	{
		*exp = 0;
		return 0.0L;
	}
	long double abs_num = fabsl(num);
	*exp = (int)log2l(abs_num) + 1;
	long double mantissa = abs_num / powl(2.0L, *exp);
	if (num < 0.0L)
	{
		mantissa = -mantissa;
	}
	return mantissa;
}

export double hypot(double x, double y)
{
	return sqrt(x * x + y * y);
}

export float hypotf(float x, float y)
{
	return sqrtf(x * x + y * y);
}

export long double hypotl(long double x, long double y)
{
	return sqrtl(x * x + y * y);
}

export int ilogb(double x)
{
	if (x == 0.0)
	{
		errno = EDOM;
		return FP_ILOGB0;
	}
	if (isnan(x))
	{
		errno = EDOM;
		return FP_ILOGBNAN;
	}
	return (int)log2(fabs(x));
}

export int ilogbf(float x)
{
	if (x == 0.0f)
	{
		errno = EDOM;
		return FP_ILOGB0;
	}
	if (isnan(x))
	{
		errno = EDOM;
		return FP_ILOGBNAN;
	}
	return (int)log2f(fabsf(x));
}

export int ilogbl(long double x)
{
	if (x == 0.0L)
	{
		errno = EDOM;
		return FP_ILOGB0;
	}
	if (isnan(x))
	{
		errno = EDOM;
		return FP_ILOGBNAN;
	}
	return (int)log2l(fabsl(x));
}
export double j0(double x)
{
	double ax = fabs(x);
	if (ax < 8.0)
	{
		double y = x * x;
		double ans1 = 57568490574.0 + y * (-13362590354.0 + y * (651619640.7 + y * (-11214424.18 + y * (77392.33017 + y * (-184.9052456)))));
		double ans2 = 57568490411.0 + y * (1029532985.0 + y * (9494680.718 + y * (59272.64853 + y * (267.8532712 + y * 1.0))));
		return ans1 / ans2;
	}
	else
	{
		double z = 8.0 / ax;
		double y = z * z;
		double xx = ax - 0.785398164;
		double ans1 = 1.0 + y * (-0.1098628627e-2 + y * (0.2734510407e-4 + y * (-0.2073370639e-5 + y * 0.2093887211e-6)));
		double ans2 = -0.1562499995e-1 + y * (0.1430488765e-3 + y * (-0.6911147651e-5 + y * (0.7621095161e-6 - y * 0.934935152e-7)));
		return sqrt(0.636619772 / ax) * (cos(xx) * ans1 - z * sin(xx) * ans2);
	}
}

export double j1(double x)
{
	double ax = fabs(x);
	if (ax < 8.0)
	{
		double y = x * x;
		double ans1 = x * (72362614232.0 + y * (-7895059235.0 + y * (242396853.1 + y * (-2972611.439 + y * (15704.48260 + y * (-30.16036606))))));
		double ans2 = 144725228442.0 + y * (2300535178.0 + y * (18583304.74 + y * (99447.43394 + y * (376.9991397 + y * 1.0))));
		return ans1 / ans2;
	}
	else
	{
		double z = 8.0 / ax;
		double y = z * z;
		double xx = ax - 2.356194491;
		double ans1 = 1.0 + y * (0.183105e-2 + y * (-0.3516396496e-4 + y * (0.2457520174e-5 + y * (-0.240337019e-6))));
		double ans2 = 0.04687499995 + y * (-0.2002690873e-3 + y * (0.8449199096e-5 + y * (-0.88228987e-6 + y * 0.105787412e-6)));
		double ans = sqrt(0.636619772 / ax) * (cos(xx) * ans1 - z * sin(xx) * ans2);
		if (x < 0.0)
			ans = -ans;
		return ans;
	}
}

#define BIGNO 1.0e10
#define BIGNI 1.0e-10

export double jn(int n, double x)
{
	if (n == 0)
		return j0(x);
	if (n == 1)
		return j1(x);
	if (x == 0.0)
		return 0.0;
	double ax = fabs(x);
	double tox = 2.0 / ax;
	double bj, bjm, bjp, sum;
	int j, m;
	if (ax > (double)n)
	{
		bjm = j0(ax);
		bj = j1(ax);
		for (j = 1; j < n; j++)
		{
			bjp = j * tox * bj - bjm;
			bjm = bj;
			bj = bjp;
		}
		sum = bj;
	}
	else
	{
#define ACC 40
		m = 2 * ((n + (int)sqrt(ACC * n)) / 2);
		bjp = sum = 0.0;
		bj = 1.0;
		for (j = m; j > 0; j--)
		{
			bjm = j * tox * bj - bjp;
			bjp = bj;
			bj = bjm;
			if (fabs(bj) > BIGNO)
			{
				bj *= BIGNI;
				bjp *= BIGNI;
				sum *= BIGNI;
			}
			if (j == n)
				sum = bjp;
		}
		sum *= j0(ax) / bj;
	}
	return (x < 0.0 && (n & 1)) ? -sum : sum;
}

export double ldexp(double x, int exp)
{
	return x * pow(2.0, exp);
}

export float ldexpf(float x, int exp)
{
	return x * powf(2.0f, exp);
}

export long double ldexpl(long double x, int exp)
{
	return x * powl(2.0L, exp);
}
export double lgamma(double x)
{
	if (x <= 0.0 && floor(x) == x)
	{
		errno = EDOM;
		return NAN;
	}
	double result = 0.0;
	if (x < 0.5)
	{
		result = log(M_PI) - log(sin(M_PI * x)) - lgamma(1.0 - x);
	}
	else
	{
		x -= 1.0;
		double a = 0.99999999999980993;
		double coefficients[] = {
			676.5203681218851,
			-1259.1392167224028,
			771.32342877765313,
			-176.61502916214059,
			12.507343278686905,
			-0.13857109526572012,
			9.9843695780195716e-6,
			1.5056327351493116e-7};
		for (int i = 0; i < 8; i++)
		{
			a += coefficients[i] / (x + i + 1);
		}
		double t = x + 7.5;
		result = log(2.5066282746310007 * a) - t + log(t) * (x + 0.5);
	}
	return result;
}

export float lgammaf(float x)
{
	return (float)lgamma((double)x);
}

export long double lgammal(long double x)
{
	return (long double)lgamma((double)x);
}

export long long llrint(double x)
{
	return (long long)round(x);
}

export long long llrintf(float x)
{
	return (long long)roundf(x);
}

export long long llrintl(long double x)
{
	return (long long)roundl(x);
}

export long long llround(double x)
{
	return (long long)round(x);
}

export long long llroundf(float x)
{
	return (long long)roundf(x);
}

export long long llroundl(long double x)
{
	return (long long)roundl(x);
}

export double log(double x)
{
	if (x <= 0.0)
	{
		errno = EDOM;
		return NAN;
	}
	double result = 0.0;
	double y = (x - 1) / (x + 1);
	double y2 = y * y;
	double term = y;
	for (int i = 1; term > DBL_EPSILON || term < -DBL_EPSILON; i += 2)
	{
		result += term / i;
		term *= y2;
	}
	return 2 * result;
}

export double log10(double x)
{
	return log(x) / log(10.0);
}

export float log10f(float x)
{
	return (float)log10((double)x);
}

export long double log10l(long double x)
{
	return (long double)log10((double)x);
}

export double log1p(double x)
{
	if (x <= -1.0)
	{
		errno = EDOM;
		return NAN;
	}
	double result = 0.0;
	double term = x;
	for (int i = 1; term > DBL_EPSILON || term < -DBL_EPSILON; i++)
	{
		result += term / i;
		term *= -x;
	}
	return result;
}

export float log1pf(float x)
{
	return (float)log1p((double)x);
}

export long double log1pl(long double x)
{
	return (long double)log1p((double)x);
}

export double log2(double x)
{
	return log(x) / log(2.0);
}

export float log2f(float x)
{
	return (float)log2((double)x);
}

export long double log2l(long double x)
{
	return (long double)log2((double)x);
}

export double logb(double x)
{
	if (x == 0.0)
	{
		errno = ERANGE;
		return -HUGE_VAL;
	}
	if (isnan(x))
	{
		errno = EDOM;
		return NAN;
	}
	if (isinf(x))
	{
		return HUGE_VAL;
	}
	int exp;
	frexp(x, &exp);
	return (double)(exp - 1);
}

export float logbf(float x)
{
	return (float)logb((double)x);
}

export long double logbl(long double x)
{
	return (long double)logb((double)x);
}

export float logf(float x)
{
	return (float)log((double)x);
}

export long double logl(long double x)
{
	return (long double)log((double)x);
}

export long lrint(double x)
{
	if (isnan(x) || isinf(x))
	{
		errno = EDOM;
		return 0;
	}
	return (long)rint(x);
}

export long lrintf(float x)
{
	if (isnan(x) || isinf(x))
	{
		errno = EDOM;
		return 0;
	}
	return (long)rintf(x);
}

export long lrintl(long double x)
{
	if (isnan(x) || isinf(x))
	{
		errno = EDOM;
		return 0;
	}
	return (long)rintl(x);
}

export long lround(double x)
{
	return (long)round(x);
}

export long lroundf(float x)
{
	return (long)roundf(x);
}

export long lroundl(long double x)
{
	return (long)roundl(x);
}

export double modf(double x, double *iptr)
{
	*iptr = floor(x);
	return x - *iptr;
}

export float modff(float value, float *iptr)
{
	*iptr = floorf(value);
	return value - *iptr;
}

export long double modfl(long double x, long double *iptr)
{
	*iptr = floorl(x);
	return x - *iptr;
}

export double nan(const char *tagp)
{
	return NAN;
}

export float nanf(const char *tagp)
{
	return NAN;
}

export long double nanl(const char *tagp)
{
	return NAN;
}

export double nearbyint(double x)
{
	return round(x);
}

export float nearbyintf(float x)
{
	return roundf(x);
}

export long double nearbyintl(long double x)
{
	return roundl(x);
}

export double nextafter(double x, double y)
{
	if (isnan(x) || isnan(y))
	{
		return NAN;
	}
	if (x == y)
	{
		return y;
	}
	if (x == 0.0)
	{
		return (y > 0.0) ? DBL_MIN : -DBL_MIN;
	}
	union
	{
		double d;
		uint64_t u;
	} ux = {x};
	if ((x > 0.0) == (y > x))
	{
		ux.u++;
	}
	else
	{
		ux.u--;
	}
	return ux.d;
}

export float nextafterf(float x, float y)
{
	if (isnan(x) || isnan(y))
	{
		return NAN;
	}
	if (x == y)
	{
		return y;
	}
	if (x == 0.0f)
	{
		return (y > 0.0f) ? FLT_MIN : -FLT_MIN;
	}
	union
	{
		float f;
		uint32_t u;
	} ux = {x};
	if ((x > 0.0f) == (y > x))
	{
		ux.u++;
	}
	else
	{
		ux.u--;
	}
	return ux.f;
}

export long double nextafterl(long double x, long double y)
{
	if (isnan(x) || isnan(y))
	{
		return NAN;
	}
	if (x == y)
	{
		return y;
	}
	if (x == 0.0L)
	{
		return (y > 0.0L) ? LDBL_MIN : -LDBL_MIN;
	}
	union
	{
		long double ld;
		uint64_t u[2];
	} ux = {x};
	if ((x > 0.0L) == (y > x))
	{
		ux.u[0]++;
		if (ux.u[0] == 0)
		{
			ux.u[1]++;
		}
	}
	else
	{
		if (ux.u[0] == 0)
		{
			ux.u[1]--;
		}
		ux.u[0]--;
	}
	return ux.ld;
}

export double nexttoward(double x, long double y)
{
	return nextafterl(x, y);
}

export float nexttowardf(float x, long double y)
{
	return nextafterl(x, y);
}

export long double nexttowardl(long double x, long double y)
{
	return nextafterl(x, y);
}

export double pow(double x, double y)
{
	if (x == 0.0 && y == 0.0)
	{
		errno = EDOM;
		return NAN;
	}
	return exp(y * log(x));
}

export float powf(float x, float y)
{
	return (float)pow((double)x, (double)y);
}

export long double powl(long double x, long double y)
{
	return (long double)pow((double)x, (double)y);
}

export double remainder(double x, double y)
{
	if (y == 0.0)
	{
		errno = EDOM;
		return NAN;
	}
	return x - y * round(x / y);
}

export float remainderf(float x, float y)
{
	return (float)remainder((double)x, (double)y);
}

export long double remainderl(long double x, long double y)
{
	return (long double)remainder((double)x, (double)y);
}

export double remquo(double x, double y, int *quo)
{
	if (y == 0.0)
	{
		errno = EDOM;
		return NAN;
	}
	double result = remainder(x, y);
	*quo = (int)((x - result) / y);
	return result;
}

export float remquof(float x, float y, int *quo)
{
	return (float)remquo((double)x, (double)y, quo);
}

export long double remquol(long double x, long double y, int *quo)
{
	return (long double)remquo((double)x, (double)y, quo);
}

export double rint(double x)
{
	int current_rounding_mode = fegetround();
	double result;

	switch (current_rounding_mode)
	{
	case FE_TONEAREST:
		result = nearbyint(x);
		break;
	case FE_DOWNWARD:
		result = floor(x);
		break;
	case FE_UPWARD:
		result = ceil(x);
		break;
	case FE_TOWARDZERO:
		result = trunc(x);
		break;
	default:
		result = nearbyint(x);
		break;
	}

	if (result != x)
	{
		feraiseexcept(FE_INEXACT);
	}

	return result;
}

export float rintf(float x)
{
	int current_rounding_mode = fegetround();
	float result;

	switch (current_rounding_mode)
	{
	case FE_TONEAREST:
		result = nearbyintf(x);
		break;
	case FE_DOWNWARD:
		result = floorf(x);
		break;
	case FE_UPWARD:
		result = ceilf(x);
		break;
	case FE_TOWARDZERO:
		result = truncf(x);
		break;
	default:
		result = nearbyintf(x);
		break;
	}

	if (result != x)
	{
		feraiseexcept(FE_INEXACT);
	}

	return result;
}

export long double rintl(long double x)
{
	int current_rounding_mode = fegetround();
	long double result;

	switch (current_rounding_mode)
	{
	case FE_TONEAREST:
		result = nearbyintl(x);
		break;
	case FE_DOWNWARD:
		result = floorl(x);
		break;
	case FE_UPWARD:
		result = ceill(x);
		break;
	case FE_TOWARDZERO:
		result = truncl(x);
		break;
	default:
		result = nearbyintl(x);
		break;
	}

	if (result != x)
	{
		feraiseexcept(FE_INEXACT);
	}

	return result;
}

export double round(double x)
{
	return (x < 0.0) ? ceil(x - 0.5) : floor(x + 0.5);
}

export float roundf(float x)
{
	return (x < 0.0f) ? ceilf(x - 0.5f) : floorf(x + 0.5f);
}

export long double roundl(long double x)
{
	return (x < 0.0L) ? ceill(x - 0.5L) : floorl(x + 0.5L);
}

export double scalb(double x, double n)
{
	if (isnan(x) || isnan(n))
	{
		return NAN;
	}
	if (n == 0.0)
	{
		return x;
	}
	if (isinf(x) && !isinf(n))
	{
		return x;
	}
	if (x == 0.0 && !isinf(n))
	{
		return x;
	}
	if (x == 0.0 && isinf(n))
	{
		errno = EDOM;
		return NAN;
	}
	if (isinf(x) && isinf(n))
	{
		errno = EDOM;
		return NAN;
	}
	if (isinf(n))
	{
		return (n > 0.0) ? HUGE_VAL : 0.0;
	}
	if (isinf(-n))
	{
		return (n < 0.0) ? -HUGE_VAL : 0.0;
	}

	int exp = (int)n;
	double result = x;
	if (exp > 0)
	{
		while (exp > 0)
		{
			result *= 2.0;
			exp--;
		}
	}
	else
	{
		while (exp < 0)
		{
			result /= 2.0;
			exp++;
		}
	}
	return result;
}

export double scalbln(double x, long n)
{
	return x * pow(2.0, (double)n);
}

export float scalblnf(float x, long n)
{
	return x * powf(2.0f, (float)n);
}

export long double scalblnl(long double x, long n)
{
	return x * powl(2.0L, (long double)n);
}

export double scalbn(double x, int n)
{
	return x * pow(2.0, (double)n);
}

export float scalbnf(float x, int n)
{
	return x * powf(2.0f, (float)n);
}

export long double scalbnl(long double x, int n)
{
	return x * powl(2.0L, (long double)n);
}

export double sin(double x)
{
	double result = 0.0;
	double term = x;
	double x2 = x * x;
	for (int i = 1; term > DBL_EPSILON || term < -DBL_EPSILON; i += 2)
	{
		result += term;
		term *= -x2 / ((i + 1) * (i + 2));
	}
	return result;
}

export float sinf(float x)
{
	return (float)sin((double)x);
}

export double sinh(double x)
{
	return (exp(x) - exp(-x)) / 2.0;
}

export float sinhf(float x)
{
	return (float)sinh((double)x);
}

export long double sinhl(long double x)
{
	return (long double)sinh((double)x);
}

export long double sinl(long double x)
{
	return (long double)sin((double)x);
}

export double sqrt(double x)
{
	if (x < 0.0)
	{
		errno = EDOM;
		return NAN;
	}
	double result = x;
	double last_result;
	do
	{
		last_result = result;
		result = (result + x / result) / 2.0;
	} while (fabs(result - last_result) > DBL_EPSILON);
	return result;
}

export float sqrtf(float x)
{
	return (float)sqrt((double)x);
}

export long double sqrtl(long double x)
{
	return (long double)sqrt((double)x);
}

export double tan(double x)
{
	return sin(x) / cos(x);
}

export float tanf(float x)
{
	return (float)tan((double)x);
}

export double tanh(double x)
{
	return (exp(x) - exp(-x)) / (exp(x) + exp(-x));
}

export float tanhf(float x)
{
	return (float)tanh((double)x);
}

export long double tanhl(long double x)
{
	return (long double)tanh((double)x);
}

export long double tanl(long double x)
{
	return (long double)tan((double)x);
}

export double tgamma(double x)
{
	if (x <= 0.0 && floor(x) == x)
	{
		errno = EDOM;
		return NAN;
	}
	double result = 1.0;
	if (x < 0.5)
	{
		return M_PI / (sin(M_PI * x) * tgamma(1.0 - x));
	}
	x -= 1.0;
	double coefficients[] = {
		1.000000000190015,
		76.18009172947146,
		-86.50532032941677,
		24.01409824083091,
		-1.231739572450155,
		0.001208650973866179,
		-0.000005395239384953};
	double y = x + 5.5;
	y -= (x + 0.5) * log(y);
	for (int i = 0; i < 7; i++)
	{
		result += coefficients[i] / (x + i + 1);
	}
	return sqrt(2.0 * M_PI) * pow(x + 5.5, x + 0.5) * exp(-y) * result;
}

export float tgammaf(float x)
{
	return (float)tgamma((double)x);
}

export long double tgammal(long double x)
{
	return (long double)tgamma((double)x);
}

export double trunc(double x)
{
	return (x < 0.0) ? ceil(x) : floor(x);
}

export float truncf(float x)
{
	return (x < 0.0f) ? ceilf(x) : floorf(x);
}

export long double truncl(long double x)
{
	return (x < 0.0L) ? ceill(x) : floorl(x);
}

export double y0(double x)
{
	if (x < 8.0)
	{
		double y = x * x;
		double ans1 = -2957821389.0 + y * (7062834065.0 + y * (-512359803.6 + y * (10879881.29 + y * (-86327.92757 + y * 228.4622733))));
		double ans2 = 40076544269.0 + y * (745249964.8 + y * (7189466.438 + y * (47447.26470 + y * (226.1030244 + y * 1.0))));
		return (ans1 / ans2) + 0.636619772 * j0(x) * log(x);
	}
	else
	{
		double z = 8.0 / x;
		double y = z * z;
		double xx = x - 0.785398164;
		double ans1 = 1.0 + y * (-0.1098628627e-2 + y * (0.2734510407e-4 + y * (-0.2073370639e-5 + y * 0.2093887211e-6)));
		double ans2 = -0.1562499995e-1 + y * (0.1430488765e-3 + y * (-0.6911147651e-5 + y * (0.7621095161e-6 - y * 0.934935152e-7)));
		return sqrt(0.636619772 / x) * (sin(xx) * ans1 + z * cos(xx) * ans2);
	}
}

export double y1(double x)
{
	if (x < 8.0)
	{
		double y = x * x;
		double ans1 = x * (-0.4900604943e13 + y * (0.1275274390e13 + y * (-0.5153438139e11 + y * (0.7349264551e9 + y * (-0.4237922726e7 + y * 0.8511937935e4)))));
		double ans2 = 0.2499580570e14 + y * (0.4244419664e12 + y * (0.3733650367e10 + y * (0.2245904002e8 + y * (0.1020426050e6 + y * (0.3549632885e3 + y)))));
		return (ans1 / ans2) + 0.636619772 * (j1(x) * log(x) - 1.0 / x);
	}
	else
	{
		double z = 8.0 / x;
		double y = z * z;
		double xx = x - 2.356194491;
		double ans1 = 1.0 + y * (0.183105e-2 + y * (-0.3516396496e-4 + y * (0.2457520174e-5 + y * (-0.240337019e-6))));
		double ans2 = 0.04687499995 + y * (-0.2002690873e-3 + y * (0.8449199096e-5 + y * (-0.88228987e-6 + y * 0.105787412e-6)));
		return sqrt(0.636619772 / x) * (sin(xx) * ans1 + z * cos(xx) * ans2);
	}
}

export double yn(int n, double x)
{
	if (n == 0)
		return y0(x);
	if (n == 1)
		return y1(x);
	if (x == 0.0)
		return -HUGE_VAL;
	if (x < 0.0)
		return NAN;
	double tox = 2.0 / x;
	double by, bym, byp;
	int j;
	bym = y0(x);
	by = y1(x);
	for (j = 1; j < n; j++)
	{
		byp = j * tox * by - bym;
		bym = by;
		by = byp;
	}
	return by;
}
