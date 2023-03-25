#ifndef __FENNIX_KERNEL_INTERNAL_MEMORY_H__
#define __FENNIX_KERNEL_INTERNAL_MEMORY_H__

#ifdef __cplusplus
#include <filesystem.hpp>
#include <boot/binfo.h>
#include <bitmap.hpp>
#include <lock.hpp>
#include <std.hpp>
#endif // __cplusplus
#include <types.h>

#ifdef __cplusplus

extern uintptr_t _kernel_start, _kernel_end;
extern uintptr_t _kernel_text_end, _kernel_data_end, _kernel_rodata_end;

// kilobyte
#define TO_KB(d) (d / 1024)
// megabyte
#define TO_MB(d) (d / 1024 / 1024)
// gigabyte
#define TO_GB(d) (d / 1024 / 1024 / 1024)
// terabyte
#define TO_TB(d) (d / 1024 / 1024 / 1024 / 1024)
// petabyte
#define TO_PB(d) (d / 1024 / 1024 / 1024 / 1024 / 1024)
// exobyte
#define TO_EB(d) (d / 1024 / 1024 / 1024 / 1024 / 1024 / 1024)
// zettabyte
#define TO_ZB(d) (d / 1024 / 1024 / 1024 / 1024 / 1024 / 1024 / 1024)
// yottabyte
#define TO_YB(d) (d / 1024 / 1024 / 1024 / 1024 / 1024 / 1024 / 1024 / 1024)
// brontobyte
#define TO_BB(d) (d / 1024 / 1024 / 1024 / 1024 / 1024 / 1024 / 1024 / 1024 / 1024)
// geopbyte
#define TO_GPB(d) (d / 1024 / 1024 / 1024 / 1024 / 1024 / 1024 / 1024 / 1024 / 1024 / 1024)

#define PAGE_SIZE 0x1000       // 4KB
#define STACK_SIZE 0x4000      // 16kb
#define USER_STACK_SIZE 0x2000 // 8kb

// To pages
#define TO_PAGES(d) ((d) / PAGE_SIZE + 1)
// From pages
#define FROM_PAGES(d) ((d)*PAGE_SIZE)

#if defined(a64) || defined(aa64)
#define NORMAL_VMA_OFFSET 0xFFFF800000000000
#define KERNEL_VMA_OFFSET 0xFFFFFFFF80000000
#define KERNEL_HEAP_BASE 0xFFFFA00000000000
#define USER_HEAP_BASE 0xFFFFB00000000000
#define USER_STACK_BASE 0xFFFFEFFFFFFF0000
#elif defined(a32)
#define NORMAL_VMA_OFFSET 0x80000000
#define KERNEL_VMA_OFFSET 0xC0000000
#define KERNEL_HEAP_BASE 0xA0000000
#define USER_HEAP_BASE 0xB0000000
#define USER_STACK_BASE 0xEFFFFFFF
#endif

namespace Memory
{
    enum MemoryAllocatorType
    {
        None,
        Pages,
        XallocV1,
        liballoc11
    };

    /**
     * @brief https://wiki.osdev.org/images/4/41/64-bit_page_tables1.png
     * @brief https://wiki.osdev.org/images/6/6b/64-bit_page_tables2.png
     */
    enum PTFlag
    {
        /** @brief Present */
        P = 1 << 0,

        /** @brief Read/Write */
        RW = 1 << 1,

        /** @brief User/Supervisor */
        US = 1 << 2,

        /** @brief Write-Through */
        PWT = 1 << 3,

        /** @brief Cache Disable */
        PCD = 1 << 4,

        /** @brief Accessed */
        A = 1 << 5,

        /** @brief Dirty */
        D = 1 << 6,

        /** @brief Page Size */
        PS = 1 << 7,

        /** @brief Global */
        G = 1 << 8,

        /** @brief Available 0 */
        AVL0 = 1 << 9,

        /** @brief Available 1 */
        AVL1 = 1 << 10,

        /** @brief Available 2 */
        AVL2 = 1 << 11,

        /** @brief Page Attribute Table */
        PAT = 1 << 12,

        /** @brief Available 3 */
        AVL3 = (uint64_t)1 << 52,

