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

#ifndef __FENNIX_KERNEL_MODULE_H__
#define __FENNIX_KERNEL_MODULE_H__

#include <types.h>

#include <memory.hpp>
#include <ints.hpp>
#include <lock.hpp>
#include <debug.h>
#include <cpu.hpp>
#include <pci.hpp>
#include <vector>

namespace Module
{
	enum ModuleCode
	{
		/* This must be the same as in mapi.hpp ModuleReturnCode */
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

		/* End of module-only errors */

		INVALID_FEX_HEADER,
		INVALID_MODULE_DATA,
		NOT_MODULE,
		MODULE_RETURNED_ERROR,
		UNKNOWN_MODULE_TYPE,
		UNKNOWN_MODULE_BIND_TYPE,
		PCI_DEVICE_NOT_FOUND,
		MODULE_CONFLICT
	};

	class ModuleInterruptHook;
	struct ModuleFile
	{
		bool Enabled = false;
		bool BuiltIn = false;
		unsigned int modUniqueID = 0;
		void *Address = nullptr;
		void *ExtendedHeaderAddress = nullptr;
		void *InterruptCallback = nullptr;
		Memory::VirtualMemoryArea *vma = nullptr;
		ModuleInterruptHook *InterruptHook[16]{};

		bool operator==(const ModuleFile &Other) const
		{
			return modUniqueID == Other.modUniqueID;
		}
	};

	struct BuiltInModuleInfo
	{
		int (*EntryPoint)(void *);
		void *ExtendedHeader;
	};

	class ModuleInterruptHook : public Interrupts::Handler
	{
	private:
		NewLock(DriverInterruptLock);

		ModuleFile Handle;
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
		ModuleInterruptHook(int Interrupt, ModuleFile Handle);
		virtual ~ModuleInterruptHook() = default;
	};

	class Module
	{
	private:
		NewLock(ModuleInitLock);

		std::vector<ModuleFile> Modules;
		unsigned int modUIDs = 0;
		/* If BuiltIn is true, the "fex" is the entry point. */
		ModuleCode CallModuleEntryPoint(void *fex, bool BuiltIn = false);

	public:
		/**
		 * @brief Load and bind a module to a PCI device.
		 *
		 * This function will search for a PCI device with the given VendorID and DeviceID.
		 * If the device is found, the module will be loaded and bound to the device.
		 *
		 * @param ModuleAddress The address of the module. The file will be copied to a new location.
		 * @param Size The size of the module.
		 * @param IsBuiltIn If the module is built-in, the @param ModuleAddress will be @see BuiltInModuleInfo and the @param Size will be ignored.
		 * @return ModuleCode The result of the operation.
		 */
		ModuleCode ModuleLoadBindPCI(uintptr_t ModuleAddress, size_t Size, bool IsBuiltIn = false);

		/**
		 * @brief Load and bind a module to an interrupt.
		 *
		 * This function will search for an interrupt with the given IRQ.
		 * If the interrupt is found, the module will be loaded and bound to the interrupt.
		 *
		 * @param ModuleAddress The address of the module. The file will be copied to a new location.
		 * @param Size The size of the module.
		 * @param IsBuiltIn If the module is built-in, the @param ModuleAddress will be @see BuiltInModuleInfo and the @param Size will be ignored.
		 * @return ModuleCode The result of the operation.
		 */
		ModuleCode ModuleLoadBindInterrupt(uintptr_t ModuleAddress, size_t Size, bool IsBuiltIn = false);

		/**
		 * @brief Load and bind a module to an input device.
		 *
		 * This function will attach the module to the input device.
		 *
		 * @param ModuleAddress The address of the module. The file will be copied to a new location.
		 * @param Size The size of the module.
		 * @param IsBuiltIn If the module is built-in, the @param ModuleAddress will be @see BuiltInModuleInfo and the @param Size will be ignored.
		 * @return ModuleCode The result of the operation.
		 */
		ModuleCode ModuleLoadBindInput(uintptr_t ModuleAddress, size_t Size, bool IsBuiltIn = false);

		/**
		 * @brief Load and bind a module to a process.
		 *
		 * This function will attach the module to the process.
		 *
		 * @param ModuleAddress The address of the module. The file will be copied to a new location.
		 * @param Size The size of the module.
		 * @param IsBuiltIn If the module is built-in, the @param ModuleAddress will be @see BuiltInModuleInfo and the @param Size will be ignored.
		 * @return ModuleCode The result of the operation.
		 */
		ModuleCode ModuleLoadBindProcess(uintptr_t ModuleAddress, size_t Size, bool IsBuiltIn = false);

		/**
		 * @brief Get the currently loaded drivers.
		 *
		 * This function returns a clone of the drivers vector.
		 * This means that the vector can be modified without affecting the drivers.
		 *
		 * @return std::vector<ModuleFile> A clone of the drivers vector.
		 */
		std::vector<ModuleFile> GetModules() { return Modules; }

		/* Reserved by the kernel */
		void Panic();

		/**
		 * @brief Unload all drivers.
		 *
		 * This function will unload all drivers.
		 */
		void UnloadAllModules();

		/**
		 * @brief Unload a module.
		 *
		 * This function will unload a module with the given module unique ID.
		 * It will free the memory and remove the module from the drivers vector.
		 *
		 * @param id The module unique ID.
		 * @return true If the module was found and unloaded successfully, false otherwise.
		 */
		bool UnloadModule(unsigned long id);

		/**
		 * @brief Send a callback to a module.
		 *
		 * This function will send a callback to a module with the given module unique ID.
		 * This is used to communicate with drivers.
		 *
		 * @param id The module unique ID.
		 * @param KCB The @see KernelCallback structure.
		 * @return int The result of the operation.
		 */
		int IOCB(unsigned long id, /* KernelCallback */ void *KCB);

		/**
		 * @brief Load a module.
		 * @param fildes The file descriptor of the module file.
		 * @return ModuleCode The result of the operation.
		 */
		ModuleCode LoadModule(vfs::Node *fildes);

		/* Reserved by the kernel */
		void LoadModules();

		/* Reserved by the kernel */
		Module();

		/* Reserved by the kernel */
		~Module();
	};
}

#endif // !__FENNIX_KERNEL_MODULE_H__
