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

#ifndef __FENNIX_KERNEL_MEMORY_TEMP_PAGE_TABLE_H__
#define __FENNIX_KERNEL_MEMORY_TEMP_PAGE_TABLE_H__

#include <types.h>

#include <memory/table.hpp>

extern Memory::PageTable *KernelPageTable;

namespace Memory
{
	class SwapPT
	{
	private:
		PageTable *Replace = nullptr;
		PageTable *Restore = nullptr;
		bool Ignore;

	public:
		SwapPT(auto ReplaceWith,
			   auto _RestoreWith = nullptr)
			: Replace((PageTable *)ReplaceWith)
		{
			PageTable *RestoreWith = (PageTable *)_RestoreWith;

			this->Ignore = Replace == RestoreWith;
			if (this->Ignore)
				return;

			if (RestoreWith)
				Restore = RestoreWith;
			else
				Restore = KernelPageTable;

			Replace->Update();
		}

		~SwapPT()
		{
			if (this->Ignore)
				return;

			Restore->Update();
		}
	};
}

#endif // !__FENNIX_KERNEL_MEMORY_TEMP_PAGE_TABLE_H__
