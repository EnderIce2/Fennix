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

	class SlaveDeviceFile : public vfs::Node
	{
	private:
		int /* DeviceDriverType */ DeviceType;

		std::list<uint8_t> KeyQueue;

	public:
		typedef int (*drvOpen_t)(dev_t, dev_t, int, mode_t);
		typedef int (*drvClose_t)(dev_t, dev_t);
		typedef size_t (*drvRead_t)(dev_t, dev_t, uint8_t *, size_t, off_t);
		typedef size_t (*drvWrite_t)(dev_t, dev_t, uint8_t *, size_t, off_t);
		typedef int (*drvIoctl_t)(dev_t, dev_t, unsigned long, void *);

		drvOpen_t Open;
		drvClose_t Close;
		drvRead_t Read;
		drvWrite_t Write;
		drvIoctl_t Ioctl;

		int open(int Flags, mode_t Mode) final;
		int close() final;
		size_t read(uint8_t *Buffer,
					size_t Size,
					off_t Offset) final;
		size_t write(uint8_t *Buffer,
					 size_t Size,
					 off_t Offset) final;
		int ioctl(unsigned long Request,
				  void *Argp) final;

		void ClearBuffers();

		int ReportKeyEvent(uint8_t ScanCode);

		SlaveDeviceFile(const char *Name, vfs::Node *Parent, int Type, vfs::NodeType NType);
		~SlaveDeviceFile();
	};

	class MasterDeviceFile : private vfs::Node
	{
	private:
		typedef dev_t maj_t;
		typedef dev_t min_t;
		char SlaveName[16];
		vfs::Node *SlaveParent;
		int /* DeviceDriverType */ DeviceType;
		min_t SlaveIDCounter = 0;

		typedef std::unordered_map<min_t, SlaveDeviceFile *> *Slaves;
		std::unordered_map<maj_t, Slaves> SlavesMap;

		std::list<uint8_t> RawKeyQueue;
		std::list<uint8_t> KeyQueue;
		bool UpperCase = false;
		bool CapsLock = false;

	public:
		typedef int (*drvOpen_t)(dev_t, dev_t, int, mode_t);
		typedef int (*drvClose_t)(dev_t, dev_t);
		typedef size_t (*drvRead_t)(dev_t, dev_t, uint8_t *, size_t, off_t);
		typedef size_t (*drvWrite_t)(dev_t, dev_t, uint8_t *, size_t, off_t);
		typedef int (*drvIoctl_t)(dev_t, dev_t, unsigned long, void *);

		int open(int Flags, mode_t Mode) final;
		int close() final;
		size_t read(uint8_t *Buffer,
					size_t Size,
					off_t Offset) final;
		size_t write(uint8_t *Buffer,
					 size_t Size,
					 off_t Offset) final;
		int ioctl(unsigned long Request,
				  void *Argp) final;

		void ClearBuffers();

		int ReportKeyEvent(maj_t ID, min_t MinorID, uint8_t ScanCode);
		int ReportMouseEvent(maj_t ID, min_t MinorID,
							 bool LeftButton, bool RightButton, bool MiddleButton,
							 bool Button4, bool Button5, bool Button6,
							 bool Button7, bool Button8,
							 uintptr_t X, uintptr_t Y, int8_t Z, bool Relative);

		int ReportNetworkPacket(maj_t ID, min_t MinorID, void *Buffer, size_t Size);

		int NewBlock(maj_t ID, min_t MinorID, drvOpen_t Open, drvClose_t Close,
					 drvRead_t Read, drvWrite_t Write, drvIoctl_t Ioctl);

		int NewAudio(maj_t ID, min_t MinorID, drvOpen_t Open, drvClose_t Close,
					 drvRead_t Read, drvWrite_t Write, drvIoctl_t Ioctl);

		int NewNet(maj_t ID, min_t MinorID, drvOpen_t Open, drvClose_t Close,
				   drvRead_t Read, drvWrite_t Write, drvIoctl_t Ioctl);

		dev_t Register(maj_t ID);
		int Unregister(maj_t ID, min_t MinorID);

		MasterDeviceFile(const char *MasterName,
						 const char *SlaveName,
						 vfs::Node *Parent,
						 int Type);
		~MasterDeviceFile();
	};

	struct DriverObject
	{
		uintptr_t BaseAddress = 0;
		uintptr_t EntryPoint = 0;
		Memory::VirtualMemoryArea *vma = nullptr;

		/* Path has the same pointer as in the Node */
		const char *Path = nullptr;
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
		dev_t MajorIDCounter = 0;

		int LoadDriverFile(uintptr_t &EntryPoint,
						   uintptr_t &BaseAddress,
						   Memory::VirtualMemoryArea *dVma,
						   vfs::RefNode *rDrv);

	public:
		MasterDeviceFile *InputMouseDev = nullptr;
		MasterDeviceFile *InputKeyboardDev = nullptr;

		MasterDeviceFile *BlockSATADev = nullptr;
		MasterDeviceFile *BlockHDDev = nullptr;
		MasterDeviceFile *BlockNVMeDev = nullptr;

		MasterDeviceFile *AudioDev = nullptr;

		MasterDeviceFile *NetDev = nullptr;

		std::unordered_map<dev_t, DriverObject> &
		GetDrivers() { return Drivers; }

		void LoadAllDrivers();
		void UnloadAllDrivers();
		void Panic();

		Manager();
		~Manager();
	};

	void PopulateDriverAPI(void *API);
}

#endif // !__FENNIX_KERNEL_DRIVER_H__
