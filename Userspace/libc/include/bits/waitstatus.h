#ifndef _BITS_WAITSTATUS_H
#define _BITS_WAITSTATUS_H

#define __W_CONTINUED 0xFFFF

#define __WEXITSTATUS(status) (((status)&0xFF00) >> 8)
#define __WTERMSIG(status) ((status)&0x7F)
#define __WSTOPSIG(status) __WEXITSTATUS(status)
#define __WIFEXITED(status) (__WTERMSIG(status) == 0)
#define __WIFSIGNALED(status) (((signed char)(((status)&0x7F) + 1) >> 1) > 0)
#define __WIFSTOPPED(status) (((status)&0xFF) == 0x7F)
#define __WIFCONTINUED(status) ((status) == __W_CONTINUED)

#endif
