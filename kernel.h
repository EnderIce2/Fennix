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

#ifndef __FENNIX_KERNEL_KERNEL_H__
#define __FENNIX_KERNEL_KERNEL_H__

#include <types.h>

#include <boot/binfo.h>
#ifdef __cplusplus
#include <filesystem.hpp>
#include <display.hpp>
#include <symbols.hpp>
#include <kconfig.hpp>
#include <net/nc.hpp>
#include <module.hpp>
#include <power.hpp>
#include <task.hpp>
#include <time.hpp>
#include <disk.hpp>
#include <pci.hpp>
#endif

extern struct BootInfo bInfo;
extern struct KernelConfig Config;
extern bool DebuggerIsAttached;
#ifdef __cplusplus

extern Video::Display *Display;
extern SymbolResolver::Symbols *KernelSymbolTable;
extern Power::Power *PowerManager;
extern Time::time *TimeManager;
extern PCI::PCI *PCIManager;
extern vfs::Virtual *fs;
extern vfs::Node *DevFS;
extern vfs::Node *MntFS;
extern vfs::Node *ProcFS;
extern vfs::Node *VarLogFS;
extern Tasking::Task *TaskManager;

extern Disk::Manager *DiskManager;
extern Module::Module *ModuleManager;
extern NetworkInterfaceManager::NetworkInterface *NIManager;

#endif // __cplusplus

EXTERNC void putchar(char c);
EXTERNC void KPrint(const char *format, ...);
EXTERNC void Entry(struct BootInfo *Info);
EXTERNC void BeforeShutdown(bool Reboot);
EXTERNC void TaskingPanic();

EXTERNC void BootLogoAnimationThread();
EXTERNC void ExitLogoAnimationThread();

EXTERNC void KernelMainThread();
EXTERNC void KernelShutdownThread(bool Reboot);
EXTERNC void KST_Reboot();
EXTERNC void KST_Shutdown();

#endif // !__FENNIX_KERNEL_KERNEL_H__
