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

#ifndef __FENNIX_API_DRIVER_FUNCTIONS_H__
#define __FENNIX_API_DRIVER_FUNCTIONS_H__

#include <types.h>

typedef struct
{
	/* PCIDevice */ void *Device;
	/* __PCIArray */ void *Next;
} __PCIArray;

/* ========================================== */

#define PCI_END 0x0000
#define KEY_NULL 0x00

typedef enum
{
	IOCTL_AUDIO_GET_VOLUME = 0,
	IOCTL_AUDIO_SET_VOLUME = 1,

	IOCTL_AUDIO_GET_MUTE = 2,
	IOCTL_AUDIO_SET_MUTE = 3,

	IOCTL_AUDIO_GET_SAMPLE_RATE = 4,
	IOCTL_AUDIO_SET_SAMPLE_RATE = 5,

	IOCTL_AUDIO_GET_CHANNELS = 6,
	IOCTL_AUDIO_SET_CHANNELS = 7,
} AudioIoctl;

typedef enum
{
	IOCTL_NET_GET_MAC = 0,
} NetIoctl;

typedef enum
{
	MAP_PRESENT = 1 << 0,
	MAP_WRITE = 1 << 1,
	MAP_USER = 1 << 2,
	MAP_WRITE_THROUGH = 1 << 3,
	MAP_CACHE_DISABLE = 1 << 4,
} PageMapFlags;

struct __DriverInfo
{
	const char *Name;
	const char *Description;
	const char *Author;
	struct __DriverVersion
	{
		int APIVersion;
		int Major, Minor, Patch;
	} Version;
	const char *License;
};

#endif // !__FENNIX_API_DRIVER_FUNCTIONS_H__