        /** @brief Available 4 */
        AVL4 = (uint64_t)1 << 53,

        /** @brief Available 5 */
        AVL5 = (uint64_t)1 << 54,

        /** @brief Available 6 */
        AVL6 = (uint64_t)1 << 55,

        /** @brief Available 7 */
        AVL7 = (uint64_t)1 << 56,

        /** @brief Available 8 */
        AVL8 = (uint64_t)1 << 57,

        /** @brief Available 9 */
        AVL9 = (uint64_t)1 << 58,

        /** @brief Protection Key 0 */
        PK0 = (uint64_t)1 << 59,

        /** @brief Protection Key 1 */
        PK1 = (uint64_t)1 << 60,

        /** @brief Protection Key 2 */
        PK2 = (uint64_t)1 << 61,

        /** @brief Protection Key 3 */
        PK3 = (uint64_t)1 << 62,

        /** @brief Execute Disable */
        XD = (uint64_t)1 << 63
    };

    /* 2.2 Paging in IA-32e Mode - https://composter.com.ua/documents/TLBs_Paging-Structure_Caches_and_Their_Invalidation.pdf */

    union __attribute__((packed)) PageTableEntry
    {
        struct
        {
            bool Present : 1;            // 0
            bool ReadWrite : 1;          // 1
            bool UserSupervisor : 1;     // 2
            bool WriteThrough : 1;       // 3
            bool CacheDisable : 1;       // 4
            bool Accessed : 1;           // 5
            bool Dirty : 1;              // 6
            bool PageAttributeTable : 1; // 7
            bool Global : 1;             // 8
            uint8_t Available0 : 3;      // 9-11
            uint64_t Address : 40;       // 12-51
            uint32_t Available1 : 7;     // 52-58
            uint8_t ProtectionKey : 4;   // 59-62
            bool ExecuteDisable : 1;     // 63
        };
        uint64_t raw;

        /** @brief Set Address */
        void SetAddress(uintptr_t _Address)
        {
#if defined(a64)
            _Address &= 0x000000FFFFFFFFFF;
            this->raw &= 0xFFF0000000000FFF;
            this->raw |= (_Address << 12);
#elif defined(a32)
            _Address &= 0x000FFFFF;
            this->raw &= 0xFFC00003;
            this->raw |= (_Address << 12);
#elif defined(aa64)
            _Address &= 0x000000FFFFFFFFFF;
            this->raw &= 0xFFF0000000000FFF;
            this->raw |= (_Address << 12);
#endif
        }

        /** @brief Get Address */
        uintptr_t GetAddress()
        {
#if defined(a64)
            return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#elif defined(a32)
            return (this->raw & 0x003FFFFF000) >> 12;
#elif defined(aa64)
            return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#endif
        }
    };

    struct __attribute__((packed)) PageTableEntryPtr
    {
        PageTableEntry Entries[511];
    };

    union __attribute__((packed)) PageDirectoryEntry
    {
        struct
        {
            bool Present : 1;         // 0
            bool ReadWrite : 1;       // 1
            bool UserSupervisor : 1;  // 2
            bool WriteThrough : 1;    // 3
            bool CacheDisable : 1;    // 4
            bool Accessed : 1;        // 5
            bool Available0 : 1;      // 6
            bool PageSize : 1;        // 7
            uint8_t Available1 : 4;   // 8-11
            uint64_t Address : 40;    // 12-51
            uint32_t Available2 : 11; // 52-62
            bool ExecuteDisable : 1;  // 63
        };
        uint64_t raw;

        /** @brief Set PageTableEntryPtr address */
        void SetAddress(uintptr_t _Address)
        {
#if defined(a64)
            _Address &= 0x000000FFFFFFFFFF;
            this->raw &= 0xFFF0000000000FFF;
            this->raw |= (_Address << 12);
#elif defined(a32)
            _Address &= 0x000FFFFF;
            this->raw &= 0xFFC00003;
            this->raw |= (_Address << 12);
#elif defined(aa64)
            _Address &= 0x000000FFFFFFFFFF;
            this->raw &= 0xFFF0000000000FFF;
            this->raw |= (_Address << 12);
#endif
        }

