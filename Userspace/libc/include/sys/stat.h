#ifndef _SYS_STAT_H
#define _SYS_STAT_H

typedef unsigned int __dev_t;
typedef unsigned short __ino_t;
typedef unsigned short __mode_t;
typedef long __off_t;

#define dev_t __dev_t
#define ino_t __ino_t
#define mode_t __mode_t
#define off_t __off_t

int mkdir(const char *path, mode_t mode);
int remove(const char *pathname);
int rename(const char *oldpath, const char *newpath);

#endif
