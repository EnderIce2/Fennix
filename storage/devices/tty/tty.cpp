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

#include <filesystem/mounts.hpp>
#include <filesystem/ioctl.hpp>
#include <errno.h>

#include "../../../kernel.h"

namespace vfs
{
	size_t TTYDevice::write(uint8_t *Buffer, size_t Size, off_t Offset)
	{
		for (size_t i = 0; i < Size; i++)
			putchar(((char *)Buffer)[i]);

		Display->UpdateBuffer(); /* FIXME: stub */
		return Size;
	}

	int TTYDevice::ioctl(unsigned long Request, void *Argp)
	{
		switch (Request)
		{
		case TIOCGWINSZ:
		{
			struct winsize *ws = (struct winsize *)Argp;
			Video::FontInfo fi = Display->GetCurrentFont()->GetInfo();

			fixme("TIOCGWINSZ: stub");
			ws->ws_xpixel = uint16_t(Display->GetWidth);
			ws->ws_ypixel = uint16_t(Display->GetHeight);
			ws->ws_col = uint16_t(Display->GetWidth / fi.Width);
			ws->ws_row = uint16_t(Display->GetHeight / fi.Height);
			break;
		}
		default:
			fixme("Unknown request %#lx", Request);
			return -EINVAL;
		}
		return 0;
	}

	TTYDevice::TTYDevice() : Node(DevFS, "tty", CHARDEVICE) {}

	TTYDevice::~TTYDevice() {}
}
