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

#ifndef __FENNIX_KERNEL_CXXABI_H__
#define __FENNIX_KERNEL_CXXABI_H__

#include <types.h>
#include <unwind.h>
#include <typeinfo>

namespace __cxxabiv1
{
#define ATEXIT_MAX_FUNCS 128

	typedef unsigned uarch_t;

	struct atexit_func_entry_t
	{
		void (*destructor_func)(void *);
		void *obj_ptr;
		void *dso_handle;
	};

	struct __cxa_exception
	{
		std::type_info *exceptionType;
		void (*exceptionDestructor)(void *);
		std::terminate_handler unexpectedHandler;
		std::terminate_handler terminateHandler;
		__cxa_exception *nextException;
		int handlerCount;
#ifdef __ARM_EABI_UNWINDER__
		__cxa_exception *nextPropagatingException;
		int propagationCount;
#else
		int handlerSwitchValue;
		const unsigned char *actionRecord;
		const unsigned char *languageSpecificData;
		_Unwind_Ptr catchTemp;
		void *adjustedPtr;
#endif
		_Unwind_Exception unwindHeader;
	};
}

struct __cxa_eh_globals
{
	__cxxabiv1::__cxa_exception *caughtExceptions;
	unsigned int uncaughtExceptions;
#ifdef __ARM_EABI_UNWINDER__
	__cxxabiv1::__cxa_exception *propagatingExceptions;
#endif
};

#endif // !__FENNIX_KERNEL_CXXABI_H__
