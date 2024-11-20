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

#include "keyboard.hpp"
#include "ps2.hpp"
#include "uhci.hpp"
#include "ehci.hpp"
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

nsa bool DetectUSBKeyboard()
{
	if (!PCIManager)
		return false;
	std::list<PCIDevice> uhci = PCIManager->FindPCIDevice(0xC, 0x3, 0x00); /* UHCI */
	debug("There are %lu UHCI devices", uhci.size());

	std::list<PCIDevice> ohci = PCIManager->FindPCIDevice(0xC, 0x3, 0x10); /* OHCI */
	debug("There are %lu OHCI devices", ohci.size());

	std::list<PCIDevice> ehci = PCIManager->FindPCIDevice(0xC, 0x3, 0x20); /* EHCI */
	debug("There are %lu EHCI devices", uhci.size());

	std::list<PCIDevice> xhci = PCIManager->FindPCIDevice(0xC, 0x3, 0x30); /* XHCI */
	debug("There are %lu XHCI devices", xhci.size());

	debug("Initializing UHCI devices");
	for (const auto &dev : uhci)
	{
		CrashUHCIKeyboardDriver *usb = new CrashUHCIKeyboardDriver(dev);
		if (!usb->Initialize())
		{
			error("Failed to initialize UHCI keyboard driver");
			uhci.pop_front();
		}
	}

	debug("Initializing EHCI devices");
	for (const auto &dev : ehci)
	{
		CrashEHCIKeyboardDriver *usb = new CrashEHCIKeyboardDriver(dev);
		if (!usb->Initialize())
		{
			error("Failed to initialize UHCI keyboard driver");
			ehci.pop_front();
		}
	}

	debug("Initializing XHCI devices");
	for (const auto &dev : xhci)
	{
		CrashXHCIKeyboardDriver *usb = new CrashXHCIKeyboardDriver(dev);
		if (!usb->Initialize())
		{
			error("Failed to initialize XHCI keyboard driver");
			xhci.pop_front();
		}
	}

	return xhci.size() > 0 || uhci.size() > 0 || ohci.size() > 0;
}

nsa void InitializeKeyboards()
{
	if (DetectUSBKeyboard() == false)
		new CrashPS2KeyboardDriver;
}
