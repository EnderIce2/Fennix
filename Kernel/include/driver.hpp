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

#include <interface/driver.h>
#include <interface/input.h>
#include <filesystem.hpp>
#include <unordered_map>
#include <memory.hpp>
#include <ints.hpp>
#include <lock.hpp>
#include <task.hpp>
#include <ring.hpp>
#include <debug.h>
#include <cpu.hpp>
#include <pci.hpp>
#include <vector>
#include <bitset>
#include <elf.h>
#include <io.h>
#include <list>

namespace Driver
{
	char GetScanCode(uint8_t ScanCode, bool Upper);
	bool IsValidChar(uint8_t ScanCode);

	struct DriverHandlers
	{
		const InodeOperations *Ops = nullptr;
		struct Inode *Node = nullptr;
		RingBuffer<InputReport> *InputReports;
	};

	struct DriverObject
	{
		uintptr_t BaseAddress = 0;
		uintptr_t EntryPoint = 0;
		Memory::VirtualMemoryArea *vma;

		/* Path has the same pointer as in the Node */
		std::string Path;
		std::unordered_map<uint8_t, void *> *InterruptHandlers;
		std::unordered_map<dev_t, DriverHandlers> *DeviceOperations;
		dev_t ID = 0;

		char Name[32] = {'\0'};
		char Description[64] = {'\0'};
		char Author[32] = {'\0'};
		struct
		{
			int Major, Minor, Patch;
		} Version = {0, 0, 0};
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

		/**
		 * 0 - generic null/zero/random/etc devices
		 * 1 - input/... devices
		 */
		dev_t DriverIDCounter = 2;
		FileNode *devNode = nullptr;
		FileNode *devInputNode = nullptr;


		int LoadDriverFile(DriverObject &Drv, FileNode *File);

		void InitializeDaemonFS();

		dev_t RegisterInputDevice(std::unordered_map<dev_t, DriverHandlers> *, dev_t, size_t, const InodeOperations *);
		dev_t RegisterBlockDevice(std::unordered_map<dev_t, DriverHandlers> *, dev_t, size_t, const InodeOperations *);

	public:
		RingBuffer<KeyboardReport> GlobalKeyboardInputReports;
		RingBuffer<MouseReport> GlobalMouseInputReports;

		struct DeviceInode
		{
			struct Inode Node;
			FileNode *Parent;
			Inode *ParentInode;
			std::string Name;
			std::vector<DeviceInode *> Children;
		};

		std::unordered_map<dev_t, DriverObject> &
		GetDrivers() { return Drivers; }

		void Daemon();
		void PreloadDrivers();
		void LoadAllDrivers();
		void UnloadAllDrivers();
		void Panic();

		/** Prefixes:
		 * - dsk (any disk device)
		 *    - dsk0p0 (disk 0, partition 0)
		 * - blk (block device)
		 * - eth (Ethernet device)
		 * - wlan (Wireless LAN device)
		 * - lo (Loopback device)
		 * - kb (Keyboard device)
		 * - ms (Mouse device)
		 * - js (Joystick device)
		 * - tp (Touchpad device)
		 * - tc (Touchscreen device)
		 * - cam (Camera device)
		 * - spk (Speaker device)
		 * - mic (Microphone device)
		 * - snd (Sound device)
		 * - tty (Serial device)
		 * - lp (Parallel device)
		 * - gpu (Graphics device)
		 * - fb (Framebuffer device)
		 * - usb (USB device)
		 *    - usb0dsk0p0 (USB 0, disk 0, partition 0; for USB storage)
		 */
		dev_t CreateIncrementalDevice(dev_t DriverID, const std::string &Prefix, mode_t Mode, InodeOperations *Ops);

		dev_t RegisterDevice(dev_t DriverID, DeviceType Type, const InodeOperations *Operations);
		int ReportInputEvent(dev_t DriverID, InputReport *Report);

		int UnregisterDevice(dev_t DriverID, dev_t Device);

		void *AllocateMemory(dev_t DriverID, size_t Pages);
		void FreeMemory(dev_t DriverID, void *Pointer, size_t Pages);

		Manager();
		~Manager();
	};

	void ManagerDaemonWrapper();
}

void *GetSymbolByName(const char *Name, int Version);

#endif // !__FENNIX_KERNEL_DRIVER_H__
