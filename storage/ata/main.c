/*
	This file is part of Fennix Drivers.

	Fennix Drivers is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Drivers is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Drivers. If not, see <https://www.gnu.org/licenses/>.
*/

#include <errno.h>
#include <regs.h>
#include <base.h>
#include <io.h>

bool IsATAPresent()
{
	outb(0x1F0 + 2, 0);
	outb(0x1F0 + 3, 0);
	outb(0x1F0 + 4, 0);
	outb(0x1F0 + 5, 0);
	outb(0x1F0 + 7, 0xEC);
	if (inb(0x1F0 + 7) == 0 || inb(0x1F0 + 1) != 0)
		return false;
	return true;
}

void MasterInterruptHandler(TrapFrame *)
{
}

void SlaveInterruptHandler(TrapFrame *)
{
}

int DriverEntry()
{
	RegisterInterruptHandler(14, MasterInterruptHandler);
	RegisterInterruptHandler(15, SlaveInterruptHandler);

	return 0;
}

int DriverFinal()
{
	UnregisterInterruptHandler(14, MasterInterruptHandler);
	UnregisterInterruptHandler(15, SlaveInterruptHandler);

	return 0;
}

int DriverPanic()
{
	return 0;
}

int DriverProbe()
{
	if (!IsATAPresent())
		return -ENODEV;

	return 0;
}

DriverInfo("ata",
		   "Advanced Technology Attachment Driver",
		   "EnderIce2",
		   0, 0, 1,
		   "GPLv3");
