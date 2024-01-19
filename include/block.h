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

#ifndef __FENNIX_API_BLOCK_H__
#define __FENNIX_API_BLOCK_H__

#include <types.h>
#include <driver.h>

typedef int (*drvOpen_t)(dev_t, dev_t, int, mode_t);
typedef int (*drvClose_t)(dev_t, dev_t);
typedef size_t (*drvRead_t)(dev_t, dev_t, uint8_t *, size_t, off_t);
typedef size_t (*drvWrite_t)(dev_t, dev_t, uint8_t *, size_t, off_t);
typedef int (*drvIoctl_t)(dev_t, dev_t, unsigned long, void *);

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

	dev_t RegisterBlockDevice(DeviceDriverType Type,
							  drvOpen_t Open, drvClose_t Close,
							  drvRead_t Read, drvWrite_t Write, drvIoctl_t Ioctl);

	int UnregisterBlockDevice(dev_t DeviceID, DeviceDriverType Type);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !__FENNIX_API_BLOCK_H__
