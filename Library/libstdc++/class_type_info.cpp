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

namespace __cxxabiv1
{
	__class_type_info::~__class_type_info() {}

	bool __class_type_info::__do_upcast(const __class_type_info *Destination,
										const void *Object,
										__upcast_result &__restrict Result) const
	{
		if (*this != *Destination)
			return false;

		Result.dst_ptr = Object;
		Result.base_type = nonvirtual_base_type;
		Result.part2dst = __contained_public;
		return true;
	}

	bool __class_type_info::__do_upcast(const __class_type_info *DestinationType,
										void **ObjectPointer) const
	{
		__upcast_result Result(__vmi_class_type_info::__flags_unknown_mask);

		__do_upcast(DestinationType, *ObjectPointer, Result);

		if (!((Result.part2dst &
			   __class_type_info::__contained_public) ==
			  __class_type_info::__contained_public))
			return false;

		*ObjectPointer = const_cast<void *>(Result.dst_ptr);
		return true;
	}

	bool __class_type_info::__do_catch(const type_info *ThrowType,
									   void **ThrowObject,
									   unsigned Outer) const
	{
		if (*this == *ThrowType)
			return true;

		if (Outer >= 4)
			return false;

		return ThrowType->__do_upcast(this, ThrowObject);
	}

	bool __class_type_info::__do_dyncast(ptrdiff_t,
										 __sub_kind AccessPath,
										 const __class_type_info *DestinationType,
										 const void *ObjectPointer,
										 const __class_type_info *SourceType,
										 const void *SourcePointer,
										 __dyncast_result &__restrict Result) const
	{
		if (ObjectPointer == SourcePointer &&
			*this == *SourceType)
		{
			Result.whole2src = AccessPath;
			return false;
		}

		if (*this == *DestinationType)
		{
			Result.dst_ptr = ObjectPointer;
			Result.whole2dst = AccessPath;
			Result.dst2src = __not_contained;
			return false;
		}

		return false;
	}

	__class_type_info::__sub_kind __class_type_info::__do_find_public_src(ptrdiff_t,
																		  const void *ObjectPointer,
																		  const __class_type_info *,
																		  const void *SourcePointer) const
	{
		if (SourcePointer == ObjectPointer)
			return __contained_public;

		return __not_contained;
	}
}
