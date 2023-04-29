#pragma once
#include <bitmap.hpp>
#include <binfo.h>
#include <debug.h>
#include <types.h>

extern uintptr_t _kernel_start, _kernel_end;

// kilobyte
#define TO_KB(d) ((d) / 1024)
// megabyte
#define TO_MB(d) ((d) / 1024 / 1024)
// gigabyte
#define TO_GB(d) ((d) / 1024 / 1024 / 1024)

#define PAGE_SIZE 0x1000        // 4KB
#define PAGE_SIZE_4K PAGE_SIZE  // 4KB
#define PAGE_SIZE_2M 0x200000   // 2MB
#define PAGE_SIZE_4M 0x400000   // 4MB
#define PAGE_SIZE_1G 0x40000000 // 1GB

// To pages
#define TO_PAGES(d) (((d) + PAGE_SIZE - 1) / PAGE_SIZE)
// From pages
#define FROM_PAGES(d) ((d)*PAGE_SIZE)

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

namespace Memory32
{
    union __packed PageTableEntry
    {
        struct
        {
            uint32_t Present : 1;            // 0
            uint32_t ReadWrite : 1;          // 1
            uint32_t UserSupervisor : 1;     // 2
            uint32_t WriteThrough : 1;       // 3
            uint32_t CacheDisable : 1;       // 4
            uint32_t Accessed : 1;           // 5
            uint32_t Dirty : 1;              // 6
            uint32_t PageAttributeTable : 1; // 7
            uint32_t Global : 1;             // 8
            uint32_t Available0 : 3;         // 9-11
            uint32_t Address : 20;           // 12-31
        };
        uint32_t raw;

        void SetAddress(uintptr_t _Address)
        {
            _Address &= 0x000FFFFF;
            this->raw &= 0xFFC00003;
            this->raw |= (_Address << 12);
        }

        uintptr_t GetAddress()
        {
            return (this->raw & 0x003FFFFF000) >> 12;
        }
    };

    struct __packed PageTableEntryPtr
    {
        PageTableEntry Entries[1024];
    };

    union __packed PageDirectoryEntry
    {
        struct
        {
            uint32_t Present : 1;        // 0
            uint32_t ReadWrite : 1;      // 1
            uint32_t UserSupervisor : 1; // 2
            uint32_t WriteThrough : 1;   // 3
            uint32_t CacheDisable : 1;   // 4
            uint32_t Accessed : 1;       // 5
            uint32_t Available0 : 1;     // 6
            uint32_t PageSize : 1;       // 7
            uint32_t Available1 : 4;     // 8-11
            uint32_t Address : 20;       // 12-31
        };
        uint32_t raw;

        void SetAddress(uintptr_t _Address)
        {
            _Address &= 0x000FFFFF;
            this->raw &= 0xFFC00003;
            this->raw |= (_Address << 12);
        }

        uintptr_t GetAddress()
        {
            return (this->raw & 0x003FFFFF000) >> 12;
        }
    };

    struct PageTable
    {
        PageDirectoryEntry Entries[1024];
    } __aligned(0x1000);

    class Physical
    {
    private:
        uint64_t PageBitmapIndex = 0;
        Bitmap PageBitmap;

    public:
        uint64_t TotalMemory = 0;
        uint64_t FreeMemory = 0;
        uint64_t ReservedMemory = 0;
        uint64_t UsedMemory = 0;
        void LockPage(void *Address);
        void LockPages(void *Address, size_t PageCount);
        void ReservePage(void *Address);
        void ReservePages(void *Address, size_t PageCount);
        void UnreservePage(void *Address);
        void UnreservePages(void *Address, size_t PageCount);
        void *RequestPage();
        void *RequestPages(size_t Count);
        void FreePage(void *Address);
        void FreePages(void *Address, size_t Count);
        void Init(BootInfo *Info);
        Physical();
        ~Physical();
    };

    class Virtual
    {
    private:
        PageTable *Table = nullptr;

    public:
        class PageMapIndexer
        {
        public:
            uintptr_t PDEIndex = 0;
            uintptr_t PTEIndex = 0;
            PageMapIndexer(uintptr_t VirtualAddress)
            {
                uintptr_t Address = VirtualAddress;
                Address >>= 12;
                this->PTEIndex = Address & 0x3FF;
                Address >>= 10;
                this->PDEIndex = Address & 0x3FF;
            }
        };

        void Map(void *VirtualAddress, void *PhysicalAddress, uint64_t Flag = PTFlag::P);

