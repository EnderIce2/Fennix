#ifndef __FENNIX_KERNEL_KERNEL_H__
#define __FENNIX_KERNEL_KERNEL_H__

#include <types.h>

#include <boot/binfo.h>
#ifdef __cplusplus
#include <net/nc.hpp>
#include <filesystem.hpp>
#include <display.hpp>
#include <symbols.hpp>
#include <kconfig.hpp>
#include <driver.hpp>
#include <power.hpp>
#include <task.hpp>
#include <time.hpp>
#include <disk.hpp>
#include <pci.hpp>
#endif

extern struct BootInfo *bInfo;
#ifdef __cplusplus
extern Video::Display *Display;
extern SymbolResolver::Symbols *KernelSymbolTable;
extern Power::Power *PowerManager;
extern PCI::PCI *PCIManager;
extern KernelConfig Config;
extern Tasking::Task *TaskManager;
extern Time::time *TimeManager;
extern FileSystem::Virtual *vfs;
extern Driver::Driver *DriverManager;
extern Disk::Manager *DiskManager;
extern NetworkInterfaceManager::NetworkInterface *NIManager;

#define PEXIT(Code) TaskManager->GetCurrentProcess()->ExitCode = Code
#define TEXIT(Code) TaskManager->GetCurrentThread()->ExitCode = Code

#endif

EXTERNC void putchar(char c);
EXTERNC void KPrint(const char *format, ...);
EXTERNC void Entry(struct BootInfo *Info);
EXTERNC void BeforeShutdown();
EXTERNC void TaskingPanic();

EXTERNC void KernelMainThread();
EXTERNC void KernelShutdownThread(bool Reboot);

#endif // !__FENNIX_KERNEL_KERNEL_H__
