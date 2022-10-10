#ifndef __FENNIX_KERNEL_KERNEL_H__
#define __FENNIX_KERNEL_KERNEL_H__

#include <types.h>
#include <boot/binfo.h>

extern struct BootInfo *bInfo;

EXTERNC void Entry(struct BootInfo *Info);

#endif // !__FENNIX_KERNEL_KERNEL_H__
