#include <stdio.h>
#include <stdarg.h>
#include <fennix/syscall.h>

#include <sys/types.h> // For PUBLIC

PUBLIC int fputc(int c, FILE *stream)
{
	// FIXME
	// if (stream == NULL)
	// {
	//     errno = EBADF;
	//     return EOF;
	// }
	char str[2] = {c, '\0'};
	// return syscall3(sys_KernelCTL, KCTL_PRINT, str, 0);
}

PUBLIC int putc(int c, FILE *stream) { return fputc(c, stream); }

PUBLIC int fputs(const char *s, FILE *stream)
{
	for (int i = 0; s[i] != '\0'; i++)
		fputc(s[i], stream);
}

PUBLIC int puts(const char *s)
{
	for (int i = 0; s[i] != '\0'; i++)
		fputc(s[i], stdout);
}

PUBLIC int putchar(int c) { return fputc(c, stdout); }
PUBLIC void perror(const char *s) { fputs(s, stderr); }