        __always_inline inline void Map(void *VirtualAddress, void *PhysicalAddress, size_t Length, uint64_t Flags)
        {
            for (uintptr_t i = 0; i < Length; i += PAGE_SIZE_4K)
                this->Map((void *)((uintptr_t)VirtualAddress + i), (void *)((uintptr_t)PhysicalAddress + i), Flags);
        }

        Virtual(PageTable *Table = nullptr);
        ~Virtual();
    };
}

namespace Memory64
{
    union __packed PageTableEntry
    {
        struct
        {
            uint64_t Present : 1;            // 0
            uint64_t ReadWrite : 1;          // 1
            uint64_t UserSupervisor : 1;     // 2
            uint64_t WriteThrough : 1;       // 3
            uint64_t CacheDisable : 1;       // 4
            uint64_t Accessed : 1;           // 5
            uint64_t Dirty : 1;              // 6
            uint64_t PageAttributeTable : 1; // 7
            uint64_t Global : 1;             // 8
            uint64_t Available0 : 3;         // 9-11
            uint64_t Address : 40;           // 12-51
            uint64_t Available1 : 7;         // 52-58
            uint64_t ProtectionKey : 4;      // 59-62
            uint64_t ExecuteDisable : 1;     // 63
        };
        uint64_t raw;

        /** @brief Set Address */
        void SetAddress(uintptr_t _Address)
        {
            _Address &= 0x000000FFFFFFFFFF;
            this->raw &= 0xFFF0000000000FFF;
            this->raw |= (_Address << 12);
        }

        /** @brief Get Address */
        uintptr_t GetAddress()
        {
            return (this->raw & 0x000FFFFFFFFFF000) >> 12;
        }
    };

    struct __packed PageTableEntryPtr
    {
        PageTableEntry Entries[511];
    };

    union __packed PageDirectoryEntry
    {
        struct
        {
            uint64_t Present : 1;        // 0
            uint64_t ReadWrite : 1;      // 1
            uint64_t UserSupervisor : 1; // 2
            uint64_t WriteThrough : 1;   // 3
            uint64_t CacheDisable : 1;   // 4
            uint64_t Accessed : 1;       // 5
            uint64_t Available0 : 1;     // 6
            uint64_t PageSize : 1;       // 7
            uint64_t Available1 : 4;     // 8-11
            uint64_t Address : 40;       // 12-51
            uint64_t Available2 : 11;    // 52-62
            uint64_t ExecuteDisable : 1; // 63
        };

        struct
        {
            uint64_t Present : 1;            // 0
            uint64_t ReadWrite : 1;          // 1
            uint64_t UserSupervisor : 1;     // 2
            uint64_t WriteThrough : 1;       // 3
            uint64_t CacheDisable : 1;       // 4
            uint64_t Accessed : 1;           // 5
            uint64_t Dirty : 1;              // 6
            uint64_t PageSize : 1;           // 7
            uint64_t Global : 1;             // 8
            uint64_t Available0 : 3;         // 9-11
            uint64_t PageAttributeTable : 1; // 12
            uint64_t Reserved0 : 8;          // 13-20
            uint64_t Address : 31;           // 21-51
            uint64_t Available1 : 7;         // 52-58
            uint64_t ProtectionKey : 4;      // 59-62
            uint64_t ExecuteDisable : 1;     // 63
        } TwoMB;

        uint64_t raw;

        /** @brief Set PageTableEntryPtr address */
        void SetAddress(uintptr_t _Address)
        {
            _Address &= 0x000000FFFFFFFFFF;
            this->raw &= 0xFFF0000000000FFF;
            this->raw |= (_Address << 12);
        }

        /** @brief Get PageTableEntryPtr address */
        uintptr_t GetAddress()
        {
            return (this->raw & 0x000FFFFFFFFFF000) >> 12;
        }
    };

    struct __packed PageDirectoryEntryPtr
    {
        PageDirectoryEntry Entries[511];
    };

    union __packed PageDirectoryPointerTableEntry
    {
        struct
        {
            uint64_t Present : 1;        // 0
            uint64_t ReadWrite : 1;      // 1
            uint64_t UserSupervisor : 1; // 2
            uint64_t WriteThrough : 1;   // 3
            uint64_t CacheDisable : 1;   // 4
            uint64_t Accessed : 1;       // 5
            uint64_t Available0 : 1;     // 6
            uint64_t PageSize : 1;       // 7
            uint64_t Available1 : 4;     // 8-11
            uint64_t Address : 40;       // 12-51
            uint64_t Available2 : 11;    // 52-62
            uint64_t ExecuteDisable : 1; // 63
        };

