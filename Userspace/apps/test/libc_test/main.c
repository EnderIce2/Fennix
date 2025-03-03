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

#include <stdio.h>
#include <unistd.h>

__attribute__((noreturn)) __attribute__((no_stack_protector)) void __stack_chk_fail(void)
{
	_exit(0xbeef);
	__builtin_unreachable();
}

int sample_test_pass()
{
	return 0;
}

int sample_test_fail()
{
	return 1;
}

#define TEST(func)                                           \
	do                                                       \
	{                                                        \
		printf("Testing \033[1;30m%s\033[0m...", #func);     \
		fflush(stdout);                                      \
		int func(void); /* Declare the function */           \
		int result = func();                                 \
		if (result == 0)                                     \
			printf("\033[0;32m PASS (%d)\033[0m\n", result); \
		else                                                 \
		{                                                    \
			printf("\033[0;31m FAIL (%d)\033[0m\n", result); \
			failed_tests[failed_count++] = #func;            \
		}                                                    \
		fflush(stdout);                                      \
		total_tests++;                                       \
	} while (0)

int main(int, char *[])
{
	printf("---     Fennix C Library Test Suite     ---\n");
	printf("Required functions: printf, fflush & _exit\n");
	printf("-------------------------------------------\n");

	char *failed_tests[100];
	int failed_count = 0;
	int total_tests = 0;

	printf("--- assert.h ---\n");
	TEST(test_assert);

	printf("--- dirent.h ---\n");
	TEST(test_alphasort);
	TEST(test_closedir);
	TEST(test_dirfd);
	TEST(test_fdopendir);
	TEST(test_opendir);
	TEST(test_posix_getdents);
	TEST(test_readdir_r);
	TEST(test_readdir);
	TEST(test_rewinddir);
	TEST(test_scandir);
	TEST(test_seekdir);
	TEST(test_telldir);

	printf("--- errno.h ---\n");
	TEST(test___errno_location);
	TEST(test_strerror);

	printf("--- fnctl.h ---\n");
	TEST(test_creat);
	TEST(test_fcntl);
	TEST(test_open);
	TEST(test_openat);
	TEST(test_posix_fadvise);
	TEST(test_posix_fallocate);

	printf("--- string.h ---\n");
	TEST(test_memccpy);
	TEST(test_memchr);
	TEST(test_memcmp);
	TEST(test_memcpy);
	TEST(test_memmem);
	TEST(test_memmove);
	TEST(test_memset);
	TEST(test_stpcpy);
	TEST(test_stpncpy);
	TEST(test_strcat);
	TEST(test_strchr);
	TEST(test_strcmp);
	TEST(test_strcoll);
	TEST(test_strcoll_l);
	TEST(test_strcpy);
	TEST(test_strcspn);
	TEST(test_strdup);
	TEST(test_strerror);
	TEST(test_strerror_l);
	TEST(test_strerror_r);
	TEST(test_strlcat);
	TEST(test_strlcpy);
	TEST(test_strlen);
	TEST(test_strncat);
	TEST(test_strncmp);
	TEST(test_strncpy);
	TEST(test_strndup);
	TEST(test_strnlen);
	TEST(test_strpbrk);
	TEST(test_strrchr);
	TEST(test_strsignal);
	TEST(test_strspn);
	TEST(test_strstr);
	TEST(test_strtok);
	TEST(test_strtok_r);
	TEST(test_strxfrm);
	TEST(test_strxfrm_l);

	printf("--- math.h ---\n");
	TEST(test_acos);
	TEST(test_acosf);
	TEST(test_acosh);
	TEST(test_acoshf);
	TEST(test_acoshl);
	TEST(test_acosl);
	TEST(test_asin);
	TEST(test_asinf);
	TEST(test_asinh);
	TEST(test_asinhf);
	TEST(test_asinhl);
	TEST(test_asinl);
	TEST(test_atan);
	TEST(test_atan2);
	TEST(test_atan2f);
	TEST(test_atan2l);
	TEST(test_atanf);
	TEST(test_atanh);
	TEST(test_atanhf);
	TEST(test_atanhl);
	TEST(test_atanl);
	TEST(test_cbrt);
	TEST(test_cbrtf);
	TEST(test_cbrtl);
	TEST(test_ceil);
	TEST(test_ceilf);
	TEST(test_ceill);
	TEST(test_copysign);
	TEST(test_copysignf);
	TEST(test_copysignl);
	TEST(test_cos);
	TEST(test_cosf);
	TEST(test_cosh);
	TEST(test_coshf);
	TEST(test_coshl);
	TEST(test_cosl);
	TEST(test_erf);
	TEST(test_erfc);
	TEST(test_erfcf);
	TEST(test_erfcl);
	TEST(test_erff);
	TEST(test_erfl);
	TEST(test_exp);
	TEST(test_exp2);
	TEST(test_exp2f);
	TEST(test_exp2l);
	TEST(test_expf);
	TEST(test_expl);
	TEST(test_expm1);
	TEST(test_expm1f);
	TEST(test_expm1l);
	TEST(test_fabs);
	TEST(test_fabsf);
	TEST(test_fabsl);
	TEST(test_fdim);
	TEST(test_fdimf);
	TEST(test_fdiml);
	TEST(test_floor);
	TEST(test_floorf);
	TEST(test_floorl);
	TEST(test_fma);
	TEST(test_fmaf);
	TEST(test_fmal);
	TEST(test_fmax);
	TEST(test_fmaxf);
	TEST(test_fmaxl);
	TEST(test_fmin);
	TEST(test_fminf);
	TEST(test_fminl);
	TEST(test_fmod);
	TEST(test_fmodf);
	TEST(test_fmodl);
	TEST(test_frexp);
	TEST(test_frexpf);
	TEST(test_frexpl);
	TEST(test_hypot);
	TEST(test_hypotf);
	TEST(test_hypotl);
	TEST(test_ilogb);
	TEST(test_ilogbf);
	TEST(test_ilogbl);
	TEST(test_j0);
	TEST(test_j1);
	TEST(test_jn);
	TEST(test_ldexp);
	TEST(test_ldexpf);
	TEST(test_ldexpl);
	TEST(test_lgamma);
	TEST(test_lgammaf);
	TEST(test_lgammal);
	TEST(test_llrint);
	TEST(test_llrintf);
	TEST(test_llrintl);
	TEST(test_llround);
	TEST(test_llroundf);
	TEST(test_llroundl);
	TEST(test_log);
	TEST(test_log10);
	TEST(test_log10f);
	TEST(test_log10l);
	TEST(test_log1p);
	TEST(test_log1pf);
	TEST(test_log1pl);
	TEST(test_log2);
	TEST(test_log2f);
	TEST(test_log2l);
	TEST(test_logb);
	TEST(test_logbf);
	TEST(test_logbl);
	TEST(test_logf);
	TEST(test_logl);
	TEST(test_lrint);
	TEST(test_lrintf);
	TEST(test_lrintl);
	TEST(test_lround);
	TEST(test_lroundf);
	TEST(test_lroundl);
	TEST(test_modf);
	TEST(test_modff);
	TEST(test_modfl);
	TEST(test_nan);
	TEST(test_nanf);
	TEST(test_nanl);
	TEST(test_nearbyint);
	TEST(test_nearbyintf);
	TEST(test_nearbyintl);
	TEST(test_nextafter);
	TEST(test_nextafterf);
	TEST(test_nextafterl);
	TEST(test_nexttoward);
	TEST(test_nexttowardf);
	TEST(test_nexttowardl);
	TEST(test_pow);
	TEST(test_powf);
	TEST(test_powl);
	TEST(test_remainder);
	TEST(test_remainderf);
	TEST(test_remainderl);
	TEST(test_remquo);
	TEST(test_remquof);
	TEST(test_remquol);
	TEST(test_rint);
	TEST(test_rintf);
	TEST(test_rintl);
	TEST(test_round);
	TEST(test_roundf);
	TEST(test_roundl);
	TEST(test_scalb);
	TEST(test_scalbln);
	TEST(test_scalblnf);
	TEST(test_scalblnl);
	TEST(test_scalbn);
	TEST(test_scalbnf);
	TEST(test_scalbnl);
	TEST(test_sin);
	TEST(test_sinf);
	TEST(test_sinh);
	TEST(test_sinhf);
	TEST(test_sinhl);
	TEST(test_sinl);
	TEST(test_sqrt);
	TEST(test_sqrtf);
	TEST(test_sqrtl);
	TEST(test_tan);
	TEST(test_tanf);
	TEST(test_tanh);
	TEST(test_tanhf);
	TEST(test_tanhl);
	TEST(test_tanl);
	TEST(test_tgamma);
	TEST(test_tgammaf);
	TEST(test_tgammal);
	TEST(test_trunc);
	TEST(test_truncf);
	TEST(test_truncl);
	TEST(test_y0);
	TEST(test_y1);
	TEST(test_yn);

	// TEST();

	printf("-------------------------------------------\n");
	printf("Total tests: \033[1;34m%d\033[0m\n", total_tests);
	printf("Failed tests: \033[1;31m%d\033[0m\n", failed_count);
	if (failed_count > 0)
	{
		printf("Failed test functions:\n");
		for (int i = 0; i < failed_count; i++)
			printf(" - \033[1;31m%s\033[0m\n", failed_tests[i]);
	}
	printf("Failure rate: \033[1;34m%.2f%%\033[0m\n", (failed_count / (float)total_tests) * 100);
	return 0;
}
