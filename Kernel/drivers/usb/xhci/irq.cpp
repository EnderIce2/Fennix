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

namespace Driver::ExtensibleHostControllerInterface
{
	extern dev_t DriverID;

	void HCD::OnInterruptReceived(CPU::TrapFrame *Frame)
	{
		debug("xHCI interrupt received!");

		std::vector<TRB *> TRBs;
		if (EvRing->HasUnprocessedEvents())
			EvRing->DequeueEvents(TRBs);

		uint8_t cmdStatus = 0;
		// uint8_t portChange = 0;

		for (size_t i = 0; i < TRBs.size(); i++)
		{
			auto event = TRBs[i];
			switch (event->Control.TRBType())
			{
			case TRBT_CommandCompletionEvent:
				cmdStatus = 1;
				CompletedCmds.push_back((CommandCompletionEventTRB *)event);
				break;
			case TRBT_PortStatusChangeEvent:
				// portChange = 1;
				break;
			default:
				debug("unhandled TRB type: %#lx", event->Control.TRBType());
				break;
			}

			debug("TRB[%zu].Parameter: %#lx", i, TRBs[i]->Parameter);
			debug("TRB[%zu].Status: %#lx", i, TRBs[i]->Status);
			debug("TRB[%zu].Control.CycleBit: %#lx", i, TRBs[i]->Control.CycleBit());
			debug("TRB[%zu].Control.EvaluateNextTRB: %#lx", i, TRBs[i]->Control.EvaluateNextTRB());
			debug("TRB[%zu].Control.TRBType: %#lx", i, TRBs[i]->Control.TRBType());
		}

		CommandIRQComplete = cmdStatus;
		// Op->USBSTS.EventInterrupt(1);
		Op->USBSTS.raw = (1 << 3);
		Interrupter->IMAN.InterruptPending(1);
	}
}
