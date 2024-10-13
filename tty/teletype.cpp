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

#include <tty.hpp>

#include <memory.hpp>
#include <stropts.h>
#include <string.h>
#include <uart.hpp>

#include "../kernel.h"

namespace TTY
{
	ssize_t TerminalBuffer::Read(char *OutputBuffer, size_t Size)
	{
		std::lock_guard<std::mutex> lock(Mutex);
		size_t bytesRead = 0;

		while (bytesRead < Size && ReadIndex != WriteIndex)
		{
			OutputBuffer[bytesRead++] = Buffer[ReadIndex];
			ReadIndex = (ReadIndex + 1) % Buffer.size();
		}

		return bytesRead;
	}

	ssize_t TerminalBuffer::Write(const char *InputBuffer, size_t Size)
	{
		std::lock_guard<std::mutex> lock(Mutex);
		size_t bytesWritten = 0;

		for (size_t i = 0; i < Size; ++i)
		{
			Buffer[WriteIndex] = InputBuffer[i];
			WriteIndex = (WriteIndex + 1) % Buffer.size();
			bytesWritten++;
		}

		return bytesWritten;
	}

	/* ======================================================================== */

	int TeletypeDriver::Open(int Flags, mode_t Mode)
	{
		warn("Unimplemented open(%#x, %#x)", Flags, Mode);
		return -ENOSYS;
	}

	int TeletypeDriver::Close()
	{
		warn("Unimplemented close()");
		return -ENOSYS;
	}

	ssize_t TeletypeDriver::Read(void *Buffer, size_t Size, off_t Offset)
	{
		warn("Unimplemented read(%#lx, %#lx, %#lx)", Buffer, Size, Offset);
		return -ENOSYS;
	}

	ssize_t TeletypeDriver::Write(const void *Buffer, size_t Size, off_t Offset)
	{
		warn("Unimplemented write(%#lx, %#lx, %#lx)", Buffer, Size, Offset);
		return -ENOSYS;
	}

	int TeletypeDriver::Ioctl(unsigned long Request, void *Argp)
	{
		warn("Unimplemented ioctl(%#lx, %#lx)", Request, Argp);
		return -ENOSYS;
	}

	TeletypeDriver::TeletypeDriver()
		: TermBuf(1024)
	{
		this->TerminalSize = {
			.ws_row = 0,
			.ws_col = 0,
			.ws_xpixel = 0,
			.ws_ypixel = 0,
		};

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
		this->TerminalConfig.c_iflag = /*ICRNL |*/ IXON;
		this->TerminalConfig.c_oflag = OPOST | ONLCR;
		this->TerminalConfig.c_cflag = CS8 | CREAD | HUPCL;
		this->TerminalConfig.c_lflag = ECHO | ICANON;

		this->TerminalConfig.c_cc[VINTR] = 0x03;	/* ^C */
		this->TerminalConfig.c_cc[VQUIT] = 0x1C;	/* ^\ */
		this->TerminalConfig.c_cc[VERASE] = 0x7F;	/* DEL */
		this->TerminalConfig.c_cc[VKILL] = 0x15;	/* ^U */
		this->TerminalConfig.c_cc[VEOF] = 0x04;		/* ^D */
		this->TerminalConfig.c_cc[VTIME] = 0;		/* Timeout for non-canonical read */
		this->TerminalConfig.c_cc[VMIN] = 1;		/* Minimum number of characters for non-canonical read */
		this->TerminalConfig.c_cc[VSWTC] = 0;		/* ^O */
		this->TerminalConfig.c_cc[VSTART] = 0x11;	/* ^Q */
		this->TerminalConfig.c_cc[VSTOP] = 0x13;	/* ^S */
		this->TerminalConfig.c_cc[VSUSP] = 0x1A;	/* ^Z */
		this->TerminalConfig.c_cc[VEOL] = 0x00;		/* NUL */
		this->TerminalConfig.c_cc[VREPRINT] = 0x12; /* ^R */
		this->TerminalConfig.c_cc[VDISCARD] = 0x14; /* ^T */
		this->TerminalConfig.c_cc[VWERASE] = 0x17;	/* ^W */
		this->TerminalConfig.c_cc[VLNEXT] = 0x19;	/* ^Y */
		this->TerminalConfig.c_cc[VEOL2] = 0x7F;	/* DEL (or sometimes EOF) */
	}

	TeletypeDriver::~TeletypeDriver()
	{
	}
}
