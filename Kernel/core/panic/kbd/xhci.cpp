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

#include <interface/aip.h>
#include <display.hpp>
#include <convert.h>
#include <printf.h>
#include <kcon.hpp>
#include <debug.h>
#include <smp.hpp>
#include <cpu.hpp>
#include <io.h>

#if defined(a64)
#include "../../../arch/amd64/cpu/gdt.hpp"
#elif defined(a32)
#elif defined(aa64)
#endif

#include "../../../kernel.h"

using namespace KernelConsole;
using namespace PCI;

#define ERROR_COLOR "\x1b[31m"
#define WARN_COLOR "\x1b[33m"
#define DEFAULT_COLOR "\x1b[0m"

extern void ExPrint(const char *Format, ...);
extern void ArrowInput(uint8_t key);
extern void UserInput(char *Input);
extern FontRenderer CrashFontRenderer;

nsa bool CrashXHCIKeyboardDriver::TakeOwnership()
{
	HCExtCap *exCap = (HCExtCap *)(uintptr_t)this->ExtendedCaps;

	if (exCap->USBLEGSUP.CapID != 1)
		return true;

	if (exCap->USBLEGSUP.BIOSOwnsHC == 0)
		return true;

	exCap->USBLEGSUP.OSOwnsHC = 1;
	TimeManager->Sleep(200, Time::Milliseconds);
	if (exCap->USBLEGSUP.BIOSOwnsHC == 0)
		return true;

	ExPrint(ERROR_COLOR "BIOS owns the USB controller\n" DEFAULT_COLOR);
	return false;
}

nsa bool CrashXHCIKeyboardDriver::Initialize()
{
	int timeout = 10;

	if (!TakeOwnership())
	{
		ExPrint(ERROR_COLOR "Failed to take ownership\n" DEFAULT_COLOR);
		return false;
	}

	stub;
	return false; /* FIXME: stub */
}

nsa CrashXHCIKeyboardDriver::CrashXHCIKeyboardDriver(PCIDevice xhci)
	: Interrupts::Handler(xhci)
{
	Header = (PCIDeviceHeader *)xhci.Header;

	switch (Header->HeaderType)
	{
	case 128:
	{
		ExPrint(ERROR_COLOR "Unknown header type %d! Guessing PCI Header 0\n" DEFAULT_COLOR,
				Header->HeaderType);
		[[fallthrough]];
	}
	case 0: /* PCI Header 0 */
	{
		PCI::PCIHeader0 *hdr = (PCI::PCIHeader0 *)Header;

		uint32_t BAR[6];
		size_t BARsSize[6];

		BAR[0] = hdr->BAR0;
		BAR[1] = hdr->BAR1;
		BAR[2] = hdr->BAR2;
		BAR[3] = hdr->BAR3;
		BAR[4] = hdr->BAR4;
		BAR[5] = hdr->BAR5;

		/* BARs Size */
		for (short i = 0; i < 6; i++)
		{
			if (BAR[i] == 0)
				continue;

			size_t size;
			if ((BAR[i] & 1) == 0) /* Memory Base */
			{
				hdr->BAR0 = 0xFFFFFFFF;
				size = hdr->BAR0;
				hdr->BAR0 = BAR[i];
				BARsSize[i] = size & (~15);
				BARsSize[i] = ~BARsSize[i] + 1;
				BARsSize[i] = BARsSize[i] & 0xFFFFFFFF;
				debug("MEM BAR%d %#lx size: %d",
					  i, BAR[i], BARsSize[i]);
			}
			else if ((BAR[i] & 1) == 1) /* I/O Base */
			{
				hdr->BAR1 = 0xFFFFFFFF;
				size = hdr->BAR1;
				hdr->BAR1 = BAR[i];
				BARsSize[i] = size & (~3);
				BARsSize[i] = ~BARsSize[i] + 1;
				BARsSize[i] = BARsSize[i] & 0xFFFF;
				debug("IO BAR%d %#lx size: %d",
					  i, BAR[i], BARsSize[i]);
			}
		}

		debug("IO %d 64-BIT %d", BAR[0] & 0x1, BAR[0] & 0x4);

		uintptr_t baseAddress = BAR[0];
		if (BAR[0] & 0x4)
			baseAddress |= (uintptr_t)BAR[1] << 32;

		if (baseAddress & 0x1)
			baseAddress &= 0xFFFFFFFFFFFFFFFC;
		else
			baseAddress &= 0xFFFFFFFFFFFFFFF0;

		debug("baseAddress: %#lx", baseAddress);
		Memory::Virtual vmm;
		vmm.Map((void *)baseAddress, (void *)baseAddress, BARsSize[0], Memory::RW);
		caps = (XHCIcap *)baseAddress;
		ops = (XHCIop *)(baseAddress + caps->CAPLENGTH);
		port = (XHCIport *)(baseAddress + caps->CAPLENGTH + 0x400);
		runtime = (XHCIruntime *)(baseAddress + (caps->RTSOFF & ~0x1F));
		doorbell = (XHCIdoorbell *)(baseAddress + (caps->DBOFF & ~0x3));
		uint16_t exCapOffset = caps->HCCPARAMS1.xHCIExtendedCapacitiesPointer << 2;
		ExtendedCaps = (uintptr_t)caps + exCapOffset;
		debug("ExtendedCaps: %#lx (%#lx + %#lx)", ExtendedCaps, caps, exCapOffset);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
		Interrupter = &runtime->Interrupter[0];
#pragma GCC diagnostic pop
		break;
	}
	case 1: /* PCI Header 1 (PCI-to-PCI Bridge) */
	{
		ExPrint(ERROR_COLOR "PCI-to-PCI Bridge not supported\n" DEFAULT_COLOR);
		break;
	}
	case 2: /* PCI Header 2 (PCI-to-CardBus Bridge) */
	{
		ExPrint(ERROR_COLOR "PCI-to-CardBus Bridge not supported\n" DEFAULT_COLOR);
		break;
	}
	default:
	{
		ExPrint(ERROR_COLOR "Invalid PCI header type\n" DEFAULT_COLOR);
		break;
	}
	}
}

nsa void CrashXHCIKeyboardDriver::OnInterruptReceived(CPU::TrapFrame *Frame)
{
	debug("Interrupt received");

	Interrupter->IMAN.IP = 1;

	if (!ops->USBSTS.EINT)
		debug("!USBSTS.EINT");
	// return;

	ops->USBSTS.EINT = 1;

	stub;

	Interrupter->IMAN.IP = 0;
	ops->USBSTS.EINT = 0;
}