        struct
        {
            uint64_t Present : 1;            // 0
            uint64_t ReadWrite : 1;          // 1
            uint64_t UserSupervisor : 1;     // 2
            uint64_t WriteThrough : 1;       // 3
            uint64_t CacheDisable : 1;       // 4
            uint64_t Accessed : 1;           // 5
            uint64_t Dirty : 1;              // 6
            uint64_t PageSize : 1;           // 7
            uint64_t Global : 1;             // 8
            uint64_t Available0 : 3;         // 9-11
            uint64_t PageAttributeTable : 1; // 12
            uint64_t Reserved0 : 17;         // 13-29
            uint64_t Address : 22;           // 30-51
            uint64_t Available1 : 7;         // 52-58
            uint64_t ProtectionKey : 4;      // 59-62
            uint64_t ExecuteDisable : 1;     // 63
        } OneGB;

        uint64_t raw;

        /** @brief Set PageDirectoryEntryPtr address */
        void SetAddress(uintptr_t _Address)
        {
            _Address &= 0x000000FFFFFFFFFF;
            this->raw &= 0xFFF0000000000FFF;
            this->raw |= (_Address << 12);
        }

        /** @brief Get PageDirectoryEntryPtr address */
        uintptr_t GetAddress()
        {
            return (this->raw & 0x000FFFFFFFFFF000) >> 12;
        }
    };

    struct __packed PageDirectoryPointerTableEntryPtr
    {
        PageDirectoryPointerTableEntry Entries[511];
    };

    union __packed PageMapLevel4
    {
        struct
        {
            uint64_t Present : 1;        // 0
            uint64_t ReadWrite : 1;      // 1
            uint64_t UserSupervisor : 1; // 2
            uint64_t WriteThrough : 1;   // 3
            uint64_t CacheDisable : 1;   // 4
            uint64_t Accessed : 1;       // 5
            uint64_t Available0 : 1;     // 6
            uint64_t Reserved0 : 1;      // 7
            uint64_t Available1 : 4;     // 8-11
            uint64_t Address : 40;       // 12-51
            uint64_t Available2 : 11;    // 52-62
            uint64_t ExecuteDisable : 1; // 63
        };
        uint64_t raw;

        /** @brief Set PageDirectoryPointerTableEntryPtr address */
        void SetAddress(uintptr_t _Address)
        {
            _Address &= 0x000000FFFFFFFFFF;
            this->raw &= 0xFFF0000000000FFF;
            this->raw |= (_Address << 12);
        }

        /** @brief Get PageDirectoryPointerTableEntryPtr address */
        uintptr_t GetAddress()
        {
            return (this->raw & 0x000FFFFFFFFFF000) >> 12;
        }
    };

    struct PageTable
    {
        PageMapLevel4 Entries[511];

        /**
         * @brief Update CR3 with this PageTable
         */
        void Update()
        {
            asmv("mov %0, %%cr3" ::"r"(this));
        }
    } __aligned(0x1000);

    class Physical
    {
    private:
        uint64_t PageBitmapIndex = 0;
        Bitmap PageBitmap;

    public:
        uint64_t TotalMemory = 0;
        uint64_t FreeMemory = 0;
        uint64_t ReservedMemory = 0;
        uint64_t UsedMemory = 0;
        void LockPage(void *Address);
        void LockPages(void *Address, size_t PageCount);
        void ReservePage(void *Address);
        void ReservePages(void *Address, size_t PageCount);
        void UnreservePage(void *Address);
        void UnreservePages(void *Address, size_t PageCount);
        void *RequestPage();
        void *RequestPages(size_t Count);
        void FreePage(void *Address);
        void FreePages(void *Address, size_t Count);
        void Init(BootInfo *Info);
        Physical();
        ~Physical();
    };

    class Virtual
    {
    private:
        PageTable *Table = nullptr;

    public:
        enum MapType
        {
            NoMapType,
            FourKB,
            TwoMB,
            OneGB
        };

        class PageMapIndexer
        {
        public:
            uintptr_t PMLIndex = 0;
            uintptr_t PDPTEIndex = 0;
            uintptr_t PDEIndex = 0;
            uintptr_t PTEIndex = 0;
            PageMapIndexer(uintptr_t VirtualAddress)
            {
                uintptr_t Address = VirtualAddress;
                Address >>= 12;
                this->PTEIndex = Address & 0x1FF;
                Address >>= 9;
                this->PDEIndex = Address & 0x1FF;
                Address >>= 9;
                this->PDPTEIndex = Address & 0x1FF;
                Address >>= 9;
                this->PMLIndex = Address & 0x1FF;
            }
        };

