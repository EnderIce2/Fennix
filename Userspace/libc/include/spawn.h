#ifndef _SPAWN_H
#define _SPAWN_H

#include <stddef.h>
#include <bits/types/struct_sched_param.h>
#include <bits/types/sigset_t.h>
#include <sys/types.h>

struct __spawn_action
{
	int __stub;
};

typedef struct
{
	short int __flags;
	pid_t __pgrp;
	sigset_t __sd;
	sigset_t __ss;
	struct sched_param __sp;
	int __policy;
	int __pad[16];
} posix_spawnattr_t;

typedef struct
{
	int __allocated;
	int __used;
	struct __spawn_action *__actions;
	int __pad[16];
} posix_spawn_file_actions_t;

int posix_spawn(pid_t *pid, const char *path,
				const posix_spawn_file_actions_t *file_actions,
				const posix_spawnattr_t *attrp,
				char *const argv[],
				char *const envp[]);

int posix_spawnp(pid_t *pid, const char *file,
				 const posix_spawn_file_actions_t *file_actions,
				 const posix_spawnattr_t *attrp,
				 char *const argv[],
				 char *const envp[]);

#endif // !_SPAWN_H
