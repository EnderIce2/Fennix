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

#pragma once

#include <types.h>
#include <debug.h>

#if defined(__amd64__)
typedef uint64_t cpuid_t;
#elif defined(__i386__)
typedef uint32_t cpuid_t;
#else
typedef uint64_t cpuid_t;
#endif // __amd64__ || __i386__

#if defined(__amd64__) || defined(__i386__)
#define __kvm_cpuid_init(leaf)                            \
	CPUID##leaf()                                         \
	{                                                     \
		asmv("cpuid" : "=a"(EAX.raw), "=b"(EBX.raw),      \
			 "=c"(ECX.raw), "=d"(EDX.raw) : "a"(leaf));   \
		if (!EAX.raw && !EBX.raw && !ECX.raw && !EDX.raw) \
			warn("cpuid not supported");                  \
	}

#define __kvm_cpuid_init2(leaf, leaf2, suffix)                      \
	CPUID##leaf##suffix()                                           \
	{                                                               \
		asmv("cpuid" : "=a"(EAX.raw), "=b"(EBX.raw),                \
			 "=c"(ECX.raw), "=d"(EDX.raw) : "a"(leaf), "c"(leaf2)); \
		if (!EAX.raw && !EBX.raw && !ECX.raw && !EDX.raw)           \
			warn("cpuid not supported");                            \
	}
#else
#define __kvm_cpuid_init(leaf) \
	CPUID##leaf()              \
	{                          \
	}

#define __kvm_cpuid_init2(leaf, leaf2, suffix) \
	CPUID##leaf##suffix()                      \
	{                                          \
	}
#endif

namespace CPU
{
	namespace x86
	{
		namespace KVM
		{
			/* KVM_CPUID_SIGNATURE */
			struct CPUID0x40000000
			{
				__kvm_cpuid_init(0x40000000);

				union
				{
					struct
					{
						uint32_t MaximumFunction : 32;
					};
					cpuid_t raw;
				} EAX;

				union
				{
					struct
					{
						char Vendor[4];
					};
					cpuid_t raw;
				} EBX;

				union
				{
					struct
					{
						char Vendor[4];
					};
					cpuid_t raw;
				} ECX;

				union
				{
					struct
					{
						char Vendor[4];
					};
					cpuid_t raw;
				} EDX;
			};

			/* KVM_CPUID_FEATURES */
			struct CPUID0x40000001
			{
				__kvm_cpuid_init(0x40000001);

				union
				{
					struct
					{
						/** kvmclock available at msrs 0x11 and 0x12 */
						uint32_t KVM_FEATURE_CLOCKSOURCE : 1;

						/** not necessary to perform delays on PIO operations */
						uint32_t KVM_FEATURE_NOP_IO_DELAY : 1;

						/** deprecated */
						uint32_t KVM_FEATURE_MMU_OP : 1;

						/** kvmclock available at msrs 0x4b564d00 and 0x4b564d01 */
						uint32_t KVM_FEATURE_CLOCKSOURCE2 : 1;

						/** async pf can be enabled by writing to msr 0x4b564d02 */
						uint32_t KVM_FEATURE_ASYNC_PF : 1;

						/** steal time can be enabled by writing to msr 0x4b564d03 */
						uint32_t KVM_FEATURE_STEAL_TIME : 1;

						/** paravirtualized end of interrupt handler can be enabled by writing to msr 0x4b564d04 */
						uint32_t KVM_FEATURE_PV_EOI : 1;

						/** guest checks this feature bit before enabling paravirtualized spinlock support */
						uint32_t KVM_FEATURE_PV_UNHAULT : 1;

						uint32_t _reserved8 : 1;

						/** guest checks this feature bit before enabling paravirtualized tlb flush */
						uint32_t KVM_FEATURE_PV_TLB_FLUSH : 1;

						/** paravirtualized async PF VM EXIT can be enabled by setting bit 2 when writing to msr 0x4b564d02 */
						uint32_t KVM_FEATURE_ASYNC_PF_VMEXIT : 1;

						/** guest checks this feature bit before enabling paravirtualized send IPIs */
						uint32_t KVM_FEATURE_PV_SEND_IPI : 1;

						/** host-side polling on HLT can be disabled by writing to msr 0x4b564d05 */
						uint32_t KVM_FEATURE_PV_POLL_CONTROL : 1;

						/** guest checks this feature bit before using paravirtualized sched yield */
						uint32_t KVM_FEATURE_PV_SCHED_YIELD : 1;

						uint32_t __reserved14_23 : 10;

						/** host will warn if no guest-side per-cpu warps are expected in kvmclock */
						uint32_t KVM_FEATURE_CLOCKSOURCE_STABLE_BIT : 1;

						uint32_t __reserved25_31 : 7;
					};
					cpuid_t raw;
				} EAX;

				union
				{
					struct
					{
						uint32_t __reserved0_31;
					};
					cpuid_t raw;
				} EBX;

				union
				{
					struct
					{
						uint32_t __reserved0_31;
					};
					cpuid_t raw;
				} ECX;

				union
				{
					struct
					{
						/** guest checks this feature bit to determine that vCPUs are never preempted for an unlimited time allowing optimizations */
						uint32_t KVM_HINTS_REALTIME : 1;
					};
					cpuid_t raw;
				} EDX;
			};
		}
	}
}

#undef __kvm_cpuid_init
#undef __kvm_cpuid_init2
