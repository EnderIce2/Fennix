#include <memory.hpp>

#include <filesystem.hpp>
#include <signal.hpp>
#include <utsname.h>
#include <time.h>

namespace Memory
{
	void PageTable::Update()
	{
#if defined(__amd64__) || defined(__i386__)
		asmv("mov %0, %%cr3" ::"r"(this));
#elif defined(__aarch64__)
		asmv("msr ttbr0_el1, %0" ::"r"(this));
#endif
	}

	PageTable *PageTable::Fork()
	{
		PageTable *NewTable = (PageTable *)KernelAllocator.RequestPages(TO_PAGES(sizeof(PageTable)));
		// memset(NewTable, 0, sizeof(PageTable));
		// CreatePageTable(NewTable);
		memcpy(NewTable, this, sizeof(PageTable));

		debug("Forking page table %#lx to %#lx", this, NewTable);
#if defined(__amd64__)
		for (size_t i = 0; i < sizeof(Entries) / sizeof(Entries[0]); i++)
		{
			PageMapLevel4 *PML4 = &Entries[i];
			PageMapLevel4 *NewPML4 = &NewTable->Entries[i];
			if (!PML4->Present)
				continue;

			PageDirectoryPointerTableEntryPtr *ptrPDPT = (PageDirectoryPointerTableEntryPtr *)(PML4->GetAddress() << 12);
			PageDirectoryPointerTableEntryPtr *ptrNewPDPT = (PageDirectoryPointerTableEntryPtr *)KernelAllocator.RequestPage();
			NewPML4->SetAddress((uintptr_t)ptrNewPDPT >> 12);
			for (size_t j = 0; j < sizeof(ptrPDPT->Entries) / sizeof(ptrPDPT->Entries[0]); j++)
			{
				PageDirectoryPointerTableEntry *PDPT = &ptrPDPT->Entries[j];
				PageDirectoryPointerTableEntry *NewPDPT = &ptrNewPDPT->Entries[j];
				*NewPDPT = *PDPT;

				if (!PDPT->Present)
					continue;
				if (PDPT->PageSize)
					continue;

				PageDirectoryEntryPtr *ptrPDE = (PageDirectoryEntryPtr *)(PDPT->GetAddress() << 12);
				PageDirectoryEntryPtr *ptrNewPDE = (PageDirectoryEntryPtr *)KernelAllocator.RequestPage();
				NewPDPT->SetAddress((uintptr_t)ptrNewPDE >> 12);
				for (size_t k = 0; k < sizeof(ptrPDE->Entries) / sizeof(ptrPDE->Entries[0]); k++)
				{
					PageDirectoryEntry *PDE = &ptrPDE->Entries[k];
					PageDirectoryEntry *NewPDE = &ptrNewPDE->Entries[k];
					*NewPDE = *PDE;

					if (!PDE->Present)
						continue;
					if (PDE->PageSize)
						continue;

					PageTableEntryPtr *ptrPTE = (PageTableEntryPtr *)(PDE->GetAddress() << 12);
					PageTableEntryPtr *ptrNewPTE = (PageTableEntryPtr *)KernelAllocator.RequestPage();
					NewPDE->SetAddress((uintptr_t)ptrNewPTE >> 12);
					for (size_t l = 0; l < sizeof(ptrPTE->Entries) / sizeof(ptrPTE->Entries[0]); l++)
					{
						PageTableEntry *PTE = &ptrPTE->Entries[l];
						PageTableEntry *NewPTE = &ptrNewPTE->Entries[l];
						*NewPTE = *PTE;
					}
				}
			}
		}
#else
#warning "PageTable::Fork() not implemented for other architectures"
#endif

		debug("Forked page table %#lx to %#lx", this, NewTable);
		return NewTable;
	}

	/* We can't have Memory::Virtual in the header */
	void *PageTable::__getPhysical(void *Address)
	{
		Virtual vmm(this);
		void *PhysAddr = vmm.GetPhysical((void *)Address);
		return PhysAddr;
	}
}
