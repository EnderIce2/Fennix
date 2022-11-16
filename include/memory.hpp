#ifndef __FENNIX_KERNEL_INTERNAL_MEMORY_H__
#define __FENNIX_KERNEL_INTERNAL_MEMORY_H__

#ifdef __cplusplus
#include <boot/binfo.h>
#include <bitmap.hpp>
#include <lock.hpp>
#endif // __cplusplus
#include <types.h>

#ifdef __cplusplus

extern uint64_t _kernel_start, _kernel_end;
extern uint64_t _kernel_text_end, _kernel_data_end, _kernel_rodata_end;

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

#define PAGE_SIZE 0x1000
#define STACK_SIZE 0x1000000
#define USER_STACK_SIZE 0x1000000

// to pages
#define TO_PAGES(d) (d / PAGE_SIZE + 1)
// from pages
#define FROM_PAGES(d) (d * PAGE_SIZE - 1)

#define NORMAL_VMA_OFFSET 0xFFFF800000000000
#define KERNEL_VMA_OFFSET 0xFFFFFFFF80000000

/**
 * @brief KERNEL_HEAP_BASE is the base address of the kernel heap
 */
#define KERNEL_HEAP_BASE 0xFFFFC00000000000
/**
 * @brief USER_HEAP_BASE is the base address of the user heap allocated by the kernel
 */
#define USER_HEAP_BASE 0xFFFFD00000000000

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

    typedef union __attribute__((packed))
    {
        struct
        {
#if defined(__amd64__)
            bool Present : 1;
            bool ReadWrite : 1;
            bool UserSupervisor : 1;
            bool WriteThrough : 1;
            bool CacheDisable : 1;
            bool Accessed : 1;
            bool Dirty : 1;
            bool PageSize : 1;
            bool Global : 1;
            uint8_t Available1 : 3;
            bool PageAttributeTable : 1;
            uint64_t Reserved : 39;
            uint32_t Available2 : 7;
            uint16_t ProtectionKey : 4;
            bool ExecuteDisable : 1;
#elif defined(__i386__)
            bool Present : 1;
            bool ReadWrite : 1;
            bool UserSupervisor : 1;
            bool Accessed : 1;
            bool Dirty : 1;
            uint8_t Available : 7;
            uint32_t Frame : 20;
// TODO: i386 PDEData is not tested
#elif defined(__aarch64__)
// TODO: aarch64 PDEData not implemented
#endif
        };
        uint64_t raw;
    } PDEData;

    struct __attribute__((packed)) PageDirectoryEntry
    {
        PDEData Value;
        void AddFlag(uint64_t Flag);
        void RemoveFlags(uint64_t Flag);
        void ClearFlags();
        void SetFlag(uint64_t Flag, bool Enabled);
        bool GetFlag(uint64_t Flag);
        uint64_t GetFlag();
        void SetAddress(uint64_t Address);
        uint64_t GetAddress();
    };

    struct PageTable
    {
        PageDirectoryEntry Entries[512];
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
        void ReservePages(void *Address, uint64_t PageCount);
        void UnreservePage(void *Address);
        void UnreservePages(void *Address, uint64_t PageCount);

    public:
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
        bool SwapPages(void *Address, uint64_t PageCount);
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
        bool UnswapPages(void *Address, uint64_t PageCount);

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
        void LockPages(void *Address, uint64_t PageCount);

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
        void *RequestPages(uint64_t Count);
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
        void FreePages(void *Address, uint64_t Count);
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
        PageTable *Table = nullptr;

        class PageMapIndexer
        {
        public:
            uint64_t PDPIndex = 0;
            uint64_t PDIndex = 0;
            uint64_t PTIndex = 0;
            uint64_t PIndex = 0;
            PageMapIndexer(uint64_t VirtualAddress);
        };

    public:
        /**
         * @brief Check if page is present
         *
         * @param VirtualAddress Virtual address of the page
         * @param Flag Flag to check
         * @return true if page is present
         * @return false if page is not present
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
        void Map(void *VirtualAddress, void *PhysicalAddress, uint64_t PageCount, uint64_t Flags);

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
        void Unmap(void *VirtualAddress, uint64_t PageCount);

        /**
         * @brief Construct a new Virtual object
         *
         * @param Table Page table. If null, it will use the current page table.
         */
        Virtual(PageTable *Table = nullptr);

        /**
         * @brief Destroy the Virtual object
         *
         */
        ~Virtual();
    };
}

/**
 * @brief // stub namespace for std::align_val_t and new operator
 * @note // Found on https://gcc.gnu.org/legacy-ml/gcc-patches/2016-09/msg00628.html for "_ZnwmSt11align_val_t" compiler error
 */
namespace std
{
    typedef __SIZE_TYPE__ size_t;
    enum class align_val_t : std::size_t
    {
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
extern Memory::PageTable *KernelPageTable;
extern Memory::PageTable *UserspaceKernelOnlyPageTable;

#endif // __cplusplus

EXTERNC void *HeapMalloc(uint64_t Size);
EXTERNC void *HeapCalloc(uint64_t n, uint64_t Size);
EXTERNC void *HeapRealloc(void *Address, uint64_t Size);
EXTERNC void HeapFree(void *Address);

#define kmalloc(Size) HeapMalloc(Size)
#define kcalloc(n, Size) HeapCalloc(n, Size)
#define krealloc(Address, Size) HeapRealloc(Address, Size)
#define kfree(Address) HeapFree(Address)

#endif // !__FENNIX_KERNEL_INTERNAL_MEMORY_H__
