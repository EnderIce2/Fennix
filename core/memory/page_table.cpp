#include <memory.hpp>

#include <filesystem.hpp>

namespace Memory
{
	void PageTable::Update()
	{
#if defined(a86)
		asmv("mov %0, %%cr3" ::"r"(this));
#elif defined(aa64)
		asmv("msr ttbr0_el1, %0" ::"r"(this));
#endif
	}

	PageTable PageTable::Fork()
	{
		PageTable NewTable;
		memcpy(&NewTable, this, sizeof(PageTable));
		return NewTable;
	}

	template <typename T>
	T PageTable::Get(T Address)
	{
		Virtual vmm = Virtual(this);
		void *PhysAddr = vmm.GetPhysical((void *)Address);
		uintptr_t Diff = uintptr_t(Address);
		Diff &= 0xFFF;
		Diff = uintptr_t(PhysAddr) + Diff;
		return (T)Diff;
	}

	/* Templates */
	template struct stat *PageTable::Get<struct stat *>(struct stat *);
	template const char *PageTable::Get<const char *>(const char *);
	template const void *PageTable::Get<const void *>(const void *);
	template uintptr_t PageTable::Get<uintptr_t>(uintptr_t);
	template void *PageTable::Get<void *>(void *);
	/* ... */
}
