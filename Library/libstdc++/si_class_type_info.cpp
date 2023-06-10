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
	template <typename T>
	inline const T *adjust_pointer(const void *Base,
								   ptrdiff_t Offset)
	{
		return reinterpret_cast<const T *>(reinterpret_cast<const char *>(Base) + Offset);
	}

	__si_class_type_info::~__si_class_type_info() {}

	__class_type_info::__sub_kind __si_class_type_info::
		__do_find_public_src(ptrdiff_t SourceToDestination,
							 const void *ObjectPointer,
							 const __class_type_info *SourceType,
							 const void *SourcePointer) const
	{
		if (SourcePointer == ObjectPointer && *this == *SourceType)
			return __contained_public;

		return this->BaseType->__do_find_public_src(SourceToDestination,
													ObjectPointer,
													SourceType,
													SourcePointer);
	}

	bool __si_class_type_info::__do_dyncast(ptrdiff_t SourceToDestination,
											__sub_kind AccessPath,
											const __class_type_info *DestinationType,
											const void *ObjectPointer,
											const __class_type_info *SourceType,
											const void *SourcePointer,
											__dyncast_result &__restrict Result) const
	{
		if (*this == *DestinationType)
		{
			Result.dst_ptr = ObjectPointer;
			Result.whole2dst = AccessPath;

			if (SourceToDestination == -2)
			{
				Result.dst2src = __not_contained;
				return false;
			}

			if (SourceToDestination >= 0)
			{
				Result.dst2src = adjust_pointer<void>(ObjectPointer,
													  SourceToDestination) == SourcePointer
									 ? __contained_public
									 : __not_contained;
			}

			return false;
		}

		if (ObjectPointer == SourcePointer &&
			*this == *SourceType)
		{
			Result.whole2src = AccessPath;
			return false;
		}

		return this->BaseType->__do_dyncast(SourceToDestination,
											AccessPath,
											DestinationType,
											ObjectPointer,
											SourceType,
											SourcePointer,
											Result);
	}

	bool __si_class_type_info::__do_upcast(const __class_type_info *Destination,
										   const void *ObjectPointer,
										   __upcast_result &__restrict Result) const
	{
		if (__class_type_info::__do_upcast(Destination, ObjectPointer, Result))
			return true;

		return this->BaseType->__do_upcast(Destination, ObjectPointer, Result);
	}
}
