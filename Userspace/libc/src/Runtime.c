#include <fennix/syscall.h>
#include <sys/types.h> // For PUBLIC

extern void __libc_init_array(void);
extern void __libc_fini_array(void);

extern void __libc_init_std(void);
extern void __libc_fini_std(void);

PUBLIC void __libc_init(void)
{
	__libc_init_array();
	__libc_init_std();
}

PUBLIC void _exit(int Code)
{
	__libc_fini_std();
	__libc_fini_array();
	syscall1(sc_exit, (long)Code);
	while (1)
		;
}
