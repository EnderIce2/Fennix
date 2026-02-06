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

#include "uhci.hpp"

extern Driver::Manager *DriverManager;

namespace Driver::UniversalHostControllerInterface
{
	extern dev_t DriverID;

	void TransferQueue::AdvanceFrame()
	{
		uint16_t frnum = inw(base + REG_FRNUM);
		CurrentFrame += (frnum - CurrentFrame) & 0x7FF;
	}

	void TransferQueue::ProcessComplete()
	{
		this->AdvanceFrame();
		for (size_t i = 0; i < TDCount; i++)
		{
			if (!TDUsed[i])
				continue;

			TD *td = &TDPool[i];

			if (td->CS.Status_ACTIVE())
				continue;

			USBRequestBlock *urb = td->URB;

			assert(urb != nullptr);
			urb->ActualLength += td->CS.ActualLength();

			if (td->CS.Status_STALLED() || td->CS.Status_CRC_TO_ERROR() || td->CS.Status_DATA_BUFFER_ERROR())
			{
				urb->Status = USB_REQ_ERROR;
			}

			if (td->CS.InterruptOnComplete())
			{
				if (urb->Complete)
					urb->Complete(urb);
			}

			ReleaseTransferDescriptor(td);
		}
	}

	void TransferQueue::EnqueueTD(TD *td)
	{
		if (AsyncQH->ELEMENT.Terminate())
		{
			AsyncQH->ELEMENT.QELP((uintptr_t)td);
			AsyncQH->ELEMENT.QHTDSelect(0);
			AsyncQH->ELEMENT.Terminate(0);
		}
		else
		{
			TD *cur = (TD *)(uintptr_t)AsyncQH->ELEMENT.QELP();
			while (!cur->LINK.Terminate())
				cur = (TD *)(uintptr_t)cur->LINK.LinkPointer();

			cur->LINK.LinkPointer((uintptr_t)td);
			cur->LINK.QHTDSelect(0);
			cur->LINK.Terminate(0);
		}
	}

	QH *TransferQueue::AllocateQueueHead()
	{
		for (size_t i = 0; i < QHCount; i++)
		{
			if (!QHUsed[i])
			{
				QHUsed[i] = true;
				QH *qh = &QHPool[i];
				memset(qh, 0, sizeof(QH));
				debug("Allocated QH at index %zu (address %#lx)", i, qh);
				return qh;
			}
		}
		return nullptr;
	}

	int TransferQueue::ReleaseQueueHead(QH *qh)
	{
		qh->HEAD = 1;
		qh->ELEMENT = 1;
		// qh->HEAD.Terminate(1);
		// qh->ELEMENT.Terminate(1);
		size_t idx = qh - QHPool;
		QHUsed[idx] = false;
		debug("Released QH at index %zu (address %#lx)", idx, qh);
		return 0;
	}

	TD *TransferQueue::AllocateTransferDescriptor()
	{
		for (size_t i = 0; i < TDCount; i++)
		{
			if (!TDUsed[i])
			{
				TDUsed[i] = true;
				TD *td = &TDPool[i];
				memset(td, 0, sizeof(TD));
				debug("Allocated TD at index %zu (address %#lx)", i, td);
				return td;
			}
		}
		return nullptr;
	}

	int TransferQueue::ReleaseTransferDescriptor(TD *td)
	{
		// td->LINK.Terminate(1);
		td->LINK = 1;
		size_t idx = td - TDPool;
		TDUsed[idx] = false;
		debug("Released TD at index %zu (address %#lx)", idx, td);
		return 0;
	}

	TransferQueue::TransferQueue(uint16_t io) : base(io)
	{
		FrameList = (FrameListPointer *)KernelAllocator.RequestPages(TO_PAGES(1024 * sizeof(uint32_t)));
		memset(FrameList, 0, 1024 * sizeof(uint32_t));
		debug("FrameList at %#lx-%#lx", FrameList, (uintptr_t)FrameList + 1024 * sizeof(uint32_t));

		memset(QHUsed, 0, sizeof(QHUsed));
		memset(TDUsed, 0, sizeof(TDUsed));

		QHPool = (QH *)KernelAllocator.RequestPages(TO_PAGES(sizeof(QH) * 16));
		TDPool = (TD *)KernelAllocator.RequestPages(TO_PAGES(sizeof(TD) * 64));
		memset(QHPool, 0, sizeof(QH) * 16);
		memset(TDPool, 0, sizeof(TD) * 64);
		debug("QHPool at %#lx-%#lx", QHPool, (uintptr_t)QHPool + sizeof(QH) * 16);
		debug("TDPool at %#lx-%#lx", TDPool, (uintptr_t)TDPool + sizeof(TD) * 64);

		for (int i = 0; i < 9; i++)
		{
			QHUsed[i] = true;
			InterruptQH[i] = &QHPool[i];
			InterruptQH[i]->HEAD = 0;
			InterruptQH[i]->HEAD.Terminate(1);
			InterruptQH[i]->ELEMENT = 0;
			InterruptQH[i]->ELEMENT.Terminate(1);
			debug("InterruptQH[%d] at %#lx", i, InterruptQH[i]);
		}

		AsyncQH = &QHPool[9];
		AsyncQH->HEAD = 0;
		AsyncQH->HEAD.Terminate(1);
		AsyncQH->ELEMENT = 0;
		AsyncQH->ELEMENT.Terminate(1);
		debug("AsyncQH at %#lx", AsyncQH);

		for (int i = 8; i > 0; i--)
		{
			InterruptQH[i]->HEAD.Terminate(0);
			InterruptQH[i]->HEAD.QHTDSelect(1);
			InterruptQH[i]->HEAD.QueueHeadLinkPointer((uintptr_t)InterruptQH[i - 1]);
		}

		QH *iqh0 = InterruptQH[0];
		iqh0->HEAD.Terminate(0);
		iqh0->HEAD.QHTDSelect(1);
		iqh0->HEAD.QueueHeadLinkPointer((uintptr_t)AsyncQH);

		for (int i = 0; i < 1024; i++)
		{
			FrameList[i].QHTDSelect(1);

			if (i < 2)
				FrameList[i].FLP((uintptr_t)InterruptQH[8]);
			else if (i < 4)
				FrameList[i].FLP((uintptr_t)InterruptQH[7]);
			else if (i < 8)
				FrameList[i].FLP((uintptr_t)InterruptQH[6]);
			else if (i < 16)
				FrameList[i].FLP((uintptr_t)InterruptQH[5]);
			else if (i < 32)
				FrameList[i].FLP((uintptr_t)InterruptQH[4]);
			else if (i < 64)
				FrameList[i].FLP((uintptr_t)InterruptQH[3]);
			else if (i < 128)
				FrameList[i].FLP((uintptr_t)InterruptQH[2]);
			else if (i < 256)
				FrameList[i].FLP((uintptr_t)InterruptQH[1]);
			else
				FrameList[i].FLP((uintptr_t)AsyncQH);
		}
	}

	TransferQueue::~TransferQueue()
	{
	}
}
