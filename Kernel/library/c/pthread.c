/*
	This file is part of Fennix Kernel.

	Fennix Kernel is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Kernel is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#include <pthread.h>

#include <assert.h>

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
	assert(!"pthread_key_create not implemented yet");
}

int pthread_key_delete(pthread_key_t key)
{
	assert(!"pthread_key_delete not implemented yet");
}

int pthread_setspecific(pthread_key_t key, const void *pointer)
{
	assert(!"pthread_setspecific not implemented yet");
}

void *pthread_getspecific(pthread_key_t key)
{
	assert(!"pthread_getspecific not implemented yet");
}

struct __emutls_object
{
	__SIZE_TYPE__ size;
	__SIZE_TYPE__ align;
	void *default_v;
	void *templ;
};

void *__emutls_get_address(struct __emutls_object *obj)
{
	assert(!"__emutls_get_address not implemented yet");
}
