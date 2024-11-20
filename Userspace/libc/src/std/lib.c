#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <fennix/syscall.h>

#include "../mem/liballoc_1_1.h"

PUBLIC void abort(void)
{
	syscall1(sc_exit, -0xAB057);
	while (1)
		;
}

PUBLIC int atexit(void (*function)(void))
{
	return 1;
}

PUBLIC void exit(int status)
{
	_exit(status);
}

PUBLIC int atoi(const char *nptr)
{
	uint64_t Length = strlen((char *)nptr);
	if (nptr)
		while (nptr[Length] != '\0')
			++Length;
	uint64_t OutBuffer = 0;
	uint64_t Power = 1;
	for (uint64_t i = Length; i > 0; --i)
	{
		OutBuffer += (nptr[i - 1] - 48) * Power;
		Power *= 10;
	}
	return OutBuffer;
}

PUBLIC char **environ = NULL;

PUBLIC char *getenv(const char *name)
{
	char **env = environ;
	if (env == NULL)
		return NULL;
	size_t len = strlen(name);
	while (*env != NULL)
	{
		if ((strncmp(*env, name, len) == 0) && ((*env)[len] == '='))
			return &(*env)[len + 1];
		++env;
	}
}

PUBLIC void *malloc(size_t Size) { return PREFIX(malloc)(Size); }
PUBLIC void *realloc(void *Address, size_t Size) { return PREFIX(realloc)(Address, Size); }
PUBLIC void *calloc(size_t Count, size_t Size) { return PREFIX(calloc)(Count, Size); }
PUBLIC void free(void *Address)
{
	PREFIX(free)
	(Address);
}

PUBLIC int system(const char *command)
{
	return -1;
}

PUBLIC double atof(const char *nptr)
{
	// FIXME: This is a very bad implementation of atof.
	uint64_t Length = strlen((char *)nptr);
	if (nptr)
		while (nptr[Length] != '\0')
			++Length;
	double OutBuffer = 0;
	double Power = 1;
	for (uint64_t i = Length; i > 0; --i)
	{
		OutBuffer += (nptr[i - 1] - 48) * Power;
		Power *= 10;
	}
	return OutBuffer;
}
