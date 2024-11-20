#include <unistd.h>
#include <stddef.h>
#include <errno.h>
#include "../../../Kernel/syscalls.h"

PUBLIC int execl(const char *pathname, const char *arg, ...)
{
    errno = ENOSYS;
    return -1;
}

PUBLIC int execlp(const char *file, const char *arg, ...)
{
    errno = ENOSYS;
    return -1;
}

PUBLIC int execle(const char *pathname, const char *arg, ...)
{
    errno = ENOSYS;
    return -1;
}

PUBLIC int execv(const char *pathname, char *const argv[])
{
    errno = ENOSYS;
    return -1;
}

PUBLIC int execvp(const char *file, char *const argv[])
{
    errno = ENOSYS;
    return -1;
}

PUBLIC int execvpe(const char *file, char *const argv[], char *const envp[])
{
    errno = ENOSYS;
    return -1;
}

PUBLIC int execve(const char *pathname, char *const argv[], char *const envp[])
{
    errno = ENOSYS;
    return -1;
}

PUBLIC pid_t fork(void)
{
    return syscall0(sc_fork);
}
