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

#ifndef __FENNIX_API_BLOCK_H__
#define __FENNIX_API_BLOCK_H__

#include <types.h>

#if __has_include(<interface/fs.h>)
#include <interface/fs.h>
#else
#include <fs.h>
#endif

struct BlockDevice
{
	/**
	 * @brief Base name of the device.
	 *
	 * This name is used to identify the device in the system. It should be unique
	 * across all block devices. The kernel may append a number to this name to
	 * create a unique device name (e.g., "ahci0", "ahci1").
	 */
	const char *Name;

	/**
	 * @brief Total size of the device in bytes.
	 *
	 * This value represents the total addressable storage capacity of the device.
	 * It is used for bounds checking and partitioning.
	 */
	size_t Size;

	/**
	 * @brief Size of a single block in bytes.
	 *
	 * All read and write operations are performed in multiples of this block size.
	 * Typical values are 512 or 4096 bytes.
	 */
	uint32_t BlockSize;

	/**
	 * @brief Number of blocks in the device.
	 *
	 * This value is calculated as Size / BlockSize. It represents the total number
	 * of addressable blocks on the device.
	 */
	size_t BlockCount;

	/**
	 * @brief Pointer to the block device operations structure.
	 *
	 * This structure contains function pointers for various operations that can
	 * be performed on the block device, such as read, write, and ioctl.
	 * 
	 * Yea, inode operations are used for block devices too.
	 */
	const InodeOperations *Ops;

	/**
	 * @brief Opaque pointer to driver-specific or hardware-specific data.
	 *
	 * This field allows the driver to associate private context or state with the
	 * device, such as controller registers or internal buffers.
	 */
	void *PrivateData;
};

#ifndef __kernel__
/**
 * @brief Registers a block device with the kernel block subsystem.
 *
 * This function should be called by block device drivers after initializing
 * a device. The kernel will take ownership of the device structure and assign
 * it a unique device ID. The device will then be accessible for filesystem
 * mounting and I/O operations.
 *
 * @param Device Pointer to a fully initialized BlockDevice structure. All required fields must be set and valid for the lifetime of the device.
 * @return Device ID (dev_t) assigned by the kernel on success, or an error code on failure.
 */
dev_t RegisterBlockDevice(struct BlockDevice *Device);

/**
 * @brief Unregisters a block device from the kernel block subsystem.
 *
 * This function should be called by drivers when a device is being removed
 * or is no longer available. The kernel will release any resources associated
 * with the device and invalidate its device ID.
 *
 * @param DeviceID The device ID (dev_t) previously returned by RegisterBlockDevice().
 * @return 0 on success, or an error code.
 */
int UnregisterBlockDevice(dev_t DeviceID);
#endif // __kernel__

#endif // __FENNIX_API_BLOCK_H__
