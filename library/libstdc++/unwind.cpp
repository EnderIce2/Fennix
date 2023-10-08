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

#include <unwind.h>
#include <cxxabi.h>
#include <debug.h>
#include <cpu.hpp>
#include <smp.hpp>

#include "../../kernel.h"

using namespace __cxxabiv1;

#if (1) /* Stubs if libgcc is not present */
extern "C" _Unwind_Reason_Code _Unwind_RaiseException(_Unwind_Exception *Exception)
{
	fixme("_Unwind_RaiseException( %p ) called.", Exception);
	error("Unhandled exception.");
	return _URC_FATAL_PHASE1_ERROR;
	// return _URC_NO_REASON;
}

extern "C" void _Unwind_Resume(struct _Unwind_Exception *Exception)
{
	fixme("_Unwind_Resume( %p ) called.", Exception);
}
#endif
