#ifndef __FENNIX_LIBC_DLFCN_H__
#define __FENNIX_LIBC_DLFCN_H__

#include <types.h>

void *dlopen(const char *filename, int flags);
void *dlsym(void *handle, const char *symbol);
int dlclose(void *handle);
char *dlerror(void);

#endif // !__FENNIX_LIBC_DLFCN_H__
