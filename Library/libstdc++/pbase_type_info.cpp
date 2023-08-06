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
	__pbase_type_info::~__pbase_type_info() {}

	bool __pbase_type_info::__do_catch(const type_info *ThrowType,
									   void **ThrowObject,
									   unsigned outer) const
	{
#ifndef __GXX_RTTI
		UNUSED(ThrowType);
		UNUSED(ThrowObject);
		UNUSED(outer);
		return false;
#else
		if (*this == *ThrowType)
			return true;

		if (*ThrowType == typeid(nullptr))
		{
			if (typeid(*this) == typeid(__pointer_type_info))
			{
				*ThrowObject = nullptr;
				return true;
			}
			else if (typeid(*this) == typeid(__pointer_to_member_type_info))
			{
				if (this->Pointee->__is_function_p())
				{
					using pmf_type = void (__pbase_type_info::*)();
					static const pmf_type pmf = nullptr;
					*ThrowObject = const_cast<pmf_type *>(&pmf);
					return true;
				}
				else
				{
					using pm_type = int __pbase_type_info::*;
					static const pm_type pm = nullptr;
					*ThrowObject = const_cast<pm_type *>(&pm);
					return true;
				}
			}
		}

		if (typeid(*this) != typeid(*ThrowType))
			return false;

		if (!(outer & 1))
			return false;

		const __pbase_type_info *ThrownType =
			static_cast<const __pbase_type_info *>(ThrowType);

		unsigned TypeFlags = ThrownType->Flags;

		const unsigned FlagQualificationMask = __transaction_safe_mask | __noexcept_mask;
		unsigned ThrowFlagQualification = (TypeFlags & FlagQualificationMask);
		unsigned CatchFlagQualification = (Flags & FlagQualificationMask);
		if (ThrowFlagQualification & ~CatchFlagQualification)
			TypeFlags &= CatchFlagQualification;
		if (CatchFlagQualification & ~ThrowFlagQualification)
			return false;

		if (TypeFlags & ~Flags)
			return false;

		if (!(Flags & __const_mask))
			outer &= ~1;

		return __pointer_catch(ThrownType, ThrowObject, outer);
#endif
	}
}
