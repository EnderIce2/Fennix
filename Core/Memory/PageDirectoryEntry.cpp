#include <memory.hpp>

namespace Memory
{
    void PageDirectoryEntry::AddFlag(uint64_t Flag) { this->Value.raw |= Flag; }
    void PageDirectoryEntry::RemoveFlags(uint64_t Flag) { this->Value.raw &= ~Flag; }
    void PageDirectoryEntry::ClearFlags() { this->Value.raw = 0; }
    void PageDirectoryEntry::SetFlag(uint64_t Flag, bool Enabled)
    {
        this->Value.raw = 0;
        if (Enabled)
            this->Value.raw |= Flag;
    }
    bool PageDirectoryEntry::GetFlag(uint64_t Flag) { return (this->Value.raw & Flag) > 0 ? true : false; }
    uint64_t PageDirectoryEntry::GetFlag() { return this->Value.raw; }
    void PageDirectoryEntry::SetAddress(uint64_t Address)
    {
#if defined(__amd64__)
        Address &= 0x000000FFFFFFFFFF;
        this->Value.raw &= 0xFFF0000000000FFF;
        this->Value.raw |= (Address << 12);
#elif defined(__i386__)
        Address &= 0x000FFFFF;
        this->Value.raw &= 0xFFC00003;
        this->Value.raw |= (Address << 12);
#elif defined(__aarch64__)
        Address &= 0x000000FFFFFFFFFF;
        this->Value.raw &= 0xFFF0000000000FFF;
        this->Value.raw |= (Address << 12);
#endif
    }
    uint64_t PageDirectoryEntry::GetAddress()
    {
#if defined(__amd64__)
        return (this->Value.raw & 0x000FFFFFFFFFF000) >> 12;
#elif defined(__i386__)
        return (this->Value.raw & 0x003FFFFF000) >> 12;
#elif defined(__aarch64__)
        return (this->Value.raw & 0x000FFFFFFFFFF000) >> 12;
#endif
    }
}
