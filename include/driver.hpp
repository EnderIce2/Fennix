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

#ifndef __FENNIX_KERNEL_DRIVER_H__
#define __FENNIX_KERNEL_DRIVER_H__

#include <types.h>

#include <memory.hpp>
#include <ints.hpp>
#include <lock.hpp>
#include <debug.h>
#include <cpu.hpp>
#include <pci.hpp>
#include <vector>

namespace Driver
{
	enum DriverCode
	{
		/* This must be the same as in DAPI.hpp DriverReturnCode */
		ERROR,
		OK,
		NOT_IMPLEMENTED,
		NOT_FOUND,
		NOT_READY,
		NOT_AVAILABLE,
		NOT_AUTHORIZED,
		NOT_VALID,
		NOT_ACCEPTED,
		INVALID_PCI_BAR,
		INVALID_KERNEL_API,
		INVALID_MEMORY_ALLOCATION,
		INVALID_DATA,
		DEVICE_NOT_SUPPORTED,
		SYSTEM_NOT_SUPPORTED,
		KERNEL_API_VERSION_NOT_SUPPORTED,

		/* End of driver-only errors */

		INVALID_FEX_HEADER,
		INVALID_DRIVER_DATA,
		NOT_DRIVER,
		DRIVER_RETURNED_ERROR,
		UNKNOWN_DRIVER_TYPE,
		UNKNOWN_DRIVER_BIND_TYPE,
		PCI_DEVICE_NOT_FOUND,
		DRIVER_CONFLICT
	};

	class DriverInterruptHook;
	struct DriverFile
	{
		bool Enabled = false;
		bool BuiltIn = false;
		unsigned int DriverUID = 0;
		void *Address = nullptr;
		void *ExtendedHeaderAddress = nullptr;
		void *InterruptCallback = nullptr;
		Memory::MemMgr *MemTrk = nullptr;
		DriverInterruptHook *InterruptHook[16]{};

		bool operator==(const DriverFile &Other) const
		{
			return DriverUID == Other.DriverUID;
		}
	};

	struct BuiltInDriverInfo
	{
		int (*EntryPoint)(void *);
		void *ExtendedHeader;
	};

	class DriverInterruptHook : public Interrupts::Handler
	{
	private:
		NewLock(DriverInterruptLock);

		DriverFile Handle;
		bool Enabled = true;

#if defined(a64)
		void OnInterruptReceived(CPU::x64::TrapFrame *Frame);
#elif defined(a32)
		void OnInterruptReceived(CPU::x32::TrapFrame *Frame);
#elif defined(aa64)
		void OnInterruptReceived(CPU::aarch64::TrapFrame *Frame);
#endif

	public:
		void Enable() { Enabled = true; }
		void Disable() { Enabled = false; }
		bool IsEnabled() { return Enabled; }
		DriverInterruptHook(int Interrupt, DriverFile Handle);
		virtual ~DriverInterruptHook() = default;
	};

	class Driver
	{
	private:
		NewLock(DriverInitLock);

		std::vector<DriverFile> Drivers;
		unsigned int DriverUIDs = 0;
		/* If BuiltIn is true, the "fex" is the entry point. */
		DriverCode CallDriverEntryPoint(void *fex, bool BuiltIn = false);

	public:
		/**
		 * @brief Load and bind a driver to a PCI device.
		 *
		 * This function will search for a PCI device with the given VendorID and DeviceID.
		 * If the device is found, the driver will be loaded and bound to the device.
		 *
		 * @param DriverAddress The address of the driver. The file will be copied to a new location.
		 * @param Size The size of the driver.
		 * @param IsBuiltIn If the driver is built-in, the @param DriverAddress will be @see BuiltInDriverInfo and the @param Size will be ignored.
		 * @return DriverCode The result of the operation.
		 */
		DriverCode DriverLoadBindPCI(uintptr_t DriverAddress, size_t Size, bool IsBuiltIn = false);

		/**
		 * @brief Load and bind a driver to an interrupt.
		 *
		 * This function will search for an interrupt with the given IRQ.
		 * If the interrupt is found, the driver will be loaded and bound to the interrupt.
		 *
		 * @param DriverAddress The address of the driver. The file will be copied to a new location.
		 * @param Size The size of the driver.
		 * @param IsBuiltIn If the driver is built-in, the @param DriverAddress will be @see BuiltInDriverInfo and the @param Size will be ignored.
		 * @return DriverCode The result of the operation.
		 */
		DriverCode DriverLoadBindInterrupt(uintptr_t DriverAddress, size_t Size, bool IsBuiltIn = false);

		/**
		 * @brief Load and bind a driver to an input device.
		 *
		 * This function will attach the driver to the input device.
		 *
		 * @param DriverAddress The address of the driver. The file will be copied to a new location.
		 * @param Size The size of the driver.
		 * @param IsBuiltIn If the driver is built-in, the @param DriverAddress will be @see BuiltInDriverInfo and the @param Size will be ignored.
		 * @return DriverCode The result of the operation.
		 */
		DriverCode DriverLoadBindInput(uintptr_t DriverAddress, size_t Size, bool IsBuiltIn = false);

		/**
		 * @brief Load and bind a driver to a process.
		 *
		 * This function will attach the driver to the process.
		 *
		 * @param DriverAddress The address of the driver. The file will be copied to a new location.
		 * @param Size The size of the driver.
		 * @param IsBuiltIn If the driver is built-in, the @param DriverAddress will be @see BuiltInDriverInfo and the @param Size will be ignored.
		 * @return DriverCode The result of the operation.
		 */
		DriverCode DriverLoadBindProcess(uintptr_t DriverAddress, size_t Size, bool IsBuiltIn = false);

		/**
		 * @brief Get the currently loaded drivers.
		 *
		 * This function returns a clone of the drivers vector.
		 * This means that the vector can be modified without affecting the drivers.
		 *
		 * @return std::vector<DriverFile> A clone of the drivers vector.
		 */
		std::vector<DriverFile> GetDrivers() { return Drivers; }

		/* Reserved by the kernel */
		void Panic();

		/**
		 * @brief Unload all drivers.
		 *
		 * This function will unload all drivers.
		 */
		void UnloadAllDrivers();

		/**
		 * @brief Unload a driver.
		 *
		 * This function will unload a driver with the given driver unique ID.
		 * It will free the memory and remove the driver from the drivers vector.
		 *
		 * @param DUID The driver unique ID.
		 * @return true If the driver was found and unloaded successfully, false otherwise.
		 */
		bool UnloadDriver(unsigned long DUID);

		/**
		 * @brief Send a callback to a driver.
		 *
		 * This function will send a callback to a driver with the given driver unique ID.
		 * This is used to communicate with drivers.
		 *
		 * @param DUID The driver unique ID.
		 * @param KCB The @see KernelCallback structure.
		 * @return int The result of the operation.
		 */
		int IOCB(unsigned long DUID, /* KernelCallback */ void *KCB);

		/**
		 * @brief Load a driver.
		 * @param DriverAddress The address of the driver file.
		 * @param Size The size of the driver file.
		 * @return DriverCode The result of the operation.
		 */
		DriverCode LoadDriver(uintptr_t DriverAddress, size_t Size);

		/* Reserved by the kernel */
		void LoadDrivers();

		/* Reserved by the kernel */
		Driver();

		/* Reserved by the kernel */
		~Driver();
	};
}

#endif // !__FENNIX_KERNEL_DRIVER_H__
