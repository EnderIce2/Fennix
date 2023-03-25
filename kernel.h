#ifndef __FENNIX_KERNEL_KERNEL_H__
#define __FENNIX_KERNEL_KERNEL_H__

#include <types.h>

#include <boot/binfo.h>
#ifdef __cplusplus
#include <filesystem.hpp>
#include <recovery.hpp>
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

extern struct BootInfo *bInfo;
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
extern VirtualFileSystem::Virtual *bootanim_vfs;
extern Driver::Driver *DriverManager;
extern Disk::Manager *DiskManager;
extern NetworkInterfaceManager::NetworkInterface *NIManager;
extern Recovery::KernelRecovery *RecoveryScreen;
extern VirtualFileSystem::Node *DevFS;
extern VirtualFileSystem::Node *MntFS;
extern VirtualFileSystem::Node *ProcFS;

#define PEXIT(Code) TaskManager->GetCurrentProcess()->ExitCode = Code
#define TEXIT(Code) TaskManager->GetCurrentThread()->ExitCode = Code

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
