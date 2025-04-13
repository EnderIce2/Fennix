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
	TeletypeDriver::TeletypeDriver() : TermBuf(1024)
	{
		if (thisProcess)
			this->ProcessGroup = thisProcess->Security.ProcessGroupID;
		else
			this->ProcessGroup = 0;

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
		- ISIG   - Enable signals
		*/
		this->TerminalConfig.c_iflag = /*ICRNL |*/ IXON;
		this->TerminalConfig.c_oflag = OPOST | ONLCR;
		this->TerminalConfig.c_cflag = CS8 | CREAD | HUPCL;
		this->TerminalConfig.c_lflag = ECHO | ICANON | ISIG;

		this->TerminalConfig.c_cc[VINTR] = 'C' - 0x40;
		this->TerminalConfig.c_cc[VQUIT] = '\\' - 0x40;
		this->TerminalConfig.c_cc[VERASE] = '\177';
		this->TerminalConfig.c_cc[VKILL] = 'U' - 0x40;
		this->TerminalConfig.c_cc[VEOF] = 'D' - 0x40;
		this->TerminalConfig.c_cc[VSTART] = 'Q' - 0x40;
		this->TerminalConfig.c_cc[VSTOP] = 'S' - 0x40;
		this->TerminalConfig.c_cc[VSUSP] = 'Z' - 0x40;
		this->TerminalConfig.c_cc[VREPRINT] = 'R' - 0x40;
		this->TerminalConfig.c_cc[VDISCARD] = 'O' - 0x40;
		this->TerminalConfig.c_cc[VWERASE] = 'W' - 0x40;
		this->TerminalConfig.c_cc[VLNEXT] = 'V' - 0x40;
	}
}
