#include <stdio.h>

#include "../../../Kernel/syscalls.h"

void __libc_init_std(void)
{
	/* FIXME: Temporal workaround */
	// int IsCritical = syscall1(_KernelCTL, 6 /* KCTL_IS_CRITICAL */);
}

void __libc_fini_std(void)
{
}