        /** @brief Get PageTableEntryPtr address */
        uintptr_t GetAddress()
        {
#if defined(a64)
            return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#elif defined(a32)
            return (this->raw & 0x003FFFFF000) >> 12;
#elif defined(aa64)
            return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#endif
        }
    };

    struct __attribute__((packed)) PageDirectoryEntryPtr
    {
        PageDirectoryEntry Entries[511];
    };

    union __attribute__((packed)) PageDirectoryPointerTableEntry
    {
        struct
        {
            bool Present : 1;         // 0
            bool ReadWrite : 1;       // 1
            bool UserSupervisor : 1;  // 2
            bool WriteThrough : 1;    // 3
            bool CacheDisable : 1;    // 4
            bool Accessed : 1;        // 5
            bool Available0 : 1;      // 6
            bool PageSize : 1;        // 7
            uint8_t Available1 : 4;   // 8-11
            uint64_t Address : 40;    // 12-51
            uint32_t Available2 : 11; // 52-62
            bool ExecuteDisable : 1;  // 63
        };
        uint64_t raw;

        /** @brief Set PageDirectoryEntryPtr address */
        void SetAddress(uintptr_t _Address)
        {
#if defined(a64)
            _Address &= 0x000000FFFFFFFFFF;
            this->raw &= 0xFFF0000000000FFF;
            this->raw |= (_Address << 12);
#elif defined(a32)
            _Address &= 0x000FFFFF;
            this->raw &= 0xFFC00003;
            this->raw |= (_Address << 12);
#elif defined(aa64)
            _Address &= 0x000000FFFFFFFFFF;
            this->raw &= 0xFFF0000000000FFF;
            this->raw |= (_Address << 12);
#endif
        }

        /** @brief Get PageDirectoryEntryPtr address */
        uintptr_t GetAddress()
        {
#if defined(a64)
            return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#elif defined(a32)
            return (this->raw & 0x003FFFFF000) >> 12;
#elif defined(aa64)
            return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#endif
        }
    };

    struct __attribute__((packed)) PageDirectoryPointerTableEntryPtr
    {
        PageDirectoryPointerTableEntry Entries[511];
    };

    union __attribute__((packed)) PageMapLevel4
    {
        struct
        {
            bool Present : 1;         // 0
            bool ReadWrite : 1;       // 1
            bool UserSupervisor : 1;  // 2
            bool WriteThrough : 1;    // 3
            bool CacheDisable : 1;    // 4
            bool Accessed : 1;        // 5
            bool Available0 : 1;      // 6
            bool Reserved0 : 1;       // 7
            uint8_t Available1 : 4;   // 8-11
            uint64_t Address : 40;    // 12-51
            uint32_t Available2 : 11; // 52-62
            bool ExecuteDisable : 1;  // 63
        };
        uint64_t raw;

        /** @brief Set PageDirectoryPointerTableEntryPtr address */
        void SetAddress(uintptr_t _Address)
        {
#if defined(a64)
            _Address &= 0x000000FFFFFFFFFF;
            this->raw &= 0xFFF0000000000FFF;
            this->raw |= (_Address << 12);
#elif defined(a32)
            _Address &= 0x000FFFFF;
            this->raw &= 0xFFC00003;
            this->raw |= (_Address << 12);
#elif defined(aa64)
            _Address &= 0x000000FFFFFFFFFF;
            this->raw &= 0xFFF0000000000FFF;
            this->raw |= (_Address << 12);
#endif
        }

        /** @brief Get PageDirectoryPointerTableEntryPtr address */
        uintptr_t GetAddress()
        {
#if defined(a64)
            return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#elif defined(a32)
            return (this->raw & 0x003FFFFF000) >> 12;
#elif defined(aa64)
            return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#endif
        }
    };

    struct PageTable4
    {
        PageMapLevel4 Entries[511];
    } __attribute__((aligned(0x1000)));

    struct __attribute__((packed)) PageMapLevel5
    {
        /* FIXME: NOT IMPLEMENTED! */
    };

    struct PageTable5
    {
        PageMapLevel5 Entries[511];
    } __attribute__((aligned(0x1000)));

