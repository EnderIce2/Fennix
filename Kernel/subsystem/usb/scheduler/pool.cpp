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

#include <usb.hpp>
#include <auto_page>

namespace UniversalSerialBus
{
	void *Scheduler::SchedPoolInternal::GetBreadth(off_t Index)
	{
		uint8_t *element = this->Data + (this->SizeOfDescriptor * Index);
		return element + this->Breadth;
	}

	void *Scheduler::SchedPoolInternal::GetDepth(off_t Index)
	{
		uint8_t *element = this->Data + (this->SizeOfDescriptor * Index);
		return element + this->Depth;
	}

	void *Scheduler::SchedPoolInternal::GetSoftware(off_t Index)
	{
		uint8_t *element = this->Data + (this->SizeOfDescriptor * Index);
		return element + this->Software;
	}

	int Scheduler::GetPoolElement(USBSchedulerPool *Pool, size_t Index, void **Element)
	{
		auto pool = (SchedPoolInternal *)Pool;
		*Element = pool->Data + (pool->SizeOfDescriptor * Index);
		return 0;
	}

	// int Scheduler::GetPoolElement(size_t PoolIndex, size_t Index, void **Element)
	// {
	// 	return 0;
	// }
}
