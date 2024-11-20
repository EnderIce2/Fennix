#include <memory.hpp>

namespace Memory
{
        Virtual::PageMapIndexer::PageMapIndexer(uint64_t VirtualAddress)
        {
#if defined(__amd64__)
                uint64_t Address = VirtualAddress;
                Address >>= 12;
                this->PTEIndex = Address & 0x1FF;
                Address >>= 9;
                this->PDEIndex = Address & 0x1FF;
                Address >>= 9;
                this->PDPTEIndex = Address & 0x1FF;
                Address >>= 9;
                this->PMLIndex = Address & 0x1FF;
#elif defined(__i386__)
                uint64_t Address = VirtualAddress;
                Address >>= 12;
                this->PTEIndex = Address & 0x3FF;
                Address >>= 10;
                this->PDEIndex = Address & 0x3FF;
                Address >>= 10;
                this->PDPTEIndex = Address & 0x3FF;
#elif defined(__aarch64__)
#endif
        }
}
