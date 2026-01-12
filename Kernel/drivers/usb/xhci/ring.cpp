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

#pragma once

#include "xhci.hpp"

namespace Driver::ExtensibleHostControllerInterface
{
	extern dev_t DriverID;

	HCD::CommandRing::CommandRing(HCD &Controller) : hc(Controller)
	{
		constexpr size_t pages = (256 * sizeof(TRB) + PAGE_SIZE - 1) / PAGE_SIZE;
		Buffer = (TRB *)v0::AllocateMemory(DriverID, pages);
		memset(Buffer, 0, pages * PAGE_SIZE);
		MaxSize = (PAGE_SIZE / sizeof(TRB)) - 1;

		LinkTRB *link = (LinkTRB *)&Buffer[MaxSize];
		link->RingSegmentPointer = (uint64_t)Buffer;
		link->InterrupterTarget(0);
		link->CycleBit(1);
		link->ToggleCycle(1);
		link->ChainBit(0);
		link->InterruptOnCompletion(1);
		link->TRBType(TRBT_Link);
	}

	HCD::CommandRing::~CommandRing()
	{
	}
}
