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

#ifndef __FENNIX_API_DEVICE_H__
#define __FENNIX_API_DEVICE_H__

#include <types.h>

#ifndef __FENNIX_API_FILESYSTEM_H__
#if __has_include(<interface/fs.h>)
#include <interface/fs.h>
#else
#include <fs.h>
#endif
#endif // !__FENNIX_API_FILESYSTEM_H__

typedef enum
{
	DEVICE_TYPE_MASK = 0b1111111100000000000000000000000000000000,
	DEVICE_TYPE_NONE = 0b0000000000000000000000000000000000000000,
	DEVICE_TYPE_INPUT = 0b0000000100000000000000000000000000000000,
	DEVICE_TYPE_AUDIO = 0b0000001000000000000000000000000000000000,
	DEVICE_TYPE_NETWORK = 0b0000010000000000000000000000000000000000,
	DEVICE_TYPE_BLOCK = 0b0000100000000000000000000000000000000000,

	INPUT_TYPE_NONE = DEVICE_TYPE_INPUT + 0,
	INPUT_TYPE_KEYBOARD = DEVICE_TYPE_INPUT + 2,
	INPUT_TYPE_MOUSE = DEVICE_TYPE_INPUT + 4,
	INPUT_TYPE_JOYSTICK = DEVICE_TYPE_INPUT + 8,
	INPUT_TYPE_TOUCHSCREEN = DEVICE_TYPE_INPUT + 16,
	INPUT_TYPE_GAMEPAD = DEVICE_TYPE_INPUT + 32,
	INPUT_TYPE_ACCELEROMETER = DEVICE_TYPE_INPUT + 64,
	INPUT_TYPE_GYROSCOPE = DEVICE_TYPE_INPUT + 128,
	INPUT_TYPE_MAGNETOMETER = DEVICE_TYPE_INPUT + 256,

	AUDIO_TYPE_NONE = DEVICE_TYPE_AUDIO + 0,
	AUDIO_TYPE_PWM = DEVICE_TYPE_AUDIO + 2,
	AUDIO_TYPE_DSP = DEVICE_TYPE_AUDIO + 4,
	AUDIO_TYPE_PCM = DEVICE_TYPE_AUDIO + 8,
	AUDIO_TYPE_MIDI = DEVICE_TYPE_AUDIO + 16,

	NETWORK_TYPE_NONE = DEVICE_TYPE_NETWORK + 0,
	NETWORK_TYPE_ETHERNET = DEVICE_TYPE_NETWORK + 2,
	NETWORK_TYPE_WIFI = DEVICE_TYPE_NETWORK + 4,
	NETWORK_TYPE_BLUETOOTH = DEVICE_TYPE_NETWORK + 8,
	NETWORK_TYPE_UART = DEVICE_TYPE_NETWORK + 16,

	BLOCK_TYPE_NONE = DEVICE_TYPE_BLOCK + 0,
	BLOCK_TYPE_SDCARD = DEVICE_TYPE_BLOCK + 2,
	BLOCK_TYPE_HDD = DEVICE_TYPE_BLOCK + 4,
	BLOCK_TYPE_SSD = DEVICE_TYPE_BLOCK + 8,
	BLOCK_TYPE_USB = DEVICE_TYPE_BLOCK + 16,
	BLOCK_TYPE_NVME = DEVICE_TYPE_BLOCK + 32,
	BLOCK_TYPE_CDROM = DEVICE_TYPE_BLOCK + 64,
	BLOCK_TYPE_FLOPPY = DEVICE_TYPE_BLOCK + 128,
} DeviceType;

#ifndef __kernel__
EXTERNC dev_t CreateDeviceFile(const char *name, mode_t mode, const struct InodeOperations *Operations);
EXTERNC dev_t RegisterDevice(DeviceType Type, const struct InodeOperations *Operations);
EXTERNC int UnregisterDevice(dev_t Device);
#endif // !__kernel__

#endif // !__FENNIX_API_DEVICE_H__
