#ifndef __FENNIX_KERNEL_KERNEL_H__
#define __FENNIX_KERNEL_KERNEL_H__

#include <types.h>

#include <boot/binfo.h>
#ifdef __cplusplus
#include <display.hpp>
#include <symbols.hpp>
#include <kconfig.hpp>
#include <power.hpp>
#include <task.hpp>
#include <time.hpp>
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
#endif

EXTERNC void KPrint(const char *format, ...);
EXTERNC void Entry(struct BootInfo *Info);

EXTERNC void KernelMainThread();
EXTERNC void KernelShutdownThread(bool Reboot);

#endif // !__FENNIX_KERNEL_KERNEL_H__
