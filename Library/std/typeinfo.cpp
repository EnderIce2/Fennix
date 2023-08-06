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
#include <debug.h>

namespace std
{
	type_info::~type_info() {}

	bool type_info::__do_catch(const type_info *ThrowType,
							   void **ThrowObject,
							   unsigned Outer) const
	{
		stub;
		UNUSED(ThrowType);
		UNUSED(ThrowObject);
		UNUSED(Outer);
		return false;
	}

	bool type_info::__do_upcast(const __cxxabiv1::__class_type_info *Target,
								void **ObjectPointer) const
	{
		stub;
		UNUSED(Target);
		UNUSED(ObjectPointer);
		return false;
	}
}
