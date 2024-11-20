#include <setjmp.h>

PUBLIC int setjmp(jmp_buf env)
{
	return 0;
}

PUBLIC __attribute__((noreturn)) void longjmp(jmp_buf env, int value)
{
	_exit(value);
}