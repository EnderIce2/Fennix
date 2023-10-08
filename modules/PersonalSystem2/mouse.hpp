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

#ifndef __FENNIX_KERNEL_PS2_MOUSE_H__
#define __FENNIX_KERNEL_PS2_MOUSE_H__

#include <types.h>
#include "../../mapi.hpp"

namespace PS2Mouse
{
#define PS2LeftButton 0b00000001
#define PS2MiddleButton 0b00000100
#define PS2RightButton 0b00000010
#define PS2XSign 0b00010000
#define PS2YSign 0b00100000
#define PS2XOverflow 0b01000000
#define PS2YOverflow 0b10000000

	enum Config
	{
		READ_CONFIG = 0x20,
		WRITE_CONFIG = 0x60
	};

	enum Ports
	{
		DATA = 0x60,
		STATUS = 0x64,
		COMMAND = 0x64,
	};

	enum State
	{
		OUTPUT_FULL = (1 << 0),
		INPUT_FULL = (1 << 1),
		MOUSE_BYTE = (1 << 5)
	};

	int DriverEntry(void *);
	int CallbackHandler(KernelCallback *);
	int InterruptCallback(CPURegisters *);
}

#endif // !__FENNIX_KERNEL_PS2_MOUSE_H__
