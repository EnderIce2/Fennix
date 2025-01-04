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

#include <pthread.h>
#include <errno.h>

export int pthread_attr_destroy(pthread_attr_t *);
export int pthread_attr_getdetachstate(const pthread_attr_t *, int *);
export int pthread_attr_getguardsize(const pthread_attr_t *, size_t *);
export int pthread_attr_getinheritsched(const pthread_attr_t *, int *);
export int pthread_attr_getschedparam(const pthread_attr_t *, struct sched_param *);
export int pthread_attr_getschedpolicy(const pthread_attr_t *, int *);
export int pthread_attr_getscope(const pthread_attr_t *, int *);
export int pthread_attr_getstackaddr(const pthread_attr_t *, void **);
export int pthread_attr_getstacksize(const pthread_attr_t *, size_t *);
export int pthread_attr_init(pthread_attr_t *);
export int pthread_attr_setdetachstate(pthread_attr_t *, int);
export int pthread_attr_setguardsize(pthread_attr_t *, size_t);
export int pthread_attr_setinheritsched(pthread_attr_t *, int);
export int pthread_attr_setschedparam(pthread_attr_t *, const struct sched_param *);
export int pthread_attr_setschedpolicy(pthread_attr_t *, int);
export int pthread_attr_setscope(pthread_attr_t *, int);
export int pthread_attr_setstackaddr(pthread_attr_t *, void *);
export int pthread_attr_setstacksize(pthread_attr_t *, size_t);
export int pthread_cancel(pthread_t);
export void pthread_cleanup_push(void (*)(void *), void *);
export void pthread_cleanup_pop(int);
export int pthread_cond_broadcast(pthread_cond_t *);
export int pthread_cond_destroy(pthread_cond_t *);
export int pthread_cond_init(pthread_cond_t *, const pthread_condattr_t *);
export int pthread_cond_signal(pthread_cond_t *);
export int pthread_cond_timedwait(pthread_cond_t *, pthread_mutex_t *, const struct timespec *);
export int pthread_cond_wait(pthread_cond_t *, pthread_mutex_t *);
export int pthread_condattr_destroy(pthread_condattr_t *);
export int pthread_condattr_getpshared(const pthread_condattr_t *, int *);
export int pthread_condattr_init(pthread_condattr_t *);
export int pthread_condattr_setpshared(pthread_condattr_t *, int);
export int pthread_create(pthread_t *, const pthread_attr_t *, void *(*)(void *), void *);
export int pthread_detach(pthread_t);
export int pthread_equal(pthread_t, pthread_t);
export void pthread_exit(void *);
export int pthread_getconcurrency(void);
export int pthread_getschedparam(pthread_t, int *, struct sched_param *);
export void *pthread_getspecific(pthread_key_t);
export int pthread_join(pthread_t, void **);
export int pthread_key_create(pthread_key_t *, void (*)(void *));
export int pthread_key_delete(pthread_key_t);
export int pthread_mutex_destroy(pthread_mutex_t *);
export int pthread_mutex_getprioceiling(const pthread_mutex_t *, int *);
export int pthread_mutex_init(pthread_mutex_t *, const pthread_mutexattr_t *);

export int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	if (mutex->locked)
		return EBUSY;
	mutex->locked = 1;
	return 0;
}

export int pthread_mutex_setprioceiling(pthread_mutex_t *, int, int *);
export int pthread_mutex_trylock(pthread_mutex_t *);

export int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	if (!mutex->locked)
		return EPERM;
	mutex->locked = 0;
	return 0;
}

export int pthread_mutexattr_destroy(pthread_mutexattr_t *);
export int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *, int *);
export int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *, int *);
export int pthread_mutexattr_getpshared(const pthread_mutexattr_t *, int *);
export int pthread_mutexattr_gettype(const pthread_mutexattr_t *, int *);
export int pthread_mutexattr_init(pthread_mutexattr_t *);
export int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *, int);
export int pthread_mutexattr_setprotocol(pthread_mutexattr_t *, int);
export int pthread_mutexattr_setpshared(pthread_mutexattr_t *, int);
export int pthread_mutexattr_settype(pthread_mutexattr_t *, int);
export int pthread_once(pthread_once_t *, void (*)(void));
export int pthread_rwlock_destroy(pthread_rwlock_t *);
export int pthread_rwlock_init(pthread_rwlock_t *, const pthread_rwlockattr_t *);
export int pthread_rwlock_rdlock(pthread_rwlock_t *);
export int pthread_rwlock_tryrdlock(pthread_rwlock_t *);
export int pthread_rwlock_trywrlock(pthread_rwlock_t *);
export int pthread_rwlock_unlock(pthread_rwlock_t *);
export int pthread_rwlock_wrlock(pthread_rwlock_t *);
export int pthread_rwlockattr_destroy(pthread_rwlockattr_t *);
export int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *, int *);
export int pthread_rwlockattr_init(pthread_rwlockattr_t *);
export int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *, int);

export pthread_t pthread_self(void)
{
	pthread_t tid;
	__asm__ __volatile__("mov %%fs:0, %0"
						 : "=r"(tid));
	return tid;
}

export int pthread_setcancelstate(int, int *);
export int pthread_setcanceltype(int, int *);
export int pthread_setconcurrency(int);
export int pthread_setschedparam(pthread_t, int, const struct sched_param *);
export int pthread_setspecific(pthread_key_t, const void *);
export void pthread_testcancel(void);
