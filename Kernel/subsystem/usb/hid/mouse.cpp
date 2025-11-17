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

#include <usb.hpp>
#include <auto_page>

#include "../../../kernel.h"

namespace UniversalSerialBus
{
	USBTransfer *mouset;

	int MousePoll(USBDevice *Device)
	{
		if (mouset->Completed)
		{
			if (mouset->Success)
			{
				static uint32_t oldcol = 0;
				static int mx = 0, my = 0;
				uint8_t *buf = (uint8_t *)mouset->Buffer;
				debug("%c%c%c dx=%d dy=%d",
					  buf[0] & 0x1 ? 'L' : ' ',
					  buf[0] & 0x2 ? 'R' : ' ',
					  buf[0] & 0x4 ? 'M' : ' ',
					  (int8_t)buf[1],
					  (int8_t)buf[2]);

				Display->SetPixel(mx, my, oldcol);

				// Update mouse position with bounds checking
				mx += (int8_t)buf[1];
				my += (int8_t)buf[2];

				int width = Display->GetWidth;
				int height = Display->GetHeight;

				if (mx < 0)
					mx = 0;
				if (my < 0)
					my = 0;
				if (mx >= width)
					mx = width - 1;
				if (my >= height)
					my = height - 1;

				uint32_t color;
				if (buf[0] & 0x1)
					color = 0xff0000; // Left button: red
				else if (buf[0] & 0x2)
					color = 0x00ff00; // Right button: green
				else if (buf[0] & 0x4)
					color = 0x0000ff; // Middle button: blue
				else
					color = 0xffffff; // No button: white

				oldcol = Display->GetPixel(mx, my);
				Display->SetPixel(mx, my, color);
				buf[0] = 0;
				buf[1] = 0;
				buf[2] = 0;
				buf[3] = 0;
			}

			mouset->Completed = false;
			Device->PortInt(Device, mouset);
		}

		return 0;
	}

	int InitializeMouse(USBDevice *Device)
	{
		if (Device->Interface.bInterfaceClass != USB_CLASS_HID ||
			Device->Interface.bInterfaceSubClass != USB_HID_SUBCLASS_BOOT ||
			Device->Interface.bInterfaceProtocol != USB_HID_PROTOCOL_MOUSE)
			return -ENODEV;

		Device->PortPoll = MousePoll;
		mouset = new USBTransfer;
		mouset->Buffer = KernelAllocator.RequestPage();
		Memory::Virtual().Map(mouset->Buffer, mouset->Buffer, PAGE_SIZE, Memory::P | Memory::RW | Memory::PWT | Memory::PCD);
		mouset->Length = 4;
		mouset->Request = nullptr;
		mouset->Endpoint = &Device->Endpoint;
		Device->PortInt(Device, mouset);
		while (1)
		{
			// test code
			Device->Controller->PollHC(Device->Controller);
			Device->PortPoll(Device);
		}
		return 0;
	}
}
