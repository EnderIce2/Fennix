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

/* Taken directly from https://github.com/blanham/liballoc/blob/master/linux.c */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>

#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif
#if !defined(MAP_FAILED)
#define MAP_FAILED ((char *)-1)
#endif

#ifndef MAP_NORESERVE
#ifdef MAP_AUTORESRV
#define MAP_NORESERVE MAP_AUTORESRV
#else
#define MAP_NORESERVE 0
#endif
#endif

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int page_size = -1;

int liballoc_lock()
{
	pthread_mutex_lock(&mutex);
	return 0;
}

int liballoc_unlock()
{
	pthread_mutex_unlock(&mutex);
	return 0;
}

void *liballoc_alloc(int pages)
{
	if (page_size < 0)
		page_size = getpagesize();
	unsigned int size = pages * page_size;

	char *p2 = (char *)mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_NORESERVE | MAP_ANONYMOUS, -1, 0);
	if (p2 == MAP_FAILED)
		return NULL;

	if (mprotect(p2, size, PROT_READ | PROT_WRITE) != 0)
	{
		munmap(p2, size);
		return NULL;
	}

	return p2;
}

int liballoc_free(void *ptr, int pages)
{
	return munmap(ptr, pages * page_size);
}
