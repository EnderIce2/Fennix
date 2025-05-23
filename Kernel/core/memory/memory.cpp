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

#include <memory.hpp>

#include <convert.h>
#include <lock.hpp>
#include <debug.h>
#ifdef DEBUG
#include <uart.hpp>
#endif

#include "heap_allocators/Xalloc/Xalloc.hpp"
#include "heap_allocators/liballoc_1_1/liballoc_1_1.h"
#include "heap_allocators/rpmalloc/rpmalloc.h"
#include "../../kernel.h"

// #define DEBUG_ALLOCATIONS 1

#ifdef DEBUG_ALLOCATIONS
#define memdbg(m, ...)       \
	debug(m, ##__VA_ARGS__); \
	__sync
#else
#define memdbg(m, ...)
#endif

using namespace Memory;

Physical KernelAllocator;
Memory::KernelStackManager StackManager;
PageTable *KernelPageTable = nullptr;
bool Page1GBSupport = false;
bool PSESupport = false;

MemoryAllocatorType AllocatorType = MemoryAllocatorType::Pages;
Xalloc::V1 *XallocV1Allocator = nullptr;
Xalloc::V2 *XallocV2Allocator = nullptr;

#ifdef DEBUG
nif void tracepagetable(PageTable *pt)
{
	for (int i = 0; i < 512; i++)
	{
#if defined(__amd64__)
		if (pt->Entries[i].Present)
			debug("Entry %03d: %x %x %x %x %x %x %x %p-%#llx", i,
				  pt->Entries[i].Present, pt->Entries[i].ReadWrite,
				  pt->Entries[i].UserSupervisor, pt->Entries[i].WriteThrough,
				  pt->Entries[i].CacheDisable, pt->Entries[i].Accessed,
				  pt->Entries[i].ExecuteDisable, pt->Entries[i].Address << 12,
				  pt->Entries[i]);
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
	}
}
#endif

nif void MapEntries(PageTable *PT)
{
	debug("mapping %d memory entries", bInfo.Memory.Entries);
	Virtual vmm = Virtual(PT);

	for (uint64_t i = 0; i < bInfo.Memory.Entries; i++)
	{
		uintptr_t Base = r_cst(uintptr_t, bInfo.Memory.Entry[i].BaseAddress);
		size_t Length = bInfo.Memory.Entry[i].Length;

		debug("mapping %#lx-%#lx", Base, Base + Length);
		vmm.Map((void *)Base, (void *)Base, Length, RW);
	}

	/* Make sure 0x0 is unmapped (so we PF when nullptr is accessed) */
	vmm.Unmap((void *)0);
}

nif void MapFramebuffer(PageTable *PT)
{
	debug("Mapping Framebuffer");
	Virtual vmm = Virtual(PT);
	int itrfb = 0;
	while (1)
	{
		if (!bInfo.Framebuffer[itrfb].BaseAddress)
			break;

		size_t fbSize = bInfo.Framebuffer[itrfb].Pitch * bInfo.Framebuffer[itrfb].Height;
		fbSize = ALIGN_UP(fbSize, PAGE_SIZE);

#ifdef DEBUG
		if (DebuggerIsAttached)
			fbSize += 16 * PAGE_SIZE;
#endif

		if (PSESupport && Page1GBSupport)
		{
			vmm.OptimizedMap(bInfo.Framebuffer[itrfb].BaseAddress,
							 bInfo.Framebuffer[itrfb].BaseAddress,
							 fbSize, RW | G | KRsv);
		}
		else
		{
			vmm.Map(bInfo.Framebuffer[itrfb].BaseAddress,
					bInfo.Framebuffer[itrfb].BaseAddress,
					fbSize, RW | G | KRsv);
		}
		itrfb++;
	}
}

nif void MapKernel(PageTable *PT)
{
	debug("Mapping Kernel");

	/* RWX */
	uintptr_t BootstrapStart = (uintptr_t)&_bootstrap_start;
	uintptr_t BootstrapEnd = (uintptr_t)&_bootstrap_end;

	/* RX */
	uintptr_t KernelTextStart = (uintptr_t)&_kernel_text_start;
	uintptr_t KernelTextEnd = (uintptr_t)&_kernel_text_end;

	/* RW */
	uintptr_t KernelDataStart = (uintptr_t)&_kernel_data_start;
	uintptr_t KernelDataEnd = (uintptr_t)&_kernel_data_end;

	/* R */
	uintptr_t KernelRoDataStart = (uintptr_t)&_kernel_rodata_start;
	uintptr_t KernelRoDataEnd = (uintptr_t)&_kernel_rodata_end;

	/* RW */
	uintptr_t KernelBssStart = (uintptr_t)&_kernel_bss_start;
	uintptr_t KernelBssEnd = (uintptr_t)&_kernel_bss_end;

#ifdef DEBUG
	uintptr_t KernelStart = (uintptr_t)&_kernel_start;
	uintptr_t KernelEnd = (uintptr_t)&_kernel_end;
#endif
	uintptr_t KernelFileStart = (uintptr_t)bInfo.Kernel.FileBase;
	uintptr_t KernelFileEnd = KernelFileStart + bInfo.Kernel.Size;

	debug("Bootstrap: %#lx-%#lx", BootstrapStart, BootstrapEnd);
	debug("Kernel text: %#lx-%#lx", KernelTextStart, KernelTextEnd);
	debug("Kernel data: %#lx-%#lx", KernelDataStart, KernelDataEnd);
	debug("Kernel rodata: %#lx-%#lx", KernelRoDataStart, KernelRoDataEnd);
	debug("Kernel bss: %#lx-%#lx", KernelBssStart, KernelBssEnd);
	debug("Kernel: %#lx-%#lx", KernelStart, KernelEnd);
	debug("Kernel file: %#lx-%#lx", KernelFileStart, KernelFileEnd);

	debug("File size: %ld KiB", TO_KiB(bInfo.Kernel.Size));
	debug(".bootstrap size: %ld KiB", TO_KiB(BootstrapEnd - BootstrapStart));
	debug(".text size: %ld KiB", TO_KiB(KernelTextEnd - KernelTextStart));
	debug(".data size: %ld KiB", TO_KiB(KernelDataEnd - KernelDataStart));
	debug(".rodata size: %ld KiB", TO_KiB(KernelRoDataEnd - KernelRoDataStart));
	debug(".bss size: %ld KiB", TO_KiB(KernelBssEnd - KernelBssStart));

	uintptr_t BaseKernelMapAddress = (uintptr_t)bInfo.Kernel.PhysicalBase;
	debug("Base kernel map address: %#lx", BaseKernelMapAddress);
	uintptr_t k;
	Virtual vmm = Virtual(PT);

	/* Bootstrap section */
	if (BaseKernelMapAddress == BootstrapStart)
	{
		for (k = BootstrapStart; k < BootstrapEnd; k += PAGE_SIZE)
		{
			vmm.Map((void *)k, (void *)BaseKernelMapAddress, RW | G | KRsv);
			KernelAllocator.ReservePage((void *)BaseKernelMapAddress);
			BaseKernelMapAddress += PAGE_SIZE;
		}
	}
	else
	{
		trace("Ignoring bootstrap section.");
		/* Bootstrap section must be mapped at 0x100000. */
	}

	/* Text section */
	for (k = KernelTextStart; k < KernelTextEnd; k += PAGE_SIZE)
	{
		vmm.Map((void *)k, (void *)BaseKernelMapAddress, RW | G | KRsv);
		KernelAllocator.ReservePage((void *)BaseKernelMapAddress);
		BaseKernelMapAddress += PAGE_SIZE;
	}

	/* Data section */
	for (k = KernelDataStart; k < KernelDataEnd; k += PAGE_SIZE)
	{
		vmm.Map((void *)k, (void *)BaseKernelMapAddress, RW | G | KRsv);
		KernelAllocator.ReservePage((void *)BaseKernelMapAddress);
		BaseKernelMapAddress += PAGE_SIZE;
	}

	/* Read only data section */
	for (k = KernelRoDataStart; k < KernelRoDataEnd; k += PAGE_SIZE)
	{
		vmm.Map((void *)k, (void *)BaseKernelMapAddress, G | KRsv);
		KernelAllocator.ReservePage((void *)BaseKernelMapAddress);
		BaseKernelMapAddress += PAGE_SIZE;
	}

	/* Block starting symbol section */
	for (k = KernelBssStart; k < KernelBssEnd; k += PAGE_SIZE)
	{
		vmm.Map((void *)k, (void *)BaseKernelMapAddress, RW | G | KRsv);
		KernelAllocator.ReservePage((void *)BaseKernelMapAddress);
		BaseKernelMapAddress += PAGE_SIZE;
	}

	debug("Base kernel map address: %#lx", BaseKernelMapAddress);

	/* Kernel file */
	if (KernelFileStart != 0)
	{
		for (k = KernelFileStart; k < KernelFileEnd; k += PAGE_SIZE)
		{
			vmm.Map((void *)k, (void *)k, G | KRsv);
			KernelAllocator.ReservePage((void *)k);
		}
	}
	else
		info("Cannot determine kernel file address. Ignoring.");
}

nif void CreatePageTable(PageTable *pt)
{
	static int check_cpuid = 0;

	if (!check_cpuid++)
	{
		if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
		{
			CPU::x86::AMD::CPUID0x80000001 cpuid;
			PSESupport = cpuid.EDX.PSE;
			Page1GBSupport = cpuid.EDX.Page1GB;
		}
		else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
		{
			CPU::x86::Intel::CPUID0x00000001 cpuid;
			PSESupport = cpuid.EDX.PSE;
		}

		if (PSESupport)
		{
#if defined(__amd64__)
			CPU::x64::CR4 cr4 = CPU::x64::readcr4();
			cr4.PSE = 1;
			CPU::x64::writecr4(cr4);
#elif defined(__i386__)
			CPU::x32::CR4 cr4 = CPU::x32::readcr4();
			cr4.PSE = 1;
			CPU::x32::writecr4(cr4);
#elif defined(__aarch64__)
#endif
			trace("PSE Support Enabled");
		}

#ifdef DEBUG
		if (Page1GBSupport)
			debug("1GB Page Support Enabled");
#endif
	}

	/* TODO: Map faster */
	MapEntries(pt);
	MapFramebuffer(pt);
	MapKernel(pt);

#ifdef DEBUG
	tracepagetable(pt);
#endif
}

nif void InitializeMemoryManagement()
{
#ifdef DEBUG
#ifndef __i386__
	for (uint64_t i = 0; i < bInfo.Memory.Entries; i++)
	{
		uintptr_t Base = r_cst(uintptr_t, bInfo.Memory.Entry[i].BaseAddress);
		size_t Length = bInfo.Memory.Entry[i].Length;
		uintptr_t End = Base + Length;
		const char *Type = "Unknown";

		switch (bInfo.Memory.Entry[i].Type)
		{
		case likely(Usable):
			Type = "Usable";
			break;
		case Reserved:
			Type = "Reserved";
			break;
		case ACPIReclaimable:
			Type = "ACPI Reclaimable";
			break;
		case ACPINVS:
			Type = "ACPI NVS";
			break;
		case BadMemory:
			Type = "Bad Memory";
			break;
		case BootloaderReclaimable:
			Type = "Bootloader Reclaimable";
			break;
		case KernelAndModules:
			Type = "Kernel and Modules";
			break;
		case Framebuffer:
			Type = "Framebuffer";
			break;
		default:
			break;
		}

		debug("%02ld: %p-%p %s",
			  i,
			  Base,
			  End,
			  Type);
	}
#endif // __i386__
#endif // DEBUG
	trace("Initializing Physical Memory Manager");
	// KernelAllocator = Physical(); <- Already called in the constructor
	KernelAllocator.Init();
	debug("Memory Info:\n\n%lld MiB / %lld MiB (%lld MiB reserved)\n",
		  TO_MiB(KernelAllocator.GetUsedMemory()),
		  TO_MiB(KernelAllocator.GetTotalMemory() - KernelAllocator.GetReservedMemory()),
		  TO_MiB(KernelAllocator.GetReservedMemory()));

	/* -- Debugging --
		size_t bmap_size = KernelAllocator.GetPageBitmap().Size;
		for (size_t i = 0; i < bmap_size; i++)
		{
			bool idx = KernelAllocator.GetPageBitmap().Get(i);
			if (idx == true)
				debug("Page %04d: %#lx", i, i * PAGE_SIZE);
		}

		inf_loop debug("Alloc.: %#lx", KernelAllocator.RequestPage());
	*/

	trace("Initializing Virtual Memory Manager");
	KernelPageTable = (PageTable *)KernelAllocator.RequestPages(TO_PAGES(PAGE_SIZE + 1));
	memset(KernelPageTable, 0, PAGE_SIZE);

	CreatePageTable(KernelPageTable);

	trace("Applying new page table from address %#lx", KernelPageTable);
	CPU::PageTable(KernelPageTable);
	debug("Page table updated.");

	/* FIXME: Read kernel params */
	AllocatorType = Config.AllocatorType;

	switch (AllocatorType)
	{
	case MemoryAllocatorType::Pages:
		break;
	case MemoryAllocatorType::XallocV1:
	{
		XallocV1Allocator = new Xalloc::V1((void *)nullptr, false, false);
		trace("XallocV1 Allocator initialized at %#lx", XallocV1Allocator);
		break;
	}
	case MemoryAllocatorType::XallocV2:
	{
		XallocV2Allocator = new Xalloc::V2((void *)nullptr);
		trace("XallocV2 Allocator initialized at %#lx", XallocV2Allocator);
		break;
	}
	case MemoryAllocatorType::liballoc11:
		break;
	case MemoryAllocatorType::rpmalloc_:
	{
		trace("Using rpmalloc allocator");
		rpmalloc_initialize();
		break;
		rpmalloc_config_t config = {
			.memory_map = nullptr,
			.memory_unmap = nullptr,
			.error_callback = nullptr,
			.map_fail_callback = nullptr,
			.page_size = PAGE_SIZE,
			.span_size = 4 * 1024, /* 4 KiB */
			.span_map_count = 1,
			.enable_huge_pages = 0,
			.page_name = nullptr,
			.huge_page_name = nullptr};
		rpmalloc_initialize_config(&config);
		break;
	}
	default:
	{
		error("Unknown allocator type %d", AllocatorType);
		CPU::Stop();
	}
	}
}

void *malloc(size_t Size)
{
	if (Size == 0)
	{
		warn("Attempt to allocate 0 bytes");
		Size = 16;
	}

	memdbg("malloc(%d)->[%s]", Size,
		   KernelSymbolTable ? KernelSymbolTable->GetSymbol((uintptr_t)__builtin_return_address(0))
							 : "Unknown");

	void *ret = nullptr;
	switch (AllocatorType)
	{
	case MemoryAllocatorType::Pages:
	{
		ret = KernelAllocator.RequestPages(TO_PAGES(Size + 1));
		break;
	}
	case MemoryAllocatorType::XallocV1:
	{
		ret = XallocV1Allocator->malloc(Size);
		break;
	}
	case MemoryAllocatorType::XallocV2:
	{
		ret = XallocV2Allocator->malloc(Size);
		break;
	}
	case MemoryAllocatorType::liballoc11:
	{
		ret = PREFIX(malloc)(Size);
		break;
	}
	case MemoryAllocatorType::rpmalloc_:
	{
		ret = rpmalloc(Size);
		break;
	}
	default:
	{
		error("Unknown allocator type %d", AllocatorType);
		CPU::Stop();
	}
	}

	memset(ret, 0, Size);
	return ret;
}

void *calloc(size_t n, size_t Size)
{
	if (Size == 0)
	{
		warn("Attempt to allocate 0 bytes");
		Size = 16;
	}

	memdbg("calloc(%d, %d)->[%s]", n, Size,
		   KernelSymbolTable ? KernelSymbolTable->GetSymbol((uintptr_t)__builtin_return_address(0))
							 : "Unknown");

	void *ret = nullptr;
	switch (AllocatorType)
	{
	case MemoryAllocatorType::Pages:
	{
		ret = KernelAllocator.RequestPages(TO_PAGES(n * Size + 1));
		break;
	}
	case MemoryAllocatorType::XallocV1:
	{
		ret = XallocV1Allocator->calloc(n, Size);
		break;
	}
	case MemoryAllocatorType::XallocV2:
	{
		ret = XallocV2Allocator->calloc(n, Size);
		break;
	}
	case MemoryAllocatorType::liballoc11:
	{
		void *ret = PREFIX(calloc)(n, Size);
		return ret;
	}
	case MemoryAllocatorType::rpmalloc_:
	{
		ret = rpcalloc(n, Size);
		break;
	}
	default:
	{
		error("Unknown allocator type %d", AllocatorType);
		CPU::Stop();
	}
	}

	memset(ret, 0, n * Size);
	return ret;
}

void *realloc(void *Address, size_t Size)
{
	if (Size == 0)
	{
		warn("Attempt to allocate 0 bytes");
		Size = 16;
	}

	memdbg("realloc(%#lx, %d)->[%s]", Address, Size,
		   KernelSymbolTable ? KernelSymbolTable->GetSymbol((uintptr_t)__builtin_return_address(0))
							 : "Unknown");

	void *ret = nullptr;
	switch (AllocatorType)
	{
	case unlikely(MemoryAllocatorType::Pages):
	{
		ret = KernelAllocator.RequestPages(TO_PAGES(Size + 1)); // WARNING: Potential memory leak
		break;
	}
	case MemoryAllocatorType::XallocV1:
	{
		ret = XallocV1Allocator->realloc(Address, Size);
		break;
	}
	case MemoryAllocatorType::XallocV2:
	{
		ret = XallocV2Allocator->realloc(Address, Size);
		break;
	}
	case MemoryAllocatorType::liballoc11:
	{
		void *ret = PREFIX(realloc)(Address, Size);
		return ret;
	}
	case MemoryAllocatorType::rpmalloc_:
	{
		ret = rprealloc(Address, Size);
		break;
	}
	default:
	{
		error("Unknown allocator type %d", AllocatorType);
		CPU::Stop();
	}
	}

	memset(ret, 0, Size);
	return ret;
}

void free(void *Address)
{
	if (Address == nullptr)
	{
		warn("Attempt to free a null pointer");
		return;
	}

	memdbg("free(%#lx)->[%s]", Address,
		   KernelSymbolTable ? KernelSymbolTable->GetSymbol((uintptr_t)__builtin_return_address(0))
							 : "Unknown");

	switch (AllocatorType)
	{
	case unlikely(MemoryAllocatorType::Pages):
	{
		KernelAllocator.FreePage(Address); // WARNING: Potential memory leak
		break;
	}
	case MemoryAllocatorType::XallocV1:
	{
		XallocV1Allocator->free(Address);
		break;
	}
	case MemoryAllocatorType::XallocV2:
	{
		XallocV2Allocator->free(Address);
		break;
	}
	case MemoryAllocatorType::liballoc11:
	{
		PREFIX(free)
		(Address);
		break;
	}
	case MemoryAllocatorType::rpmalloc_:
	{
		rpfree(Address);
		break;
	}
	default:
	{
		error("Unknown allocator type %d", AllocatorType);
		CPU::Stop();
	}
	}
}
