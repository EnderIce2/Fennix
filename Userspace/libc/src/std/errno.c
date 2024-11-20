#include <errno.h>

#include <sys/types.h> // For PUBLIC

int __local_stub_errno = 0;

PUBLIC int *__errno_location(void)
{
    return &__local_stub_errno;
}
