/*
	This file is part of Fennix Kernel.

	Fennix Kernel is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Kernel is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __FENNIX_KERNEL_MEMORY_TABLE_H__
#define __FENNIX_KERNEL_MEMORY_TABLE_H__

#include <types.h>

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
		CoW = 1 << 9,

		/** @brief Available 1 */
		KRsv = 1 << 10,

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

	union __packed PageTableEntry
	{
		struct
		{
#if defined(a64)
			uintptr_t Present : 1;			  // 0
			uintptr_t ReadWrite : 1;		  // 1
			uintptr_t UserSupervisor : 1;	  // 2
			uintptr_t WriteThrough : 1;		  // 3
			uintptr_t CacheDisable : 1;		  // 4
			uintptr_t Accessed : 1;			  // 5
			uintptr_t Dirty : 1;			  // 6
			uintptr_t PageAttributeTable : 1; // 7
			uintptr_t Global : 1;			  // 8
			uintptr_t CopyOnWrite : 1;		  // 9
			uintptr_t KernelReserve : 1;	  // 10
			uintptr_t Available2 : 1;		  // 11
			uintptr_t Address : 40;			  // 12-51
			uintptr_t Available3 : 1;		  // 52
			uintptr_t Available4 : 1;		  // 53
			uintptr_t Available5 : 1;		  // 54
			uintptr_t Available6 : 1;		  // 55
			uintptr_t Available7 : 1;		  // 56
			uintptr_t Available8 : 1;		  // 57
			uintptr_t Available9 : 1;		  // 58
			uintptr_t ProtectionKey : 4;	  // 59-62
			uintptr_t ExecuteDisable : 1;	  // 63
#elif defined(a32)
			uintptr_t Present : 1;			  // 0
			uintptr_t ReadWrite : 1;		  // 1
			uintptr_t UserSupervisor : 1;	  // 2
			uintptr_t WriteThrough : 1;		  // 3
			uintptr_t CacheDisable : 1;		  // 4
			uintptr_t Accessed : 1;			  // 5
			uintptr_t Dirty : 1;			  // 6
			uintptr_t PageAttributeTable : 1; // 7
			uintptr_t Global : 1;			  // 8
			uintptr_t CopyOnWrite : 1;		  // 9
			uintptr_t KernelReserve : 1;	  // 10
			uintptr_t Available2 : 1;		  // 11
			uintptr_t Address : 20;			  // 12-31
#elif defined(aa64)
#endif
		};
		uintptr_t raw = 0;

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
			return ((uintptr_t)(this->raw & 0x003FFFFF000) >> 12);
#elif defined(aa64)
			return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#endif
		}
	};

	struct __packed PageTableEntryPtr
	{
#if defined(a64)
		PageTableEntry Entries[512];
#elif defined(a32)
		PageTableEntry Entries[1024];
#elif defined(aa64)
#endif
	};

	union __packed PageDirectoryEntry
	{
#if defined(a64)
		struct
		{
			uintptr_t Present : 1;		  // 0
			uintptr_t ReadWrite : 1;	  // 1
			uintptr_t UserSupervisor : 1; // 2
			uintptr_t WriteThrough : 1;	  // 3
			uintptr_t CacheDisable : 1;	  // 4
			uintptr_t Accessed : 1;		  // 5
			uintptr_t CopyOnWrite : 1;	  // 6
			uintptr_t PageSize : 1;		  // 7
			uintptr_t KernelReserve : 1;  // 8
			uintptr_t Available2 : 1;	  // 9
			uintptr_t Available3 : 1;	  // 10
			uintptr_t Available4 : 1;	  // 11
			uintptr_t Address : 40;		  // 12-51
			uintptr_t Available5 : 1;	  // 52
			uintptr_t Available6 : 1;	  // 53
			uintptr_t Available7 : 1;	  // 54
			uintptr_t Available8 : 1;	  // 55
			uintptr_t Available9 : 1;	  // 56
			uintptr_t Available10 : 1;	  // 57
			uintptr_t Available11 : 1;	  // 58
			uintptr_t Available12 : 1;	  // 59
			uintptr_t Available13 : 1;	  // 60
			uintptr_t Available14 : 1;	  // 61
			uintptr_t Available15 : 1;	  // 62
			uintptr_t ExecuteDisable : 1; // 63
		};

		struct
		{
			uintptr_t Present : 1;			  // 0
			uintptr_t ReadWrite : 1;		  // 1
			uintptr_t UserSupervisor : 1;	  // 2
			uintptr_t WriteThrough : 1;		  // 3
			uintptr_t CacheDisable : 1;		  // 4
			uintptr_t Accessed : 1;			  // 5
			uintptr_t Dirty : 1;			  // 6
			uintptr_t PageSize : 1;			  // 7
			uintptr_t Global : 1;			  // 8
			uintptr_t CopyOnWrite : 1;		  // 9
			uintptr_t KernelReserve : 1;	  // 10
			uintptr_t Available2 : 1;		  // 11
			uintptr_t PageAttributeTable : 1; // 12
			uintptr_t Reserved0 : 8;		  // 13-20
			uintptr_t Address : 31;			  // 21-51
			uintptr_t Available3 : 1;		  // 52
			uintptr_t Available4 : 1;		  // 53
			uintptr_t Available5 : 1;		  // 54
			uintptr_t Available6 : 1;		  // 55
			uintptr_t Available7 : 1;		  // 56
			uintptr_t Available8 : 1;		  // 57
			uintptr_t Available9 : 1;		  // 58
			uintptr_t ProtectionKey : 4;	  // 59-62
			uintptr_t ExecuteDisable : 1;	  // 63
		} TwoMiB;
#elif defined(a32)
		struct
		{
			uintptr_t Present : 1;		  // 0
			uintptr_t ReadWrite : 1;	  // 1
			uintptr_t UserSupervisor : 1; // 2
			uintptr_t WriteThrough : 1;	  // 3
			uintptr_t CacheDisable : 1;	  // 4
			uintptr_t Accessed : 1;		  // 5
			uintptr_t CopyOnWrite : 1;	  // 6
			uintptr_t PageSize : 1;		  // 7
			uintptr_t KernelReserve : 1;  // 8
			uintptr_t Available2 : 1;	  // 9
			uintptr_t Available3 : 1;	  // 10
			uintptr_t Available4 : 1;	  // 11
			uintptr_t Address : 20;		  // 12-31
		};

		struct
		{
			uintptr_t Present : 1;			  // 0
			uintptr_t ReadWrite : 1;		  // 1
			uintptr_t UserSupervisor : 1;	  // 2
			uintptr_t WriteThrough : 1;		  // 3
			uintptr_t CacheDisable : 1;		  // 4
			uintptr_t Accessed : 1;			  // 5
			uintptr_t Dirty : 1;			  // 6
			uintptr_t PageSize : 1;			  // 7
			uintptr_t Global : 1;			  // 8
			uintptr_t CopyOnWrite : 1;		  // 9
			uintptr_t KernelReserve : 1;	  // 10
			uintptr_t Available2 : 1;		  // 11
			uintptr_t PageAttributeTable : 1; // 12
			uintptr_t Address0 : 8;			  // 13-20
			uintptr_t Reserved0 : 1;		  // 21
			uintptr_t Address1 : 10;		  // 22-31
		} FourMiB;
#elif defined(aa64)
#endif
		uintptr_t raw = 0;

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
			return ((uintptr_t)(this->raw & 0x003FFFFF000) >> 12);
#elif defined(aa64)
			return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#endif
		}
	};

	struct __packed PageDirectoryEntryPtr
	{
		PageDirectoryEntry Entries[512];
	};

	union __packed PageDirectoryPointerTableEntry
	{
#if defined(a64)
		struct
		{
			uintptr_t Present : 1;		  // 0
			uintptr_t ReadWrite : 1;	  // 1
			uintptr_t UserSupervisor : 1; // 2
			uintptr_t WriteThrough : 1;	  // 3
			uintptr_t CacheDisable : 1;	  // 4
			uintptr_t Accessed : 1;		  // 5
			uintptr_t CopyOnWrite : 1;	  // 6
			uintptr_t PageSize : 1;		  // 7
			uintptr_t KernelReserve : 1;  // 8
			uintptr_t Available2 : 1;	  // 9
			uintptr_t Available3 : 1;	  // 10
			uintptr_t Available4 : 1;	  // 11
			uintptr_t Address : 40;		  // 12-51
			uintptr_t Available5 : 1;	  // 52
			uintptr_t Available6 : 1;	  // 53
			uintptr_t Available7 : 1;	  // 54
			uintptr_t Available8 : 1;	  // 55
			uintptr_t Available9 : 1;	  // 56
			uintptr_t Available10 : 1;	  // 57
			uintptr_t Available11 : 1;	  // 58
			uintptr_t Available12 : 1;	  // 59
			uintptr_t Available13 : 1;	  // 60
			uintptr_t Available14 : 1;	  // 61
			uintptr_t Available15 : 1;	  // 62
			uintptr_t ExecuteDisable : 1; // 63
		};

		struct
		{
			uintptr_t Present : 1;			  // 0
			uintptr_t ReadWrite : 1;		  // 1
			uintptr_t UserSupervisor : 1;	  // 2
			uintptr_t WriteThrough : 1;		  // 3
			uintptr_t CacheDisable : 1;		  // 4
			uintptr_t Accessed : 1;			  // 5
			uintptr_t Dirty : 1;			  // 6
			uintptr_t PageSize : 1;			  // 7
			uintptr_t Global : 1;			  // 8
			uintptr_t CopyOnWrite : 1;		  // 9
			uintptr_t KernelReserve : 1;	  // 10
			uintptr_t Available2 : 1;		  // 11
			uintptr_t PageAttributeTable : 1; // 12
			uintptr_t Reserved0 : 17;		  // 13-29
			uintptr_t Address : 22;			  // 30-51
			uintptr_t Available3 : 1;		  // 52
			uintptr_t Available4 : 1;		  // 53
			uintptr_t Available5 : 1;		  // 54
			uintptr_t Available6 : 1;		  // 55
			uintptr_t Available7 : 1;		  // 56
			uintptr_t Available8 : 1;		  // 57
			uintptr_t Available9 : 1;		  // 58
			uintptr_t ProtectionKey : 4;	  // 59-62
			uintptr_t ExecuteDisable : 1;	  // 63
		} OneGiB;
#elif defined(aa64)
#endif
		uintptr_t raw = 0;

		/** @brief Set PageDirectoryEntryPtr address */
		void SetAddress(uintptr_t _Address)
		{
#if defined(a64)
			_Address &= 0x000000FFFFFFFFFF;
			this->raw &= 0xFFF0000000000FFF;
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
			return 0;
#elif defined(aa64)
			return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#endif
		}
	};

	struct __packed PageDirectoryPointerTableEntryPtr
	{
		PageDirectoryPointerTableEntry Entries[512];
	};

	union __packed PageMapLevel4
	{
#if defined(a64)
		struct
		{
			uintptr_t Present : 1;		  // 0
			uintptr_t ReadWrite : 1;	  // 1
			uintptr_t UserSupervisor : 1; // 2
			uintptr_t WriteThrough : 1;	  // 3
			uintptr_t CacheDisable : 1;	  // 4
			uintptr_t Accessed : 1;		  // 5
			uintptr_t CopyOnWrite : 1;	  // 6
			uintptr_t Reserved0 : 1;	  // 7
			uintptr_t KernelReserve : 1;  // 8
			uintptr_t Available2 : 1;	  // 9
			uintptr_t Available3 : 1;	  // 10
			uintptr_t Available4 : 1;	  // 11
			uintptr_t Address : 40;		  // 12-51
			uintptr_t Available5 : 1;	  // 52
			uintptr_t Available6 : 1;	  // 53
			uintptr_t Available7 : 1;	  // 54
			uintptr_t Available8 : 1;	  // 55
			uintptr_t Available9 : 1;	  // 56
			uintptr_t Available10 : 1;	  // 57
			uintptr_t Available11 : 1;	  // 58
			uintptr_t Available12 : 1;	  // 59
			uintptr_t Available13 : 1;	  // 60
			uintptr_t Available14 : 1;	  // 61
			uintptr_t Available15 : 1;	  // 62
			uintptr_t ExecuteDisable : 1; // 63
		};
#elif defined(aa64)
#endif
		uintptr_t raw = 0;

		/** @brief Set PageDirectoryPointerTableEntryPtr address */
		void SetAddress(uintptr_t _Address)
		{
#if defined(a64)
			_Address &= 0x000000FFFFFFFFFF;
			this->raw &= 0xFFF0000000000FFF;
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
			return 0;
#elif defined(aa64)
			return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#endif
		}
	};

	struct __packed PageMapLevel4Ptr
	{
		PageMapLevel4 Entries[512];
	};

	union __packed PageMapLevel5
	{
#if defined(a64)
		struct
		{
			uintptr_t Present : 1;		  // 0
			uintptr_t ReadWrite : 1;	  // 1
			uintptr_t UserSupervisor : 1; // 2
			uintptr_t WriteThrough : 1;	  // 3
			uintptr_t CacheDisable : 1;	  // 4
			uintptr_t Accessed : 1;		  // 5
			uintptr_t Available0 : 1;	  // 6
			uintptr_t Reserved0 : 1;	  // 7
			uintptr_t KernelReserve : 1;  // 8
			uintptr_t Available1 : 1;	  // 9
			uintptr_t Available2 : 1;	  // 10
			uintptr_t Available3 : 1;	  // 11
			uintptr_t Address : 40;		  // 12-51
			uintptr_t Available4 : 1;	  // 52
			uintptr_t Available5 : 1;	  // 53
			uintptr_t Available6 : 1;	  // 54
			uintptr_t Available7 : 1;	  // 55
			uintptr_t Available8 : 1;	  // 56
			uintptr_t Available9 : 1;	  // 57
			uintptr_t Available10 : 1;	  // 58
			uintptr_t Available11 : 1;	  // 59
			uintptr_t Available12 : 1;	  // 60
			uintptr_t Available13 : 1;	  // 61
			uintptr_t Available14 : 1;	  // 62
			uintptr_t ExecuteDisable : 1; // 63
		};
#elif defined(aa64)
#endif
		uintptr_t raw = 0;

		/** @brief Set PageMapLevel4Ptr address */
		void SetAddress(uintptr_t _Address)
		{
#if defined(a64)
			_Address &= 0x000000FFFFFFFFFF;
			this->raw &= 0xFFF0000000000FFF;
			this->raw |= (_Address << 12);
#elif defined(aa64)
			_Address &= 0x000000FFFFFFFFFF;
			this->raw &= 0xFFF0000000000FFF;
			this->raw |= (_Address << 12);
#endif
		}

		/** @brief Get PageMapLevel4Ptr address */
		uintptr_t GetAddress()
		{
#if defined(a64)
			return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#elif defined(a32)
			return 0;
#elif defined(aa64)
			return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#endif
		}
	};

	class PageTable
	{
	public:
#if defined(a64)
		PageMapLevel4 Entries[512];
#elif defined(a32)
		PageDirectoryEntry Entries[1024];
#elif defined(aa64)
#endif

		/**
		 * @brief Update CR3 with this PageTable
		 */
		void Update();

		/**
		 * @brief Fork this PageTable
		 *
		 * @return A new PageTable with the same content
		 */
		PageTable *Fork();

		void *__getPhysical(void *Address);

		/**
		 * @brief Get the Physical Address of a virtual address
		 *
		 * This function will return the physical address
		 * of a virtual address and not the virtual address
		 * of the current page table. This is intentional because
		 * the kernel page table has 1:1 mapping for the free
		 * memory.
		 *
		 * @tparam T
		 * @param Address The virtual address
		 * @return The physical address
		 */
		template <typename T>
		T Get(T Address)
		{
			void *PhysAddr = __getPhysical((void *)Address);
			if (PhysAddr == nullptr)
				return {};
			uintptr_t Diff = uintptr_t(Address);
			Diff &= 0xFFF;
			Diff = uintptr_t(PhysAddr) + Diff;
			return (T)Diff;
		}
	} __aligned(0x1000);
}

#endif // !__FENNIX_KERNEL_MEMORY_TABLE_H__
