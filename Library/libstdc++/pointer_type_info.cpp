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

#include <typeinfo>

namespace __cxxabiv1
{
	__pointer_type_info::~__pointer_type_info() {}

	bool __pointer_type_info::__is_pointer_p() const { return true; }

	bool __pointer_type_info::__pointer_catch(const __pbase_type_info *ThrownType,
											  void **ThrowObject,
											  unsigned Outer) const
	{
#ifndef __GXX_RTTI
		UNUSED(ThrownType);
		UNUSED(ThrowObject);
		UNUSED(Outer);
		return false;
#else
		if (Outer < 2 && *this->Pointee == typeid(void))
			return !ThrownType->Pointee->__is_function_p();

		return __pbase_type_info::__pointer_catch(ThrownType,
												  ThrowObject,
												  Outer);
#endif
	}

}
