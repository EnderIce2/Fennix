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

#include <irq.hpp>

#include "../../kernel.h"

namespace Interrupt
{
	Handler::Handler(int InterruptLine)
	{
		this->InterruptLine = InterruptLine;
		irq.RegisterHandler((Handle)this, InterruptLine, nullptr, true);
		debug("registered interrupt handler for IRQ %d", InterruptLine);
	}

	Handler::~Handler()
	{
		irq.UnregisterHandler((Handle)this, this->InterruptLine);
		debug("unregistered interrupt handler for IRQ %d", this->InterruptLine);
	}

	int Handler::OnInterruptReceived(CPU::TrapFrame *Frame)
	{
		debug("Unhandled interrupt %d", Frame->InterruptNumber);
		return ENOTSUP;
	}

	int Handler::OnInterruptReceived(CPU::SchedulerFrame *Frame)
	{
		debug("Unhandled scheduler interrupt %d", Frame->InterruptNumber);
		return ENOTSUP;
	}
}
