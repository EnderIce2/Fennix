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

#include "ehci.hpp"

#include <interface/aip.h>
#include <display.hpp>
#include <convert.h>
#include <printf.h>
#include <kcon.hpp>
#include <debug.h>
#include <smp.hpp>
#include <cpu.hpp>
#include <io.h>

#if defined(__amd64__)
#include "../../../arch/amd64/cpu/gdt.hpp"
#elif defined(__i386__)
#elif defined(__aarch64__)
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

nsa int CrashEHCIKeyboardDriver::OnInterruptReceived(CPU::TrapFrame *Frame)
{
	return EOK;
}

nsa bool CrashEHCIKeyboardDriver::Initialize()
{
	return false;
}

nsa CrashEHCIKeyboardDriver::CrashEHCIKeyboardDriver(PCI::PCIDevice dev)
	: Interrupts::Handler(dev)
{
	Header = (PCIDeviceHeader *)dev.Header;

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
		UNUSED(hdr);
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
