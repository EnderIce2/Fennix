#ifndef __FENNIX_KERNEL_INTERNAL_MEMORY_H__
#define __FENNIX_KERNEL_INTERNAL_MEMORY_H__

#ifndef LYNX_MEMORY_TYPES_H
#define LYNX_MEMORY_TYPES_H

typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __UINT64_TYPE__ uint64_t;
typedef __SIZE_TYPE__ size_t;
typedef __UINTPTR_TYPE__ uintptr_t;
#ifndef NULL
#define NULL ((void *)0)
#endif

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#define ALIGN_UP(x, align) ((__typeof__(x))(((uint64_t)(x) + ((align)-1)) & (~((align)-1))))
#define ALIGN_DOWN(x, align) ((__typeof__(x))((x) & (~((align)-1))))

#endif // !LYNX_MEMORY_TYPES_H

#include "../bitmap.hpp"

#include <efi.h>
#include <efilib.h>

#define PAGE_SIZE 0x1000

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

#ifdef __cplusplus

namespace Memory
{
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
        };
        uint64_t raw;
    } PDEData;

    struct __attribute__((packed)) PageDirectoryEntry
    {
        PDEData Value;
        void AddFlag(uint64_t Flag) { this->Value.raw |= Flag; }
        void RemoveFlags(uint64_t Flag) { this->Value.raw &= ~Flag; }
        void ClearFlags() { this->Value.raw = 0; }
        void SetFlag(uint64_t Flag, bool Enabled)
        {
            this->Value.raw &= ~Flag;
            if (Enabled)
                this->Value.raw |= Flag;
        }
        bool GetFlag(uint64_t Flag) { return (this->Value.raw & Flag) > 0 ? true : false; }
        uint64_t GetFlag() { return this->Value.raw; }
        void SetAddress(uint64_t Address)
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
        uint64_t GetAddress()
        {
#if defined(__amd64__)
            return (this->Value.raw & 0x000FFFFFFFFFF000) >> 12;
#elif defined(__i386__)
            return (this->Value.raw & 0x003FFFFF000) >> 12;
#elif defined(__aarch64__)
            return (this->Value.raw & 0x000FFFFFFFFFF000) >> 12;
#endif
        }
    };

    struct PageTable
    {
        PageDirectoryEntry Entries[512];
    } __attribute__((aligned(0x1000)));

    class Physical
    {
    private:
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
        void Init(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable);
        /** @brief Do not use. */
        Physical();
        /** @brief Do not use. */
        ~Physical();
    };

    class Virtual
    {
    private:
        PageTable *Table = nullptr;

        class PageMapIndexer
        {
        public:
            uint64_t PDP_i;
            uint64_t PD_i;
            uint64_t PT_i;
            uint64_t P_i;

            PageMapIndexer(uint64_t VirtualAddress)
            {
#if defined(__amd64__)
                PDP_i = (VirtualAddress & ((uint64_t)0x1FF << 39)) >> 39;
                PD_i = (VirtualAddress & ((uint64_t)0x1FF << 30)) >> 30;
                PT_i = (VirtualAddress & ((uint64_t)0x1FF << 21)) >> 21;
                P_i = (VirtualAddress & ((uint64_t)0x1FF << 12)) >> 12;
#elif defined(__i386__)
                PD_i = (VirtualAddress & ((uint64_t)0x3FF << 22)) >> 22;
                PT_i = (VirtualAddress & ((uint64_t)0x3FF << 12)) >> 12;
                P_i = (VirtualAddress & ((uint64_t)0xFFF << 0)) >> 0;
#elif defined(__aarch64__)
                PD_i = (VirtualAddress & ((uint64_t)0x1FF << 30)) >> 30;
                PT_i = (VirtualAddress & ((uint64_t)0x1FF << 21)) >> 21;
                P_i = (VirtualAddress & ((uint64_t)0x1FF << 12)) >> 12;
#endif
            }
        };

    public:
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

void *operator new(uint64_t Size);
void *operator new[](uint64_t Size);
void operator delete(void *Pointer);
void operator delete[](void *Pointer);
void operator delete(void *Pointer, long unsigned int Size);
void operator delete[](void *Pointer, long unsigned int Size);

extern Memory::Physical KernelAllocator;
extern Memory::PageTable *KernelPageTable;

#endif // __cplusplus

EXTERNC void InitializeMemoryManagement(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable);

EXTERNC void *HeapMalloc(uint64_t Size);
EXTERNC void *HeapCalloc(uint64_t n, uint64_t Size);
EXTERNC void *HeapRealloc(void *Address, uint64_t Size);
EXTERNC void HeapFree(void *Address);

#define kmalloc(Size) HeapMalloc(Size)
#define kcalloc(n, Size) HeapCalloc(n, Size)
#define krealloc(Address, Size) HeapRealloc(Address, Size)
#define kfree(Address) HeapFree(Address)

#endif // !__FENNIX_KERNEL_INTERNAL_MEMORY_H__