    class Physical
    {
    private:
        NewLock(MemoryLock);

        uint64_t TotalMemory = 0;
        uint64_t FreeMemory = 0;
        uint64_t ReservedMemory = 0;
        uint64_t UsedMemory = 0;
        uint64_t PageBitmapIndex = 0;
        Bitmap PageBitmap;

        void ReservePage(void *Address);
        void ReservePages(void *Address, size_t PageCount);
        void UnreservePage(void *Address);
        void UnreservePages(void *Address, size_t PageCount);

    public:
        Bitmap GetPageBitmap() { return PageBitmap; }
        /**
         * @brief Get Total Memory
         *
         * @return uint64_t
         */
        uint64_t GetTotalMemory();
        /**
         * @brief Get Free Memory
         *
         * @return uint64_t
         */
        uint64_t GetFreeMemory();
        /**
         * @brief Get Reserved Memory
         *
         * @return uint64_t
         */
        uint64_t GetReservedMemory();
        /**
         * @brief Get Used Memory
         *
         * @return uint64_t
         */
        uint64_t GetUsedMemory();

        /**
         * @brief Swap page
         *
         * @param Address Address of the page
         * @return true if swap was successful
         * @return false if swap was unsuccessful
         */
        bool SwapPage(void *Address);
        /**
         * @brief Swap pages
         *
         * @param Address Address of the pages
         * @param PageCount Number of pages
         * @return true if swap was successful
         * @return false if swap was unsuccessful
         */
        bool SwapPages(void *Address, size_t PageCount);
        /**
         * @brief Unswap page
         *
         * @param Address Address of the page
         * @return true if unswap was successful
         * @return false if unswap was unsuccessful
         */
        bool UnswapPage(void *Address);
        /**
         * @brief Unswap pages
         *
         * @param Address Address of the pages
         * @param PageCount Number of pages
         * @return true if unswap was successful
         * @return false if unswap was unsuccessful
         */
        bool UnswapPages(void *Address, size_t PageCount);

        /**
         * @brief Lock page
         *
         * @param Address Address of the page
         */
        void LockPage(void *Address);
        /**
         * @brief Lock pages
         *
         * @param Address Address of the pages
         * @param PageCount Number of pages
         */
        void LockPages(void *Address, size_t PageCount);

        /**
         * @brief Request page
         *
         * @return void* Allocated page address
         */
        void *RequestPage();
        /**
         * @brief Request pages
         *
         * @param PageCount Number of pages
         * @return void* Allocated pages address
         */
        void *RequestPages(size_t Count);
        /**
         * @brief Free page
         *
         * @param Address Address of the page
         */
        void FreePage(void *Address);
        /**
         * @brief Free pages
         *
         * @param Address Address of the pages
         * @param PageCount Number of pages
         */
        void FreePages(void *Address, size_t Count);
        /** @brief Do not use. */
        void Init(BootInfo *Info);
        /** @brief Do not use. */
        Physical();
        /** @brief Do not use. */
        ~Physical();
    };

    class Virtual
    {
    private:
        NewLock(MemoryLock);
        PageTable4 *Table = nullptr;

    public:
        class PageMapIndexer
        {
        public:
            uintptr_t PMLIndex = 0;
            uintptr_t PDPTEIndex = 0;
            uintptr_t PDEIndex = 0;
            uintptr_t PTEIndex = 0;
            PageMapIndexer(uintptr_t VirtualAddress);
        };

        /**
         * @brief Check if page has the specified flag.
         *
         * @param VirtualAddress Virtual address of the page
         * @param Flag Flag to check
         * @return true if page has the specified flag.
         * @return false if page is has the specified flag.
         */
        bool Check(void *VirtualAddress, PTFlag Flag = PTFlag::P);

        /**
         * @brief Map page.
         *
         * @param VirtualAddress Virtual address of the page.
         * @param PhysicalAddress Physical address of the page.
         * @param Flags Flags of the page. Check PTFlag enum.
         */
        void Map(void *VirtualAddress, void *PhysicalAddress, uint64_t Flags);

