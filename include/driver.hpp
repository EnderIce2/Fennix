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

#include <filesystem.hpp>
#include <unordered_map>
#include <memory.hpp>
#include <ints.hpp>
#include <lock.hpp>
#include <task.hpp>
#include <debug.h>
#include <cpu.hpp>
#include <pci.hpp>
#include <vector>
#include <io.h>
#include <list>

namespace Driver
{
	char GetScanCode(uint8_t ScanCode, bool Upper);
	bool IsValidChar(uint8_t ScanCode);

	struct DriverObject
	{
		uintptr_t BaseAddress = 0;
		uintptr_t EntryPoint = 0;
		Memory::VirtualMemoryArea *vma = nullptr;

		/* Path has the same pointer as in the Node */
		std::string Path;
		std::unordered_map<uint8_t, void *> *InterruptHandlers;

		char Name[32] = {'\0'};
		char Description[64] = {'\0'};
		char Author[32] = {'\0'};
		char Version[16] = {'\0'};
		char License[32] = {'\0'};
		bool Initialized = false;
		int ErrorCode = 0;

		int (*Entry)() = nullptr;
		int (*Final)() = nullptr;
		int (*Panic)() = nullptr;
		int (*Probe)() = nullptr;
	};

	class Manager
	{
	private:
		NewLock(ModuleInitLock);
		std::unordered_map<dev_t, DriverObject> Drivers;
		dev_t DriverIDCounter = 0;

		int LoadDriverFile(uintptr_t &EntryPoint,
						   uintptr_t &BaseAddress,
						   Memory::VirtualMemoryArea *dVma,
						   FileNode *rDrv);

	public:
		std::unordered_map<dev_t, DriverObject> &
		GetDrivers() { return Drivers; }

		void PreloadDrivers();
		void LoadAllDrivers();
		void UnloadAllDrivers();
		void Panic();

		Manager();
		~Manager();
	};

	void PopulateDriverAPI(void *API);
}

#endif // !__FENNIX_KERNEL_DRIVER_H__
