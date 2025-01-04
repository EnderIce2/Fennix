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

#include <unistd.h>
#include <fennix/syscalls.h>

export char *optarg;
export int optind, opterr, optopt;

export int access(const char *, int);
export unsigned int alarm(unsigned int);
export int brk(void *);
export int chdir(const char *);
export int chroot(const char *);
export int chown(const char *, uid_t, gid_t);
export int close(int);
export size_t confstr(int, char *, size_t);
export char *crypt(const char *, const char *);
export char *ctermid(char *);
export char *cuserid(char *s);
export int dup(int);
export int dup2(int, int);
export void encrypt(char[64], int);
export int execl(const char *, const char *, ...);
export int execle(const char *, const char *, ...);
export int execlp(const char *, const char *, ...);
export int execv(const char *, char *const[]);
export int execve(const char *, char *const[], char *const[]);
export int execvp(const char *, char *const[]);
export void _exit(int);
export int fchown(int, uid_t, gid_t);
export int fchdir(int);
export int fdatasync(int);
export pid_t fork(void);
export long int fpathconf(int, int);
export int fsync(int);
export int ftruncate(int, off_t);
export char *getcwd(char *, size_t);
export int getdtablesize(void);
export gid_t getegid(void);
export uid_t geteuid(void);
export gid_t getgid(void);
export int getgroups(int, gid_t[]);
export long gethostid(void);
export char *getlogin(void);
export int getlogin_r(char *, size_t);
export int getopt(int, char *const[], const char *);

export int getpagesize(void) { return 0x1000; } /* TODO: getpagesize */

export char *getpass(const char *);
export pid_t getpgid(pid_t);
export pid_t getpgrp(void);

export pid_t getpid(void) { return syscall0(SYS_GETPID); }
export pid_t getppid(void) { return syscall0(SYS_GETPPID); }

export pid_t getsid(pid_t);
export uid_t getuid(void);
export char *getwd(char *);
export int isatty(int);
export int lchown(const char *, uid_t, gid_t);
export int link(const char *, const char *);
export int lockf(int, int, off_t);
export off_t lseek(int, off_t, int);
export int nice(int);
export long int pathconf(const char *, int);
export int pause(void);
export int pipe(int[2]);
export ssize_t pread(int, void *, size_t, off_t);
export int pthread_atfork(void (*)(void), void (*)(void), void (*)(void));
export ssize_t pwrite(int, const void *, size_t, off_t);
export ssize_t read(int, void *, size_t);
export int readlink(const char *, char *, size_t);
export int rmdir(const char *);
export void *sbrk(intptr_t);
export int setgid(gid_t);
export int setpgid(pid_t, pid_t);
export pid_t setpgrp(void);
export int setregid(gid_t, gid_t);
export int setreuid(uid_t, uid_t);
export pid_t setsid(void);
export int setuid(uid_t);
export unsigned int sleep(unsigned int);
export void swab(const void *, void *, ssize_t);
export int symlink(const char *, const char *);
export void sync(void);
export long int sysconf(int);
export pid_t tcgetpgrp(int);
export int tcsetpgrp(int, pid_t);
export int truncate(const char *, off_t);
export char *ttyname(int);
export int ttyname_r(int, char *, size_t);
export useconds_t ualarm(useconds_t, useconds_t);
export int unlink(const char *);
export int usleep(useconds_t);
export pid_t vfork(void);
export ssize_t write(int, const void *, size_t);
