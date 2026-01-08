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

#include "ohci.hpp"

extern Driver::Manager *DriverManager;

namespace Driver::OpenHostControllerInterface
{
	extern dev_t DriverID;

	void HCD::OnInterruptReceived(CPU::TrapFrame *Frame)
	{
	}

	int HCD::Reset()
	{
		uint32_t interval = mminl((void *)&regs->HcFmInterval);
		debug("frame interval = %#lx", interval);

		const uint32_t HCR_BIT = (1u << 0);
		mmoutl((void *)&regs->HcCommandStatus, HCR_BIT);

		for (int i = 0; i < 100000; ++i)
		{
			uint32_t cs = mminl((void *)&regs->HcCommandStatus);
			if ((cs & HCR_BIT) == 0)
			{
				debug("ok");
				break;
			}
		}

		mmoutl((void *)&regs->HcFmInterval, interval);

		Memory::Virtual vmm;
		void *hcca = KernelAllocator.RequestPage();
		debug("HCCA physical address: %#lx (virtual %#lx)", hcca, mminl((void *)&regs->HcHCCA));
		mmoutl((void *)&regs->HcHCCA, (uint32_t)(uintptr_t)hcca);

		mmoutl((void *)&regs->HcControlHeadED, 0);
		mmoutl((void *)&regs->HcBulkHeadED, 0);
		mmoutl((void *)&regs->HcDoneHead, 0);

		uint32_t fi = interval & 0x3FFFu;
		uint32_t ps = (fi * 90) / 100; /* approx 90% */
		mmoutl((void *)&regs->HcPeriodicStart, ps & 0x3FFFU);
		return 0;
	}

	int HCD::Start(bool WaitForStart)
	{
		return 0;
	}

	int HCD::Stop()
	{
		return 0;
	}

	int HCD::Detect()
	{
		return 0;
	}

	int HCD::Poll()
	{
		return 0;
	}

	HCD::HCD(uintptr_t base, PCI::PCIDevice &pciHeader)
		: Interrupts::Handler(pciHeader),
		  regs((OHCIRegisters *)base),
		  Header(pciHeader)
	{
		debug("Found OHCI controller at %#lx", regs);
		mmoutl((void *)&regs->HcInterruptEnable, 0);
		mmoutl((void *)&regs->HcInterruptDisable, (1 << 31));

		uint32_t hcRevision = mminl((void *)&regs->HcRevision) & 0xFF;
		debug("Host Controller Revision: %d.%d (%#lx)", (hcRevision >> 4) & 0xF, hcRevision & 0xF, hcRevision);

		queue = new Queue;
	}

	HCD::~HCD()
	{
		this->Stop();
		delete queue;
	}
}
