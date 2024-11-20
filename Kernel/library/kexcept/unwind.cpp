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

#include <kexcept/unwind.h>
#include <kexcept/cxxabi.h>
#include <debug.h>
#include <cpu.hpp>
#include <smp.hpp>

#include "../../kernel.h"

using namespace __cxxabiv1;

#if (1) /* Set to 0 to use a library or 1 to use this code */

extern "C" _Unwind_Reason_Code _Unwind_RaiseException(struct _Unwind_Exception *object)
{
	stub;
	return _URC_NO_REASON;
}

extern "C" void _Unwind_Resume(struct _Unwind_Exception *object)
{
	stub;
}

extern "C" _Unwind_Reason_Code _Unwind_Resume_or_Rethrow(struct _Unwind_Exception *exception_object)
{
	stub;
	return _URC_NO_REASON;
}

#endif // 0 or 1
