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
#include <memory.hpp>

namespace UniversalSerialBus
{
	USBSchedulerPool *Scheduler::CreateNewPool(size_t SizeOfDescriptor, int RequredAlignment, size_t Count, off_t Breadth, off_t Depth, off_t Software)
	{
		assert(SizeOfDescriptor - Software >= sizeof(void *));
		SchedPoolInternal pool;
		pool.Lock = fnx::spinlock_t();
		pool.SizeOfDescriptor = ROUND_UP(SizeOfDescriptor, RequredAlignment);
		pool.Count = Count;
		pool.Breadth = Breadth;
		pool.Depth = Depth;
		pool.Software = Software;
		pool.Data = (uint8_t *)KernelAllocator.RequestPages(TO_PAGES(pool.SizeOfDescriptor * pool.Count));

		assert((uintptr_t)pool.Data < 0x80000000);
		Memory::Virtual().Map(pool.Data, pool.Data, TO_PAGES(pool.SizeOfDescriptor * pool.Count), Memory::P | Memory::RW | Memory::PWT | Memory::PCD);
		memset(pool.Data, 0, pool.SizeOfDescriptor * pool.Count);

		return &Pools.emplace_back(pool);
	}

	int Scheduler::GetCurrentFrame()
	{
		stub;
		return 0;
	}

	bool Scheduler::Initialize(uint32_t FrameListData)
	{
		if (Pools.size() == 0)
			return false;

		for (auto &&pool : this->Pools)
		{
			// pool.GetSoftware(0);
			// TODO: do smth like link them or idk
		}
		stub;

		for (size_t i = 0; i < this->Frames; i++)
			this->FrameList[i] = FrameListData;
		return true;
	}

	Scheduler::Scheduler(size_t NumOfFrames, size_t NumOfSubFrames, size_t MaxBandwidth)
	{
		this->Frames = NumOfFrames;
		this->SubFrames = NumOfSubFrames;
		this->BandwidthSize = MaxBandwidth;

		size_t flBytes = sizeof(uint32_t) * Frames;
		this->FrameList = (uint32_t *)KernelAllocator.RequestPages(TO_PAGES(flBytes));

		assert((uintptr_t)this->FrameList < 0x80000000);
		Memory::Virtual().Map(this->FrameList, this->FrameList, TO_PAGES(flBytes), Memory::P | Memory::RW | Memory::PWT | Memory::PCD);
		memset(this->FrameList, 0, flBytes);
	}

	Scheduler::~Scheduler()
	{
		KernelAllocator.FreePages(this->FrameList, TO_PAGES(sizeof(uint32_t) * Frames));
	}
}