        bool Check(void *VirtualAddress, PTFlag Flag = PTFlag::P, MapType Type = MapType::FourKB);
        void *GetPhysical(void *VirtualAddress);
        void Map(void *VirtualAddress, void *PhysicalAddress, uint64_t Flag = PTFlag::P, MapType Type = MapType::FourKB);

        __always_inline inline void Map(void *VirtualAddress, void *PhysicalAddress, size_t Length, uint64_t Flags, MapType Type = MapType::FourKB)
        {
            int PageSize = PAGE_SIZE_4K;

            if (Type == MapType::TwoMB)
                PageSize = PAGE_SIZE_2M;
            else if (Type == MapType::OneGB)
                PageSize = PAGE_SIZE_1G;

            for (uintptr_t i = 0; i < Length; i += PageSize)
                this->Map((void *)((uintptr_t)VirtualAddress + i), (void *)((uintptr_t)PhysicalAddress + i), Flags, Type);
        }

        __always_inline inline MapType OptimizedMap(void *VirtualAddress, void *PhysicalAddress, size_t Length, uint64_t Flags, bool Fit = false, bool FailOnModulo = false)
        {
            if (unlikely(Fit))
            {
                while (Length >= PAGE_SIZE_1G)
                {
                    this->Map(VirtualAddress, PhysicalAddress, Length, Flags, Virtual::MapType::OneGB);
                    VirtualAddress = (void *)((uintptr_t)VirtualAddress + PAGE_SIZE_1G);
                    PhysicalAddress = (void *)((uintptr_t)PhysicalAddress + PAGE_SIZE_1G);
                    Length -= PAGE_SIZE_1G;
                }

                while (Length >= PAGE_SIZE_2M)
                {
                    this->Map(VirtualAddress, PhysicalAddress, Length, Flags, Virtual::MapType::TwoMB);
                    VirtualAddress = (void *)((uintptr_t)VirtualAddress + PAGE_SIZE_2M);
                    PhysicalAddress = (void *)((uintptr_t)PhysicalAddress + PAGE_SIZE_2M);
                    Length -= PAGE_SIZE_2M;
                }

                while (Length >= PAGE_SIZE_4K)
                {
                    this->Map(VirtualAddress, PhysicalAddress, Length, Flags, Virtual::MapType::FourKB);
                    VirtualAddress = (void *)((uintptr_t)VirtualAddress + PAGE_SIZE_4K);
                    PhysicalAddress = (void *)((uintptr_t)PhysicalAddress + PAGE_SIZE_4K);
                    Length -= PAGE_SIZE_4K;
                }

                return Virtual::MapType::FourKB;
            }

            Virtual::MapType Type = Virtual::MapType::FourKB;

            if (Length >= PAGE_SIZE_1G)
            {
                Type = Virtual::MapType::OneGB;
                if (Length % PAGE_SIZE_1G != 0)
                {
                    warn("Length is not a multiple of 1GB.");
                    if (FailOnModulo)
                        return Virtual::MapType::NoMapType;
                }
            }
            else if (Length >= PAGE_SIZE_2M)
            {
                Type = Virtual::MapType::TwoMB;
                if (Length % PAGE_SIZE_2M != 0)
                {
                    warn("Length is not a multiple of 2MB.");
                    if (FailOnModulo)
                        return Virtual::MapType::NoMapType;
                }
            }

            this->Map(VirtualAddress, PhysicalAddress, Length, Flags, Type);
            return Type;
        }

        void Unmap(void *VirtualAddress, MapType Type = MapType::FourKB);

        __always_inline inline void Unmap(void *VirtualAddress, size_t Length, MapType Type = MapType::FourKB)
        {
            int PageSize = PAGE_SIZE_4K;

            if (Type == MapType::TwoMB)
                PageSize = PAGE_SIZE_2M;
            else if (Type == MapType::OneGB)
                PageSize = PAGE_SIZE_1G;

            for (uintptr_t i = 0; i < Length; i += PageSize)
                this->Unmap((void *)((uintptr_t)VirtualAddress + i), Type);
        }

        __always_inline inline void Remap(void *VirtualAddress, void *PhysicalAddress, uint64_t Flags, MapType Type = MapType::FourKB)
        {
            this->Unmap(VirtualAddress, Type);
            this->Map(VirtualAddress, PhysicalAddress, Flags, Type);
        }

        Virtual(PageTable *Table = nullptr);
        ~Virtual();
    };
}

extern Memory32::Physical KernelAllocator32;
extern Memory32::PageTable *KernelPageTable32;

extern Memory64::Physical KernelAllocator64;
extern Memory64::PageTable *KernelPageTable64;

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void InitializeMemoryManagement(BootInfo *Info, bool is32);
