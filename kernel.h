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
#include <driver.hpp>
#include <power.hpp>
#include <task.hpp>
#include <time.hpp>
#include <disk.hpp>
#include <pci.hpp>
#endif

extern struct BootInfo bInfo;
extern bool DebuggerIsAttached;
#ifdef __cplusplus

#ifdef DEBUG
#define MEM_TRK_MAX_SIZE 0x100
extern bool EnableExternalMemoryTracer;
extern char mExtTrkLog[];
extern LockClass mExtTrkLock;
#endif

extern Video::Display *Display;
extern SymbolResolver::Symbols *KernelSymbolTable;
extern Power::Power *PowerManager;
extern PCI::PCI *PCIManager;
extern KernelConfig Config;
extern Tasking::Task *TaskManager;
extern Time::time *TimeManager;
extern VirtualFileSystem::Virtual *vfs;
extern Driver::Driver *DriverManager;
extern Disk::Manager *DiskManager;
extern NetworkInterfaceManager::NetworkInterface *NIManager;
extern VirtualFileSystem::Node *DevFS;
extern VirtualFileSystem::Node *MntFS;
extern VirtualFileSystem::Node *ProcFS;
extern VirtualFileSystem::Node *VarLogFS;

#endif // __cplusplus

EXTERNC void putchar(char c);
EXTERNC void KPrint(const char *format, ...);
EXTERNC void Entry(struct BootInfo *Info);
EXTERNC void BeforeShutdown(bool Reboot);
EXTERNC void TaskingPanic();

EXTERNC void KernelMainThread();
EXTERNC void KernelShutdownThread(bool Reboot);
EXTERNC void KST_Reboot();
EXTERNC void KST_Shutdown();

#endif // !__FENNIX_KERNEL_KERNEL_H__
