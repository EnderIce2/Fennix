#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

	extern char **environ;

	int execl(const char *pathname, const char *arg, ...);
	int execlp(const char *file, const char *arg, ...);
	int execle(const char *pathname, const char *arg, ...);
	int execv(const char *pathname, char *const argv[]);
	int execvp(const char *file, char *const argv[]);
	int execvpe(const char *file, char *const argv[], char *const envp[]);
	int execve(const char *pathname, char *const argv[], char *const envp[]);

	pid_t fork(void);

	unsigned int sleep(unsigned int seconds);
	int usleep(useconds_t usec);

#ifdef __cplusplus
}
#endif
#endif
