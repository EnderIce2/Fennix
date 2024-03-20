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
#include <smp.hpp>
#include <errno.h>

#include "../../../kernel.h"

namespace vfs
{
	size_t KConDevice::read(uint8_t *Buffer, size_t Size, off_t Offset)
	{
		return DriverManager->InputKeyboardDev->read(Buffer, Size, Offset);
	}

	size_t KConDevice::write(uint8_t *Buffer, size_t Size, off_t Offset)
	{
		if (Offset != 0)
			fixme("Offset is %d", Offset);

		for (size_t i = 0; i < Size; i++)
			putchar(((char *)Buffer)[i]);

		if (!Config.Quiet)
			Display->UpdateBuffer();
		return Size;
	}

	int KConDevice::ioctl(unsigned long Request, void *Argp)
	{
		static_assert(sizeof(struct termios) < PAGE_SIZE);

		void *pArgp = thisProcess->PageTable->Get(Argp);
		switch (Request)
		{
		case TCGETS:
		{
			struct termios *t = (struct termios *)pArgp;
			memcpy(t, &this->term, sizeof(struct termios));
			break;
		}
		case TCSETS:
		{
			struct termios *t = (struct termios *)pArgp;
			memcpy(&this->term, t, sizeof(struct termios));
			break;
		}
		case TIOCGPGRP:
		{
			*((pid_t *)Argp) = 0;
			fixme("TIOCGPGRP not implemented");
			return 0;
		}
		case TIOCSPGRP:
		{
			*((pid_t *)Argp) = 0;
			fixme("TIOCSPGRP not implemented");
			return 0;
		}
		case TIOCGWINSZ:
		{
			struct winsize *ws = (struct winsize *)pArgp;
			memcpy(ws, &this->termSize, sizeof(struct winsize));
			break;
		}
		case TIOCSWINSZ:
		{
			struct winsize *ws = (struct winsize *)pArgp;
			memcpy(&this->termSize, ws, sizeof(struct winsize));
			break;
		}
		case TCSETSW:
		case TCSETSF:
		case TCGETA:
		case TCSETA:
		case TCSETAW:
		case TCSETAF:
		case TCSBRK:
		case TCXONC:
		case TCFLSH:
		case TIOCEXCL:
		case TIOCNXCL:
		case TIOCSCTTY:
		case TIOCOUTQ:
		case TIOCSTI:
		case TIOCMGET:
		case TIOCMBIS:
		case TIOCMBIC:
		case TIOCMSET:
		{
			fixme("ioctl %#lx not implemented", Request);
			return -ENOSYS;
		}
		case TIOCGPTN:
		case 0xffffffff80045430: /* FIXME: ???? */
		{
			int *n = (int *)pArgp;
			*n = -1;
			break;
		}
		case TIOCSPTLCK:
		{
			int *n = (int *)pArgp;
			*n = 0;
			break;
		}
		default:
		{
			debug("Unknown ioctl %#lx", Request);
			return -EINVAL;
		}
		}

		return 0;
	}

	KConDevice::KConDevice() : Node(DevFS, "kcon", CHARDEVICE)
	{
		/*
			- ICRNL  - Map Carriage Return to New Line
			- IXON   - Enable XON/XOFF flow control

			- OPOST  - Enable output processing
			- ONLCR  - Map New Line to Carriage Return - New Line

			- CS8    - 8-bit characters
			- CREAD  - Enable receiver
			- HUPCL  - Hang up on last close

			- ECHO   - Echo input characters
			- ICANON - Enable canonical input (enable line editing)
		*/
		this->term.c_iflag = /*ICRNL |*/ IXON;
		this->term.c_oflag = OPOST | ONLCR;
		this->term.c_cflag = CS8 | CREAD | HUPCL;
		this->term.c_lflag = ECHO | ICANON;
		this->term.c_cc[VEOF] = 0x04;	 /* ^D */
		this->term.c_cc[VEOL] = 0x00;	 /* NUL */
		this->term.c_cc[VERASE] = 0x7f;	 /* DEL */
		this->term.c_cc[VINTR] = 0x03;	 /* ^C */
		this->term.c_cc[VKILL] = 0x15;	 /* ^U */
		this->term.c_cc[VMIN] = 1;		 /* Minimum number of characters for non-canonical read */
		this->term.c_cc[VQUIT] = 0x1c;	 /* ^\ */
		this->term.c_cc[VSTART] = 0x11;	 /* ^Q */
		this->term.c_cc[VSTOP] = 0x13;	 /* ^S */
		this->term.c_cc[VSUSP] = 0x1a;	 /* ^Z */
		this->term.c_cc[VTIME] = 0;		 /* Timeout for non-canonical read */
		this->term.c_cc[VWERASE] = 0x17; /* ^W */
	}

	KConDevice::~KConDevice()
	{
	}
}
