#include "Xalloc.hpp"

namespace Xalloc
{
    class XLockClass
    {
        struct SpinLockData
        {
            uint64_t LockData = 0x0;
            const char *CurrentHolder = "(null)";
            const char *AttemptingToGet = "(null)";
            uint64_t Count = 0;
        };

        void DeadLock(SpinLockData Lock)
        {
            Xalloc_warn("Potential deadlock in lock '%s' held by '%s'! %ld locks in queue.", Lock.AttemptingToGet, Lock.CurrentHolder, Lock.Count);
        }

    private:
        SpinLockData LockData;
        bool IsLocked = false;

    public:
        int Lock(const char *FunctionName)
        {
            LockData.AttemptingToGet = FunctionName;
        Retry:
            unsigned int i = 0;
            while (__atomic_exchange_n(&IsLocked, true, __ATOMIC_ACQUIRE) && ++i < 0x10000000)
                ;
            if (i >= 0x10000000)
            {
                DeadLock(LockData);
                goto Retry;
            }
            LockData.Count++;
            LockData.CurrentHolder = FunctionName;
            __sync_synchronize();
            return 0;
        }

        int Unlock()
        {
            __sync_synchronize();
            __atomic_store_n(&IsLocked, false, __ATOMIC_RELEASE);
            LockData.Count--;
            IsLocked = false;
            return 0;
        }
    };

    class XSmartLock
    {
    private:
        XLockClass *LockPointer = nullptr;

    public:
        XSmartLock(XLockClass &Lock, const char *FunctionName)
        {
            this->LockPointer = &Lock;
            this->LockPointer->Lock(FunctionName);
        }
        ~XSmartLock() { this->LockPointer->Unlock(); }
    };

    XLockClass XLock;

#define XSL XSmartLock CONCAT(lock##_, __COUNTER__)(XLock, __FUNCTION__)

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
        XSL;
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
        XSL;
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
        XSL;
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
        XLock.Unlock();
        return this->Malloc(Size);
    }

    void AllocatorV1::Free(void *Address)
    {
        SmartSMAP;
        XSL;
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
        XSL;
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

        XLock.Unlock();
        void *Block = this->Malloc(NumberOfBlocks * Size);
        XLock.Lock(__FUNCTION__);

        if (Block)
            Xmemset(Block, 0, NumberOfBlocks * Size);
        return Block;
    }

    void *AllocatorV1::Realloc(void *Address, Xuint64_t Size)
    {
        SmartSMAP;
        XSL;
        if (this->HeapStart == nullptr)
        {
            Xalloc_err("Memory allocation not initialized yet!");
            return 0;
        }
        if (!Address && Size == 0)
        {
            XLock.Unlock();
            this->Free(Address);
            return nullptr;
        }
        else if (!Address)
        {
            XLock.Unlock();
            return this->Calloc(Size, sizeof(char));
        }

        if (Size < 0x10)
        {
            // Xalloc_warn("Allocation size is too small, using 0x10 instead!");
            Size = 0x10;
        }

        XLock.Unlock();
        void *newAddress = this->Calloc(Size, sizeof(char));
        XLock.Lock(__FUNCTION__);
        Xmemcpy(newAddress, Address, Size);
        return newAddress;
    }
}
