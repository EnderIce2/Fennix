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

nsa void CrashUHCIKeyboardDriver::OnInterruptReceived(CPU::TrapFrame *Frame)
{
}

__no_sanitize("undefined") nsa bool CrashUHCIKeyboardDriver::Initialize()
{
	return false; /* FIXME: stub */

	debug("Allocating resources");
	this->FrameList = (uint32_t *)(uintptr_t)KernelAllocator.RequestPages(TO_PAGES(1024 * sizeof(FrameList[0])));
	this->td = (TD *)KernelAllocator.RequestPages(TO_PAGES(sizeof(TD) * 32));
	memset(td, 0, sizeof(TD) * 32);
	this->qh = (QH *)KernelAllocator.RequestPages(TO_PAGES(sizeof(QH) * 8));
	memset(qh, 0, sizeof(QH) * 8);

	/* FIXME: stub */

	debug("Initializing controller");
	outw((uint16_t)((uintptr_t)io + 0xC0), 0x8F00); /* Disable Legacy Mode Support */

	/* Disable All Interrupts */
	io->USBINTR.TOCRC = 0;
	io->USBINTR.RIE = 0;
	io->USBINTR.IOCE = 0;
	io->USBINTR.SPIE = 0;

	/* Configure Frame List */
	io->FRNUM.FN = 0;
	io->FRBASEADD.BA = (uint32_t)(uintptr_t)FrameList;
	io->SOFMOD.SOFTVAL = 0b1000000;

	io->USBSTS.raw = 0xFFFF; /* Clear USBSTS */

	if (io->USBSTS.HCH)
		io->USBCMD.RS = 1;
	else
	{
		debug("Controller not halted, what to do?");
	}

	return true;
}

nsa CrashUHCIKeyboardDriver::CrashUHCIKeyboardDriver(PCI::PCIDevice dev)
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
				debug("I/O BAR%d %#lx size: %d",
					  i, BAR[i], BARsSize[i]);
			}
		}

		uintptr_t baseAddress = BAR[4];

		assert(baseAddress & 0x1); /* must be I/O */

		debug("baseAddress: %#lx size %#lx", baseAddress, BARsSize[4]);
		Memory::Virtual vmm;
		vmm.Map((void *)baseAddress, (void *)baseAddress, BARsSize[4], Memory::RW);
		io = (IORegisters *)baseAddress;

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
