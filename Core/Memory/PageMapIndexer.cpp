#include <memory.hpp>

namespace Memory
{
        Virtual::PageMapIndexer::PageMapIndexer(uint64_t VirtualAddress)
        {
#if defined(__amd64__)
                uint64_t Address = VirtualAddress;
                Address >>= 12;
                this->PIndex = Address & 0x1FF;
                Address >>= 9;
                this->PTIndex = Address & 0x1FF;
                Address >>= 9;
                this->PDIndex = Address & 0x1FF;
                Address >>= 9;
                this->PDPIndex = Address & 0x1FF;
#elif defined(__i386__)
                uint64_t Address = VirtualAddress;
                Address >>= 12;
                this->PIndex = Address & 0x3FF;
                Address >>= 10;
                this->PTIndex = Address & 0x3FF;
                Address >>= 10;
                this->PDIndex = Address & 0x3FF;
#elif defined(__aarch64__)
#endif
        }
}
