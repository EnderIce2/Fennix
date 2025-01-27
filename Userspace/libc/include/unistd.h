/*
	This file is part of Fennix C Library.

	Fennix C Library is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix C Library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix C Library. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _UNISTD_H
#define _UNISTD_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <sys/types.h>
#include <inttypes.h>

#define _POSIX_VERSION 200809L
#define _POSIX2_VERSION 200809L
#define _XOPEN_VERSION 700

#define F_OK 0
#define R_OK 1
#define W_OK 2
#define X_OK 3

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#ifndef NULL
#define NULL ((void *)0)
#endif

#define _POSIX_JOB_CONTROL 1
#define _POSIX_SAVED_IDS 1
#define _POSIX_NO_TRUNC 1
#define _POSIX_VDISABLE '\0'

#define ARG_MAX 4096
#define CHILD_MAX 25
#define CLK_TCK 100

#define _SC_PAGESIZE 4096
#define _SC_OPEN_MAX 256

	extern char *optarg;
	extern int optind, opterr, optopt;
	extern char **environ;

	int access(const char *, int);
	unsigned int alarm(unsigned int seconds);
	int brk(void *);
	int chdir(const char *);
	int chroot(const char *);
	int chown(const char *, uid_t, gid_t);
	int close(int fildes);
	size_t confstr(int, char *, size_t);
	char *crypt(const char *, const char *);
	char *ctermid(char *);
	char *cuserid(char *s);
	int dup(int);
	int dup2(int, int);
	void encrypt(char[64], int);
	int execl(const char *path, const char *arg0, ... /*, (char *)0 */);
	int execle(const char *path, const char *arg0, ... /*, (char *)0, char *const envp[]*/);
	int execlp(const char *file, const char *arg0, ... /*, (char *)0 */);
	int execv(const char *path, char *const argv[]);
	int execve(const char *path, char *const argv[], char *const envp[]);
	int execvp(const char *file, char *const argv[]);
	void _exit(int);
	int fchown(int, uid_t, gid_t);
	int fchdir(int);
	int fdatasync(int);
	pid_t fork(void);
	long int fpathconf(int, int);
	int fsync(int);
	int ftruncate(int, off_t);
	char *getcwd(char *, size_t);
	int getdtablesize(void);
	gid_t getegid(void);
	uid_t geteuid(void);
	gid_t getgid(void);
	int getgroups(int, gid_t[]);
	long gethostid(void);
	char *getlogin(void);
	int getlogin_r(char *, size_t);
	int getopt(int, char *const[], const char *);
	int getpagesize(void);
	char *getpass(const char *);
	pid_t getpgid(pid_t);
	pid_t getpgrp(void);
	pid_t getpid(void);
	pid_t getppid(void);
	pid_t getsid(pid_t);
	uid_t getuid(void);
	char *getwd(char *);
	int isatty(int);
	int lchown(const char *, uid_t, gid_t);
	int link(const char *, const char *);
	int lockf(int, int, off_t);
	off_t lseek(int, off_t, int);
	int nice(int);
	long int pathconf(const char *, int);
	int pause(void);
	int pipe(int[2]);
	ssize_t pread(int, void *, size_t, off_t);
	int pthread_atfork(void (*)(void), void (*)(void), void (*)(void));
	ssize_t pwrite(int, const void *, size_t, off_t);
	ssize_t read(int, void *, size_t);
	int readlink(const char *, char *, size_t);
	int rmdir(const char *);
	void *sbrk(intptr_t);
	int setgid(gid_t);
	int setpgid(pid_t, pid_t);
	pid_t setpgrp(void);
	int setregid(gid_t, gid_t);
	int setreuid(uid_t, uid_t);
	pid_t setsid(void);
	int setuid(uid_t);
	unsigned int sleep(unsigned int seconds);
	void swab(const void *, void *, ssize_t);
	int symlink(const char *, const char *);
	void sync(void);
	long int sysconf(int);
	pid_t tcgetpgrp(int);
	int tcsetpgrp(int, pid_t);
	int truncate(const char *, off_t);
	char *ttyname(int);
	int ttyname_r(int, char *, size_t);
	useconds_t ualarm(useconds_t, useconds_t);
	int unlink(const char *);
	int usleep(useconds_t);
	pid_t vfork(void);
	ssize_t write(int, const void *, size_t);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_UNISTD_H
