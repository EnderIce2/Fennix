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

#include <kcon.hpp>

#include <filesystem/ioctl.hpp>
#include <memory.hpp>
#include <stropts.h>
#include <string.h>

namespace KernelConsole
{
	termios term{};
	winsize termSize{};

	ssize_t KConRead(struct Inode *Node, void *Buffer, size_t Size, off_t Offset)
	{
		fixme("Reading %d bytes... \"%.*s\"", Size, Size, (char *)Buffer);
		return Size;
	}

	ssize_t KConWrite(struct Inode *Node, const void *Buffer, size_t Size, off_t Offset)
	{
		fixme("Writing %d bytes... \"%.*s\"", Size, Size, (char *)Buffer);
		return Size;
	}

	int KConIoctl(struct Inode *Node, unsigned long Request, void *Argp)
	{
		switch (Request)
		{
		case TCGETS:
		{
			struct termios *t = (struct termios *)Argp;
			memcpy(t, &term, sizeof(struct termios));
			return 0;
		}
		case TCSETS:
		{
			debug("TCSETS not supported");
			return -EINVAL;

			struct termios *t = (struct termios *)Argp;
			memcpy(&term, t, sizeof(struct termios));
			return 0;
		}
		case TIOCGPGRP:
		{
			*((pid_t *)Argp) = 0;
			return 0;
		}
		case TIOCSPGRP:
		{
			*((pid_t *)Argp) = 0;
			return 0;
		}
		case TIOCGWINSZ:
		{
			struct winsize *ws = (struct winsize *)Argp;
			memcpy(ws, &termSize, sizeof(struct winsize));
			return 0;
		}
		case TIOCSWINSZ:
		{
			debug("TIOCSWINSZ not supported");
			return -EINVAL;

			struct winsize *ws = (struct winsize *)Argp;
			memcpy(&termSize, ws, sizeof(struct winsize));
			return 0;
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
			fixme("stub ioctl %#lx", Request);

			int *n = (int *)Argp;
			*n = -1;
			break;
		}
		case TIOCSPTLCK:
		{
			fixme("stub ioctl %#lx", Request);

			int *n = (int *)Argp;
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

	void EarlyInit()
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
		term.c_iflag = /*ICRNL |*/ IXON;
		term.c_oflag = OPOST | ONLCR;
		term.c_cflag = CS8 | CREAD | HUPCL;
		term.c_lflag = ECHO | ICANON;
		term.c_cc[VEOF] = 0x04;	   /* ^D */
		term.c_cc[VEOL] = 0x00;	   /* NUL */
		term.c_cc[VERASE] = 0x7f;  /* DEL */
		term.c_cc[VINTR] = 0x03;   /* ^C */
		term.c_cc[VKILL] = 0x15;   /* ^U */
		term.c_cc[VMIN] = 1;	   /* Minimum number of characters for non-canonical read */
		term.c_cc[VQUIT] = 0x1c;   /* ^\ */
		term.c_cc[VSTART] = 0x11;  /* ^Q */
		term.c_cc[VSTOP] = 0x13;   /* ^S */
		term.c_cc[VSUSP] = 0x1a;   /* ^Z */
		term.c_cc[VTIME] = 0;	   /* Timeout for non-canonical read */
		term.c_cc[VWERASE] = 0x17; /* ^W */
	}

	void LateInit()
	{
	}
}
