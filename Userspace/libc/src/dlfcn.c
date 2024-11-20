#include <dlfcn.h>

static char *error = "Not implemented";

__attribute__((weak)) void *dlopen(const char *filename, int flags)
{
	return NULL;
}

__attribute__((weak)) void *dlsym(void *handle, const char *symbol)
{
	return NULL;
}

__attribute__((weak)) int dlclose(void *handle)
{
	return -1;
}

__attribute__((weak)) char *dlerror(void)
{
	return error;
}
