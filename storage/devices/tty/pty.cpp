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
#include <string.h>
#include <errno.h>

#include "../../../kernel.h"

namespace vfs
{
	PTYDevice::PTYDevice(Inode *_pts, int _id)
	{
		assert(!"Function not implemented");
		char nameBuffer[16];
		snprintf(nameBuffer, 16, "%d", id);
		// this->Name = strdup(nameBuffer);

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

		// debug("Created PTY device %s", this->Name);
	}

	PTYDevice::~PTYDevice() {}
}
