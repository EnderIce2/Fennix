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

#include <base.h>

int DriverEntry()
{
	return 0;
}

int DriverFinal()
{
	return 0;
}

int DriverPanic()
{
	return 0;
}

int DriverProbe()
{
	/* Nothing to do */
	return 0;
}

DriverInfo("fat",
		   "File Allocation Table Driver",
		   "EnderIce2",
		   0, 0, 1,
		   "GPLv3");
