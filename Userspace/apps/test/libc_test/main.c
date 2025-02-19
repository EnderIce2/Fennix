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
