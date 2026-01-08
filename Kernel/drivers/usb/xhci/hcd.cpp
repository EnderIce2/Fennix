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
	}

	int HCD::Reset()
	{
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

	HCD::HCD(PCI::PCIDevice &pciHeader)
		: Interrupts::Handler(pciHeader),
		  Header(pciHeader)
	{
		PCI::PCIHeader0 *hdr0 = (PCI::PCIHeader0 *)pciHeader.Header;
		MMIOBase = hdr0->BAR[0];
		if (hdr0->BAR[0] & 0x4)
			MMIOBase |= (uint64_t)hdr0->BAR[1] << 32;

		if (MMIOBase & 0x1)
			MMIOBase &= 0xFFFFFFFFFFFFFFFC;
		else
			MMIOBase &= 0xFFFFFFFFFFFFFFF0;
		
		Memory::Virtual vmm;
		vmm.Map((void *)MMIOBase, (void *)MMIOBase, PAGE_SIZE * 2, Memory::P | Memory::RW | Memory::PWT | Memory::PCD);

		Cap = (Capability *)MMIOBase;
		Op = (Operational *)(MMIOBase + Cap->CAPLENGTH);
		Ports = (PortRegister *)(MMIOBase + Cap->CAPLENGTH + 0x400);
		Rt = (Runtime *)(MMIOBase + (Cap->RTSOFF & ~0x1F));
		Db = (Doorbell *)(MMIOBase + (Cap->DBOFF & ~0x3));

		stub;
	}

	HCD::~HCD()
	{
		this->Stop();
	}
}
