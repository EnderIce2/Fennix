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

#include <cxxabi.h>

#include <memory.hpp>
#include <debug.h>
#include <smp.hpp>

#include "../../kernel.h"

void __dso_handle_stub() { stub; }

/* extern */ void *__dso_handle = (void *)&__dso_handle_stub;

namespace __cxxabiv1
{
	atexit_func_entry_t __atexit_funcs[ATEXIT_MAX_FUNCS];
	uarch_t __atexit_func_count = 0;

	__cxa_eh_globals *__cxa_get_globals() noexcept
	{
		return &GetCurrentCPU()->EHGlobals;
	}

	/**
	 * @param f The destructor
	 * @param objptr The object to be destructed
	 * @param dso The DSO from which the object was obtained (unused in our case)
	 * @return Zero on success, non-zero on failure
	 */
	extern "C" int __cxa_atexit(void (*f)(void *), void *objptr, void *dso)
	{
		if (KernelSymbolTable)
		{
			debug("Registering atexit function for \"%s\" with destructor \"%s\"",
				  KernelSymbolTable->GetSymbolFromAddress((uintptr_t)objptr),
				  KernelSymbolTable->GetSymbolFromAddress((uintptr_t)f));
		}
		else
		{
			debug("Registering atexit function for %p with destructor %p",
				  objptr, f);
		}

		if (__atexit_func_count >= ATEXIT_MAX_FUNCS)
			return -1;
		__atexit_funcs[__atexit_func_count].destructor_func = f;
		__atexit_funcs[__atexit_func_count].obj_ptr = objptr;
		__atexit_funcs[__atexit_func_count].dso_handle = dso;
		__atexit_func_count++;
		return 0;
	}

	extern "C" void __cxa_finalize(void *f)
	{
		function("%p", f);
		uarch_t i = __atexit_func_count;
		if (f == nullptr)
		{
			while (i--)
			{
				if (__atexit_funcs[i].destructor_func)
				{
					if (KernelSymbolTable)
					{
						debug("Calling atexit function \"%s\"",
							  KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__atexit_funcs[i].destructor_func));
					}
					else
					{
						debug("Calling atexit function %p",
							  __atexit_funcs[i].destructor_func);
					}
					(*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
				}
			}
			return;
		}

		while (i--)
		{
			if (__atexit_funcs[i].destructor_func == f)
			{
				(*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
				__atexit_funcs[i].destructor_func = 0;
			}
		}
	}

	extern "C" _Unwind_Reason_Code __gxx_personality_v0(int version, _Unwind_Action actions, _Unwind_Exception_Class exception_class, _Unwind_Exception *ue_header, _Unwind_Context *context)
	{
		fixme("__gxx_personality_v0( %d %p %p %p %p ) called.", version, actions, exception_class, ue_header, context);
		return _URC_NO_REASON;
	}

	extern "C" void *__cxa_begin_catch(void *thrown_object) noexcept
	{
		function("%p", thrown_object);

		__cxa_exception *Exception = (__cxa_exception *)thrown_object - 1;
		__cxa_eh_globals *Globals = __cxa_get_globals();

		Exception->handlerCount++;
		Globals->uncaughtExceptions--;

		Exception->nextException = Globals->caughtExceptions;
		Globals->caughtExceptions = Exception;
		return Exception + 1;
	}

	extern "C" void __cxa_end_catch()
	{
		fixme("__cxa_end_catch() called.");
	}

	static __always_inline inline size_t align_exception_allocation_size(size_t a, size_t b)
	{
		return (a + b - 1) & ~(b - 1);
	}

	static __always_inline inline void INIT_EXCEPTION_CLASS(_Unwind_Exception_Class *c)
	{
		char *ptr = (char *)c;
		ptr[0] = 'G';
		ptr[1] = 'N';
		ptr[2] = 'U';
		ptr[3] = 'C';
		ptr[4] = 'C';
		ptr[5] = '+';
		ptr[6] = '+';
		ptr[7] = '\0';
	}

	void unexpected_header_stub() { fixme("unexpected() called."); }

	void terminate_header_stub()
	{
		if (TaskManager && !TaskManager->IsPanic())
		{
			TaskManager->KillThread(thisThread, Tasking::KILL_CXXABI_EXCEPTION);
			TaskManager->Yield();
		}

		error("No task manager to kill thread!");
		CPU::Stop(); /* FIXME: Panic */
	}

	void exception_cleanup_stub(_Unwind_Reason_Code Code,
								_Unwind_Exception *Exception)
	{
		fixme("exception_cleanup( %d %p ) called.",
			  Code, Exception);
	}

	extern "C" void *__cxa_allocate_exception(size_t thrown_size) throw()
	{
		debug("Allocating exception of size %d.", thrown_size);

		size_t alloc_size = align_exception_allocation_size(thrown_size + sizeof(__cxa_exception), alignof(__cxa_exception));
		__cxa_exception *Exception = (__cxa_exception *)kmalloc(alloc_size);
		memset(Exception, 0, alloc_size);
		return Exception + 1;
	}

	extern "C" __noreturn void __cxa_throw(void *thrown_object,
										   std::type_info *tinfo,
										   void (*dest)(void *))
	{
		trace("Throwing exception of type \"%s\". ( object: %p, destructor: %p )",
			  tinfo->name(), thrown_object, dest);

		__cxa_eh_globals *Globals = __cxa_get_globals();
		Globals->uncaughtExceptions++;

		__cxa_exception *Exception = (__cxa_exception *)thrown_object - 1;
		Exception->exceptionType = (std::type_info *)tinfo;
		Exception->exceptionDestructor = dest;
		Exception->unexpectedHandler = &unexpected_header_stub;
		Exception->terminateHandler = &terminate_header_stub;
		Exception->unwindHeader.exception_cleanup = &exception_cleanup_stub;
		INIT_EXCEPTION_CLASS(&Exception->unwindHeader.exception_class);
		Exception->adjustedPtr = thrown_object;

		_Unwind_RaiseException(&Exception->unwindHeader);
		__cxa_begin_catch(&Exception->unwindHeader);

		error("Uncaught exception!");
		CPU::Stop(); /* FIXME: Panic */
	}

	extern "C" void __cxa_rethrow()
	{
		fixme("__cxa_rethrow() called.");
	}

	extern "C" void __cxa_pure_virtual()
	{
		fixme("__cxa_pure_virtual() called.");
	}

	extern "C" void __cxa_throw_bad_array_new_length()
	{
		fixme("__cxa_throw_bad_array_new_length() called.");
	}

	extern "C" void __cxa_free_exception(void *thrown_exception)
	{
		fixme("__cxa_free_exception( %p ) called.",
			  thrown_exception);
	}

	__extension__ typedef int __guard __attribute__((mode(__DI__)));

	extern "C" int __cxa_guard_acquire(__guard *g)
	{
		fixme("__cxa_guard_acquire( %p ) called.", g);
		return !*(char *)(g);
	}

	extern "C" void __cxa_guard_release(__guard *g)
	{
		fixme("__cxa_guard_release( %p ) called.", g);
		*(char *)g = 1;
	}

	extern "C" void __cxa_guard_abort(__guard *g)
	{
		fixme("__cxa_guard_abort( %p ) called.", g);
	}

	extern "C" __noreturn void __cxa_bad_typeid()
	{
		fixme("__cxa_bad_typeid() called.");
		CPU::Stop(); /* FIXME: Crash the system */
	}
}
