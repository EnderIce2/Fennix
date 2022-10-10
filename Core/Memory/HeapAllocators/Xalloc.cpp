#include "Xalloc.hpp"

namespace Xalloc
{
    class SmartSMAPClass
    {
    private:
        AllocatorV1 *allocator = nullptr;

    public:
        SmartSMAPClass(AllocatorV1 *allocator)
        {
            this->allocator = allocator;
            this->allocator->Xstac();
        }
        ~SmartSMAPClass() { this->allocator->Xclac(); }
    };
#define SmartSMAP SmartSMAPClass XALLOC_CONCAT(SmartSMAP##_, __COUNTER__)(this)

    AllocatorV1::AllocatorV1(void *Address, bool UserMode, bool SMAPEnabled)
    {
        SmartSMAP;
        void *Position = Address;
        UserMapping = UserMode;
        SMAPUsed = SMAPEnabled;
        for (Xuint64_t i = 0; i < 0x20; i++)
        {
            void *Page = Xalloc_REQUEST_PAGE();
            if (UserMapping)
                Xalloc_MAP_MEMORY(Position, Page, Xalloc_MAP_MEMORY_READ_WRITE | Xalloc_MAP_MEMORY_USER);
            else
                Xalloc_MAP_MEMORY(Position, Page, Xalloc_MAP_MEMORY_READ_WRITE);
            Xalloc_trace("Preallocate Heap Memory (%#llx-%#llx [%#llx])...", Position, (Xuint64_t)Position + Xalloc_PAGE_SIZE, Page);
            Position = (void *)((Xuint64_t)Position + Xalloc_PAGE_SIZE);
        }
        Xuint64_t HeapLength = 16 * Xalloc_PAGE_SIZE;
        this->HeapStart = Address;
        this->HeapEnd = (void *)((Xuint64_t)this->HeapStart + HeapLength);
        HeapSegment *StartSegment = (HeapSegment *)Address;
        StartSegment->Length = HeapLength - sizeof(HeapSegment);
        StartSegment->Next = nullptr;
        StartSegment->Last = nullptr;
        StartSegment->IsFree = true;
        this->LastSegment = StartSegment;
    }

    AllocatorV1::~AllocatorV1()
    {
        SmartSMAP;
        Xalloc_trace("Destructor not implemented yet.");
    }

    void AllocatorV1::ExpandHeap(Xuint64_t Length)
    {
        if (Length % Xalloc_PAGE_SIZE)
        {
            Length -= Length % Xalloc_PAGE_SIZE;
            Length += Xalloc_PAGE_SIZE;
        }
        Xuint64_t PageCount = Length / Xalloc_PAGE_SIZE;
        HeapSegment *NewSegment = (HeapSegment *)this->HeapEnd;
        for (Xuint64_t i = 0; i < PageCount; i++)
        {
            void *Page = Xalloc_REQUEST_PAGE();
            if (UserMapping)
                Xalloc_MAP_MEMORY(this->HeapEnd, Page, Xalloc_MAP_MEMORY_READ_WRITE | Xalloc_MAP_MEMORY_USER);
            else
                Xalloc_MAP_MEMORY(this->HeapEnd, Page, Xalloc_MAP_MEMORY_READ_WRITE);
            // Xalloc_trace("Expanding Heap Memory (%#llx-%#llx [%#llx])...", this->HeapEnd, (Xuint64_t)this->HeapEnd + Xalloc_PAGE_SIZE, Page);
            this->HeapEnd = (void *)((Xuint64_t)this->HeapEnd + Xalloc_PAGE_SIZE);
        }
        NewSegment->IsFree = true;
        NewSegment->Last = this->LastSegment;
        this->LastSegment->Next = NewSegment;
        this->LastSegment = NewSegment;
        NewSegment->Next = nullptr;
        NewSegment->Length = Length - sizeof(HeapSegment);
        NewSegment->CombineBackward(this->LastSegment);
    }

    void *AllocatorV1::Malloc(Xuint64_t Size)
    {
        SmartSMAP;
        if (this->HeapStart == nullptr)
        {
            Xalloc_err("Memory allocation not initialized yet!");
            return 0;
        }

        if (Size < 0x10)
        {
            // Xalloc_warn("Allocation size is too small, using 0x10 instead!");
            Size = 0x10;
        }

        // #ifdef DEBUG
        //     if (Size < 1024)
        //         debug("Allocating %dB", Size);
        //     else if (TO_KB(Size) < 1024)
        //         debug("Allocating %dKB", TO_KB(Size));
        //     else if (TO_MB(Size) < 1024)
        //         debug("Allocating %dMB", TO_MB(Size));
        //     else if (TO_GB(Size) < 1024)
        //         debug("Allocating %dGB", TO_GB(Size));
        // #endif

        if (Size % 0x10 > 0) // it is not a multiple of 0x10
        {
            Size -= (Size % 0x10);
            Size += 0x10;
        }
        if (Size == 0)
        {
            return nullptr;
        }

        HeapSegment *CurrentSegment = (HeapSegment *)this->HeapStart;
        while (true)
        {
            if (CurrentSegment->IsFree)
            {
                if (CurrentSegment->Length > Size)
                {
                    CurrentSegment->Split(Size, this->LastSegment);
                    CurrentSegment->IsFree = false;
                    return (void *)((Xuint64_t)CurrentSegment + sizeof(HeapSegment));
                }
                if (CurrentSegment->Length == Size)
                {
                    CurrentSegment->IsFree = false;
                    return (void *)((Xuint64_t)CurrentSegment + sizeof(HeapSegment));
                }
            }
            if (CurrentSegment->Next == nullptr)
                break;
            CurrentSegment = CurrentSegment->Next;
        }
        ExpandHeap(Size);
        return this->Malloc(Size);
    }

    void AllocatorV1::Free(void *Address)
    {
        SmartSMAP;
        if (this->HeapStart == nullptr)
        {
            Xalloc_err("Memory allocation not initialized yet!");
            return;
        }
        HeapSegment *Segment = (HeapSegment *)Address - 1;
        Segment->IsFree = true;
        Segment->CombineForward(this->LastSegment);
        Segment->CombineBackward(this->LastSegment);
    }

    void *AllocatorV1::Calloc(Xuint64_t NumberOfBlocks, Xuint64_t Size)
    {
        SmartSMAP;
        if (this->HeapStart == nullptr)
        {
            Xalloc_err("Memory allocation not initialized yet!");
            return 0;
        }

        if (Size < 0x10)
        {
            // Xalloc_warn("Allocation size is too small, using 0x10 instead!");
            Size = 0x10;
        }

        void *Block = this->Malloc(NumberOfBlocks * Size);
        if (Block)
            Xmemset(Block, 0, NumberOfBlocks * Size);
        return Block;
    }

    void *AllocatorV1::Realloc(void *Address, Xuint64_t Size)
    {
        SmartSMAP;
        if (this->HeapStart == nullptr)
        {
            Xalloc_err("Memory allocation not initialized yet!");
            return 0;
        }
        if (!Address && Size == 0)
        {
            this->Free(Address);
            return nullptr;
        }
        else if (!Address)
        {
            return this->Calloc(Size, sizeof(char));
        }

        if (Size < 0x10)
        {
            // Xalloc_warn("Allocation size is too small, using 0x10 instead!");
            Size = 0x10;
        }

        void *newAddress = this->Calloc(Size, sizeof(char));
        Xmemcpy(newAddress, Address, Size);
        return newAddress;
    }
}
