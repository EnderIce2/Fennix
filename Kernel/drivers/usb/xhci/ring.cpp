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

#include "xhci.hpp"

#include <mutex>

namespace Driver::ExtensibleHostControllerInterface
{
	extern dev_t DriverID;

	void HCD::CommandRing::QueueTRB(TRB *trb)
	{
		trb->Control.CycleBit(RingCycleState);
		auto buf = &Buffer[EnqueuePtr++];
		memcpy(buf, trb, sizeof(TRB));
		if (EnqueuePtr >= MaxTRBCount - 1)
			RingCycleState = !RingCycleState;
	}

	CommandCompletionEventTRB *HCD::CommandRing::_EnqueueTRB(TRB *trb)
	{
		// std::mutex lock;
		// lock.lock();
		trb->Control.CycleBit(RingCycleState);
		Buffer[EnqueuePtr] = *trb;

		if (++EnqueuePtr == MaxTRBCount - 1)
		{
			LinkTRB *link = (LinkTRB *)&Buffer[MaxTRBCount - 1];
			link->RingSegmentPointer((uint64_t)Buffer);
			link->Status.InterrupterTarget(0);
			link->Control.CycleBit(RingCycleState);
			link->Control.ToggleCycle(1);
			link->Control.ChainBit(0);
			link->Control.InterruptOnCompletion(1);
			link->Control.TRBType(TRBT_Link);

			EnqueuePtr = 0;
			RingCycleState = !RingCycleState;
		}
		return nullptr;
	}

	HCD::CommandRing::CommandRing(HCD &Controller, size_t Count) : hc(Controller), MaxTRBCount(Count)
	{
		const size_t ringSize = MaxTRBCount * sizeof(TRB);
		Buffer = (TRB *)v0::AllocateMemory(DriverID, TO_PAGES(ringSize));
		memset(Buffer, 0, ringSize);
		Memory::Virtual(KernelPageTable).Map(Buffer, Buffer, ringSize, Memory::P | Memory::RW | Memory::PCD | Memory::PWT);

		LinkTRB *link = (LinkTRB *)&Buffer[MaxTRBCount - 1];
		link->RingSegmentPointer((uint64_t)Buffer);
		link->Status.InterrupterTarget(0);
		link->Control.CycleBit(RingCycleState);
		link->Control.ToggleCycle(1);
		link->Control.ChainBit(0);
		link->Control.InterruptOnCompletion(1);
		link->Control.TRBType(TRBT_Link);
	}

	HCD::CommandRing::~CommandRing()
	{
	}

	void EventRing::UpdateERDP()
	{
		uintptr_t ptr = (uintptr_t)Segments[0].trb + (DequeuePtr * sizeof(TRB));
		Interrupter->ERDP.EventRingDequeuePointer(ptr);
	}

	TRB *EventRing::DequeueTRB()
	{
		if (Segments[0].trb[DequeuePtr].Control.CycleBit() != RingCycleState)
		{
			error("Ring cycle state mismatch!");
			return nullptr;
		}

		auto trb = &Segments[0].trb[DequeuePtr];
		if (++DequeuePtr == Segments[0].Size)
		{
			DequeuePtr = 0;
			RingCycleState = !RingCycleState;
		}
		return trb;
	}

	bool EventRing::HasUnprocessedEvents()
	{
		return Segments[0].trb[DequeuePtr].Control.CycleBit() == RingCycleState;
	}

	void EventRing::DequeueEvents(std::vector<TRB *> &TRBs)
	{
		while (HasUnprocessedEvents())
		{
			auto trb = DequeueTRB();
			if (trb == nullptr)
				break;
			TRBs.push_back(trb);
		}

		UpdateERDP();
		Interrupter->ERDP.EventHandlerBusy(1);
	}

	void EventRing::FlushUnprocessedEvents()
	{
		std::vector<TRB *> TRBs;
		DequeueEvents(TRBs);
	}

	EventRing::EventRing(size_t c, XHCIinterrupter *i) : Interrupter(i), TableSize(c)
	{
		assert(c == 1 && "Unimplemented");
		Table = (EventRingSegmentTableEntry *)v0::AllocateMemory(DriverID, TO_PAGES(TableSize * sizeof(EventRingSegmentTableEntry)));
		Memory::Virtual(KernelPageTable).Map(Table, Table, PAGE_SIZE, Memory::P | Memory::RW | Memory::PCD | Memory::PWT);
		memset(Table, 0, PAGE_SIZE);

		for (size_t i = 0; i < TableSize; i++)
		{
			Segment e{
				.trb = (TRB *)v0::AllocateMemory(DriverID, 1),
				.Size = (PAGE_SIZE / sizeof(TRB)),
			};

			Memory::Virtual(KernelPageTable).Map(e.trb, e.trb, PAGE_SIZE, Memory::P | Memory::RW | Memory::PCD | Memory::PWT);
			memset(e.trb, 0, PAGE_SIZE);

			Table[i].RSBA.RingSegmentBaseAddress((uint64_t)e.trb);
			Table[i].RSS.RingSegmentSize(e.Size);
			Segments.push_back(e);
		}

		UpdateERDP();
		Interrupter->ERSTSZ.EventRingSegmentTableSize(TableSize);
		Interrupter->ERSTBA.EventRingSegmentTableBaseAddress((uint64_t)Table);
		Interrupter->ERDP.EventRingDequeuePointer((uint64_t)Segments[0].trb);
	}

	EventRing::~EventRing()
	{
	}
}
