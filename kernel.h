#ifndef __FENNIX_KERNEL_KERNEL_H__
#define __FENNIX_KERNEL_KERNEL_H__

#include <types.h>

#include <boot/binfo.h>
#ifdef __cplusplus
#include <display.hpp>
#include <symbols.hpp>
#include <power.hpp>
#include <pci.hpp>
#endif

extern struct BootInfo *bInfo;
#ifdef __cplusplus
extern Video::Display *Display;
extern SymbolResolver::Symbols *KernelSymbolTable;
extern Power::Power *PowerManager;
extern PCI::PCI *PCIManager;
#endif

EXTERNC void Entry(struct BootInfo *Info);

#endif // !__FENNIX_KERNEL_KERNEL_H__
