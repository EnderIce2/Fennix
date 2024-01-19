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

#include <block.h>

extern __driverAPI *API;

dev_t RegisterBlockDevice(DeviceDriverType Type, drvOpen_t Open, drvClose_t Close,
						  drvRead_t Read, drvWrite_t Write, drvIoctl_t Ioctl)
{
	return API->RegisterBlockDevice(API->MajorID, Type, Open, Close, Read, Write, Ioctl);
}

int UnregisterBlockDevice(dev_t DeviceID, DeviceDriverType Type)
{
	return API->UnregisterBlockDevice(API->MajorID, DeviceID, Type);
}
