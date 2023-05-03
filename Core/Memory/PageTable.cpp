#include <memory.hpp>

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
}
