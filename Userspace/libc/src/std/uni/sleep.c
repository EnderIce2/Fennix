#include <unistd.h>
#include <stddef.h>
#include <errno.h>
#include "../../../Kernel/syscalls.h"

PUBLIC unsigned int sleep(unsigned int seconds)
{
	// return syscall1(sys_Sleep, seconds * 1000000);
}

PUBLIC int usleep(useconds_t usec)
{
	// return syscall1(sys_Sleep, usec);
}
