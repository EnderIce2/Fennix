#include <memory.hpp>

namespace Memory
{
    Virtual::PageMapIndexer::PageMapIndexer(uint64_t VirtualAddress)
    {
#if defined(__amd64__)
        this->PDPIndex = (VirtualAddress & ((uint64_t)0x1FF << 39)) >> 39;
        this->PDIndex = (VirtualAddress & ((uint64_t)0x1FF << 30)) >> 30;
        this->PTIndex = (VirtualAddress & ((uint64_t)0x1FF << 21)) >> 21;
        this->PIndex = (VirtualAddress & ((uint64_t)0x1FF << 12)) >> 12;
#elif defined(__i386__)
        this->PDIndex = (VirtualAddress & ((uint64_t)0x3FF << 22)) >> 22;
        this->PTIndex = (VirtualAddress & ((uint64_t)0x3FF << 12)) >> 12;
        this->PIndex = (VirtualAddress & ((uint64_t)0xFFF)) >> 0;
#elif defined(__aarch64__)
        this->PDIndex = (VirtualAddress & ((uint64_t)0x1FF << 30)) >> 30;
        this->PTIndex = (VirtualAddress & ((uint64_t)0x1FF << 21)) >> 21;
        this->PIndex = (VirtualAddress & ((uint64_t)0x1FF << 12)) >> 12;
#endif
    }
}
