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
	__vmi_class_type_info::~__vmi_class_type_info()
	{
	}

	void *__vmi_class_type_info::cast_to(void *obj, const struct __class_type_info *other) const
	{
		if (__do_upcast(other, &obj))
			return obj;
		return 0;
	}

	bool __vmi_class_type_info::__do_upcast(const __class_type_info *target, void **thrown_object) const
	{
		if (this == target)
			return true;

		stub;
		return 0;
	}
}
