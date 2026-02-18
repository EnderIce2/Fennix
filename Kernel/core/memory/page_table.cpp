#include <memory.hpp>

#include <fs/vfs.hpp>
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
#elif defined(__arm__)
		asmv("mcr p15, 0, %0, c2, c0, 0" : : "r"(this) : "memory"); // TTBR0
		asmv("dsb; isb");
#elif defined(__riscv) || defined(__riscv64)
		uintptr_t satp_val = ((uintptr_t)this >> 12) | (8UL << 60); /* MODE=8 for Sv39 */
		asmv("csrw satp, %0; sfence.vma" : : "r"(satp_val) : "memory");
#else
#error "PageTable::Update() not implemented for this architecture"
#endif
	}

	PageTable *PageTable::Fork()
	{
		PageTable *newTable = (PageTable *)KernelAllocator.RequestPages(TO_PAGES(sizeof(PageTable)));
		memcpy(newTable, this, sizeof(PageTable));

		debug("Forking page table %#lx to %#lx", this, newTable);

#if defined(__amd64__)
		for (size_t i = 0; i < numof(this->Entries); i++)
		{
			PageMapLevel4 *oldPML4 = &this->Entries[i];
			PageMapLevel4 *newPML4 = &newTable->Entries[i];
			if (!oldPML4->Present)
				continue;

			PageDirectoryPointerTableEntryPtr *ptrOldPDPT = (PageDirectoryPointerTableEntryPtr *)(oldPML4->GetAddress() << 12);
			PageDirectoryPointerTableEntryPtr *ptrNewPDPT = (PageDirectoryPointerTableEntryPtr *)KernelAllocator.RequestPage();
			newPML4->SetAddress((uintptr_t)ptrNewPDPT >> 12);

			for (size_t j = 0; j < numof(ptrOldPDPT->Entries); j++)
			{
				PageDirectoryPointerTableEntry *oldPDPT = &ptrOldPDPT->Entries[j];
				PageDirectoryPointerTableEntry *newPDPT = &ptrNewPDPT->Entries[j];
				*newPDPT = *oldPDPT;

				if (!oldPDPT->Present)
					continue;
				if (oldPDPT->PageSize)
					continue;

				PageDirectoryEntryPtr *ptrOldPDE = (PageDirectoryEntryPtr *)(oldPDPT->GetAddress() << 12);
				PageDirectoryEntryPtr *ptrNewPDE = (PageDirectoryEntryPtr *)KernelAllocator.RequestPage();
				newPDPT->SetAddress((uintptr_t)ptrNewPDE >> 12);

				for (size_t k = 0; k < numof(ptrOldPDE->Entries); k++)
				{
					PageDirectoryEntry *oldPDE = &ptrOldPDE->Entries[k];
					PageDirectoryEntry *newPDE = &ptrNewPDE->Entries[k];
					*newPDE = *oldPDE;

					if (!oldPDE->Present)
						continue;
					if (oldPDE->PageSize)
						continue;

					PageTableEntryPtr *ptrOldPTE = (PageTableEntryPtr *)(oldPDE->GetAddress() << 12);
					PageTableEntryPtr *ptrNewPTE = (PageTableEntryPtr *)KernelAllocator.RequestPage();
					newPDE->SetAddress((uintptr_t)ptrNewPTE >> 12);

					for (size_t l = 0; l < numof(ptrOldPTE->Entries); l++)
					{
						PageTableEntry *oldPTE = &ptrOldPTE->Entries[l];
						PageTableEntry *newPTE = &ptrNewPTE->Entries[l];
						*newPTE = *oldPTE;
					}
				}
			}
		}
#else
#warning "PageTable::Fork() not implemented for this architecture"
#endif

		debug("Forked page table %#lx to %#lx", this, newTable);
		return newTable;
	}

	/* We can't have Memory::Virtual in the header */
	void *PageTable::__getPhysical(void *Address)
	{
		Virtual vmm(this);
		void *addr = vmm.GetPhysical(Address);
		return addr;
	}
}