        /**
         * @brief Map multiple pages.
         *
         * @param VirtualAddress First virtual address of the page.
         * @param PhysicalAddress First physical address of the page.
         * @param PageCount Number of pages.
         * @param Flags Flags of the page. Check PTFlag enum.
         */
        void Map(void *VirtualAddress, void *PhysicalAddress, size_t PageCount, uint64_t Flags);

        /**
         * @brief Unmap page.
         *
         * @param VirtualAddress Virtual address of the page.
         */
        void Unmap(void *VirtualAddress);

        /**
         * @brief Unmap multiple pages.
         *
         * @param VirtualAddress First virtual address of the page.
         * @param PageCount Number of pages.
         */
        void Unmap(void *VirtualAddress, size_t PageCount);

        /**
         * @brief Remap page.
         *
         * @param VirtualAddress Virtual address of the page.
         * @param PhysicalAddress Physical address of the page.
         * @param Flags Flags of the page. Check PTFlag enum.
         */
        void Remap(void *VirtualAddress, void *PhysicalAddress, uint64_t Flags);

        /**
         * @brief Construct a new Virtual object
         *
         * @param Table Page table. If null, it will use the current page table.
         */
        Virtual(PageTable4 *Table = nullptr);

        /**
         * @brief Destroy the Virtual object
         *
         */
        ~Virtual();
    };

    class StackGuard
    {
    private:
        void *StackBottom = nullptr;
        void *StackTop = nullptr;
        void *StackPhyiscalBottom = nullptr;
        void *StackPhyiscalTop = nullptr;
        uint64_t Size = 0;
        bool UserMode = false;
        PageTable4 *Table = nullptr;

    public:
        /** @brief For general info */
        void *GetStackBottom() { return StackBottom; }
        /** @brief For RSP */
        void *GetStackTop() { return StackTop; }
        /** @brief For general info */
        void *GetStackPhysicalBottom() { return StackPhyiscalBottom; }
        /** @brief For general info */
        void *GetStackPhysicalTop() { return StackPhyiscalTop; }
        /** @brief Called by exception handler */
        bool Expand(uintptr_t FaultAddress);
        /**
         * @brief Construct a new Stack Guard object
         * @param User Stack for user mode?
         */
        StackGuard(bool User, PageTable4 *Table);
        /**
         * @brief Destroy the Stack Guard object
         */
        ~StackGuard();
    };

    class MemMgr
    {
    public:
        struct AllocatedPages
        {
            void *Address;
            size_t PageCount;
        };

        std::vector<AllocatedPages> GetAllocatedPagesList() { return AllocatedPagesList; }
        uint64_t GetAllocatedMemorySize();

        bool Add(void *Address, size_t Count);

        void *RequestPages(size_t Count, bool User = false);
        void FreePages(void *Address, size_t Count);

        void DetachAddress(void *Address);

        MemMgr(PageTable4 *PageTable = nullptr, VirtualFileSystem::Node *Directory = nullptr);
        ~MemMgr();

    private:
        Bitmap PageBitmap;
        PageTable4 *PageTable;
        VirtualFileSystem::Node *Directory;

        std::vector<AllocatedPages> AllocatedPagesList;
    };
}

void InitializeMemoryManagement(BootInfo *Info);

void *operator new(size_t Size);
void *operator new[](size_t Size);
void *operator new(size_t Size, std::align_val_t Alignment);
void operator delete(void *Pointer);
void operator delete[](void *Pointer);
void operator delete(void *Pointer, long unsigned int Size);
void operator delete[](void *Pointer, long unsigned int Size);

extern Memory::Physical KernelAllocator;
extern Memory::PageTable4 *KernelPageTable;
extern Memory::PageTable4 *UserspaceKernelOnlyPageTable;

#endif // __cplusplus

EXTERNC void *malloc(size_t Size);
EXTERNC void *calloc(size_t n, size_t Size);
EXTERNC void *realloc(void *Address, size_t Size);
EXTERNC void free(void *Address);

#define kmalloc(Size) malloc(Size)
#define kcalloc(n, Size) calloc(n, Size)
#define krealloc(Address, Size) realloc(Address, Size)
#define kfree(Address) free(Address)

#endif // !__FENNIX_KERNEL_INTERNAL_MEMORY_H__
