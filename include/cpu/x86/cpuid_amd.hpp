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

#ifndef __FENNIX_KERNEL_CPU_x86_CPUID_AMD_H__
#define __FENNIX_KERNEL_CPU_x86_CPUID_AMD_H__

#include <types.h>

#if defined(a64)
typedef uint64_t cpuid_t;
#elif defined(a32)
typedef uint32_t cpuid_t;
#else
typedef uint64_t cpuid_t;
#endif // a64 || a32

namespace CPU
{
	namespace x86
	{
		/** @brief EXPERIMENTAL IMPLEMENTATION */
		namespace AMD
		{
			/** @brief Basic CPU information */
			struct CPUID0x00000000
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x0));
#endif // a64 || a32
				}

				/** @brief Largest Standard Function Number */
				union
				{
					struct
					{
						uint32_t LFuncStd : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief Processor Vendor */
				union
				{
					struct
					{
						char Vendor[4];
					};
					cpuid_t raw;
				} EBX;

				/** @brief Processor Vendor */
				union
				{
					struct
					{
						char Vendor[4];
					};
					cpuid_t raw;
				} ECX;

				/** @brief Processor Vendor */
				union
				{
					struct
					{
						char Vendor[4];
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief Additional CPU information */
			struct CPUID0x00000001
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x1));
#endif // a64 || a32
				}

				/** @brief Family, Model, Stepping Identifiers */
				union
				{
					/* "This function is an identical copy of CPUID Fn8000_0001_EAX." */
					struct
					{
						uint32_t Stepping : 4;
						uint32_t BaseModel : 4;
						uint32_t BaseFamily : 4;
						uint32_t Reserved0 : 4;
						uint32_t ExtModel : 4;
						uint32_t ExtFamily : 8;
						uint32_t Reserved1 : 4;
					};
					cpuid_t raw;
				} EAX;

				/** @brief LocalApicId, LogicalProcessorCount, CLFlush */
				union
				{
					struct
					{
						uint32_t BrandId : 8;
						uint32_t CLFlush : 8;
						uint32_t LogicalProcessorCount : 8;
						uint32_t LocalApicId : 8;
					};
					cpuid_t raw;
				} EBX;

				/** @brief Feature Identifiers */
				union
				{
					struct
					{
						uint32_t SSE3 : 1;
						uint32_t PCLMULQDQ : 1;
						uint32_t Reserved0 : 1;
						uint32_t MONITOR : 1;
						uint32_t Reserved1 : 5;
						uint32_t SSSE3 : 1;
						uint32_t Reserved2 : 2;
						uint32_t FMA : 1;
						uint32_t CMPXCHG16B : 1;
						uint32_t Reserved3 : 5;
						uint32_t SSE41 : 1;
						uint32_t SSE42 : 1;
						uint32_t x2APIC : 1;
						uint32_t MOVBE : 1;
						uint32_t POPCNT : 1;
						uint32_t Reserved4 : 1;
						uint32_t AES : 1;
						uint32_t XSAVE : 1;
						uint32_t OSXSAVE : 1;
						uint32_t AVX : 1;
						uint32_t F16C : 1;
						uint32_t RDRAND : 1;

						/**
						 * @brief Reserved for use by hypervisor to indicate guest status.
						 */
						uint32_t Hypervisor : 1;
					};
					cpuid_t raw;
				} ECX;

				/** @brief Feature Identifiers */
				union
				{
					struct
					{
						uint32_t FPU : 1;
						uint32_t VME : 1;
						uint32_t DE : 1;
						uint32_t PSE : 1;
						uint32_t TSC : 1;
						uint32_t MSR : 1;
						uint32_t PAE : 1;
						uint32_t MCE : 1;
						uint32_t CMPXCHG8B : 1;
						uint32_t APIC : 1;
						uint32_t Reserved0 : 1;
						uint32_t SysEnterSysExit : 1;
						uint32_t MTRR : 1;
						uint32_t PGE : 1;
						uint32_t MCA : 1;
						uint32_t CMOV : 1;
						uint32_t PAT : 1;
						uint32_t PSE36 : 1;
						uint32_t Reserved1 : 1;
						uint32_t CLFSH : 1;
						uint32_t Reserved2 : 3;
						uint32_t MMX : 1;
						uint32_t FXSR : 1;
						uint32_t SSE : 1;
						uint32_t SSE2 : 1;
						uint32_t Reserved3 : 1;
						uint32_t HTT : 1;
						uint32_t Reserved4 : 3;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief Monitor and MWait Features */
			struct CPUID0x00000005
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x5));
#endif // a64 || a32
				}

				/** @brief Monitor/MWait */
				union
				{
					struct
					{
						uint32_t MonLineSizeMin : 16;
						uint32_t Reserved0 : 16;
					};
					cpuid_t raw;
				} EAX;

				/** @brief Monitor/MWait */
				union
				{
					struct
					{
						uint32_t MonLineSizeMax : 16;
						uint32_t Reserved0 : 16;
					};
					cpuid_t raw;
				} EBX;

				/** @brief Monitor/MWait */
				union
				{
					struct
					{
						uint32_t EMX : 1;
						uint32_t IBE : 1;
						uint32_t Reserved0 : 30;
					};
					cpuid_t raw;
				} ECX;

				/** @brief Reserved */
				union
				{
					struct
					{
						uint32_t Reserved : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief Power Management Related Features */
			struct CPUID0x00000006
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x6));
#endif // a64 || a32
				}

				/** @brief Local APIC Timer Invariance */
				union
				{
					struct
					{
						uint32_t Reserved0 : 2;
						uint32_t ARAT : 1;
						uint32_t Reserved1 : 29;
					};
					cpuid_t raw;
				} EAX;

				/** @brief Reserved */
				union
				{
					struct
					{
						uint32_t Reserved : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief Effective Processor Frequency Interface */
				union
				{
					struct
					{
						uint32_t EffFreq : 1;
						uint32_t Reserved0 : 31;
					};
					cpuid_t raw;
				} ECX;

				/** @brief Reserved */
				union
				{
					struct
					{
						uint32_t Reserved : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief Structured Extended Feature Identifiers */
			struct CPUID0x00000007
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x7));
#endif // a64 || a32
				}

				/** @brief Structured Extended Feature Identifiers */
				union
				{
					struct
					{
						uint32_t MaxSubFn : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief Structured Extended Feature Identifiers */
				union
				{
					struct
					{
						uint32_t FSGSBASE : 1;
						uint32_t Reserved0 : 2;
						uint32_t BMI1 : 1;
						uint32_t Reserved1 : 1;
						uint32_t AVX2 : 1;
						uint32_t Reserved2 : 1;
						uint32_t SMEP : 1;
						uint32_t BMI2 : 1;
						uint32_t Reserved3 : 1;
						uint32_t INVPCID : 1;
						uint32_t Reserved4 : 1;
						uint32_t PQM : 1;
						uint32_t Reserved5 : 2;
						uint32_t PQE : 1;
						uint32_t Reserved6 : 2;
						uint32_t RDSEED : 1;
						uint32_t ADX : 1;
						uint32_t SMAP : 1;
						uint32_t Reserved7 : 1;
						uint32_t RDPID : 1;
						uint32_t CLFLUSHOPT : 1;
						uint32_t CLWB : 1;
						uint32_t Reserved8 : 4;
						uint32_t SHA : 1;
						uint32_t Reserved9 : 2;
					};
					cpuid_t raw;
				} EBX;

				/** @brief Structured Extended Feature Identifiers */
				union
				{
					struct
					{
						uint32_t Reserved0 : 2;
						uint32_t UMIP : 1;
						uint32_t PKU : 1;
						uint32_t OSPKE : 1;
						uint32_t Reserved1 : 2;
						uint32_t CET_SS : 1;
						uint32_t Reserved2 : 1;
						uint32_t VAES : 1;
						uint32_t VPCLMULQDQ : 1;
						uint32_t Reserved3 : 5;
						uint32_t LA57 : 1;
						uint32_t Reserved4 : 15;
					};
					cpuid_t raw;
				} ECX;

				/** @brief Structured Extended Feature Identifiers */
				union
				{
					struct
					{
						uint32_t Reserved : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief Thread Level - Extended Topology Enumeration */
			struct CPUID0x0000000B_ECX_0
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0xB), "c"(0x0));
#endif // a64 || a32
				}

				/** @brief Extended Topology Enumeration */
				union
				{
					struct
					{
						uint32_t ThreadMaskWidth : 5;
						uint32_t Reserved0 : 27;
					};
					cpuid_t raw;
				} EAX;

				/** @brief Extended Topology Enumeration */
				union
				{
					struct
					{
						uint32_t NumberOfThreadsInACore : 5; /* No field name */
						uint32_t Reserved0 : 27;
					};
					cpuid_t raw;
				} EBX;

				/** @brief Extended Topology Enumeration */
				union
				{
					struct
					{
						uint32_t ECXInputValue : 8;
						uint32_t LevelNumber : 8;
						uint32_t Reserved0 : 16;
					};
					cpuid_t raw;
				} ECX;

				/** @brief Extended Topology Enumeration */
				union
				{
					struct
					{
						uint32_t x2APID_ID : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief Core Level - Extended Topology Enumeration */
			struct CPUID0x0000000B_ECX_1
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0xB), "c"(0x1));
#endif // a64 || a32
				}

				/** @brief Extended Topology Enumeration */
				union
				{
					struct
					{
						uint32_t CoreMaskWidth : 5;
						uint32_t Reserved0 : 27;
					};
					cpuid_t raw;
				} EAX;

				/** @brief Extended Topology Enumeration */
				union
				{
					struct
					{
						uint32_t NumberOfLogicalCoresInSocket : 5; /* No field name */
						uint32_t Reserved0 : 27;
					};
					cpuid_t raw;
				} EBX;

				/** @brief Extended Topology Enumeration */
				union
				{
					struct
					{
						uint32_t ECXInputValue : 8;
						uint32_t LevelNumber : 8;
						uint32_t Reserved0 : 16;
					};
					cpuid_t raw;
				} ECX;

				/** @brief Extended Topology Enumeration */
				union
				{
					struct
					{
						uint32_t x2APID_ID : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief Processor Extended State Enumeration */
			struct CPUID0x0000000D_ECX_0
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0xD), "c"(0x0));
#endif // a64 || a32
				}

				/** @brief Processor Extended State Enumeration */
				union
				{
					struct
					{
						uint32_t XFeatureSupportedMask : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief Processor Extended State Enumeration */
				union
				{
					struct
					{
						uint32_t XFeatureEnabledSizeMax : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief Processor Extended State Enumeration */
				union
				{
					struct
					{
						uint32_t XFeatureSupportedSizeMax : 32;
					};
					cpuid_t raw;
				} ECX;

				/** @brief Processor Extended State Enumeration */
				union
				{
					struct
					{
						uint32_t XFeatureSupportedMask : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief Processor Extended State Enumeration */
			struct CPUID0x0000000D_ECX_1
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0xD), "c"(0x1));
#endif // a64 || a32
				}

				/** @brief Processor Extended State Enumeration */
				union
				{
					struct
					{
						uint32_t XSAVEOPT : 1;
						uint32_t XSAVEC : 1;
						uint32_t XGETBV : 1;
						uint32_t XSAVES : 1;
						uint32_t Reserved0 : 28;
					};
					cpuid_t raw;
				} EAX;

				/** @brief Processor Extended State Enumeration */
				union
				{
					struct
					{
						/**
						 * @brief The value returned on EBX represents the
						 * fixed size of the save area (240h) plus the
						 * state size of each enabled extended feature:
						 * EBX = 0240h
						 * + ((XCR0[AVX] == 1) ? 0000_0100h : 0)
						 * + ((XCR0[MPK] == 1) ? 0000_0008h : 0)
						 * + ((XSS[CET_U] == 1) ? 0000_0010h : 0)
						 * + ((XSS[CET_S] == 1) ? 0000_0018h : 0)
						 */
						uint32_t ebx : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief Processor Extended State Enumeration */
				union
				{
					struct
					{
						uint32_t Reserved0 : 11;
						uint32_t CET_U : 1;
						uint32_t CET_S : 1;
						uint32_t Reserved1 : 19;
					};
					cpuid_t raw;
				} ECX;

				/** @brief Processor Extended State Enumeration */
				union
				{
					struct
					{
						uint32_t Reserved : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief Processor Extended State Enumeration */
			struct CPUID0x0000000D_ECX_2
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0xD), "c"(0x2));
#endif // a64 || a32
				}

				/** @brief Processor Extended State Enumeration */
				union
				{
					struct
					{
						uint32_t YmmSaveStateSize : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief Processor Extended State Enumeration */
				union
				{
					struct
					{
						uint32_t YmmSaveStateOffset : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief Processor Extended State Enumeration */
				union
				{
					struct
					{
						uint32_t Reserved : 32;
					};
					cpuid_t raw;
				} ECX;

				/** @brief Processor Extended State Enumeration */
				union
				{
					struct
					{
						uint32_t Reserved : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief Processor Extended State Emulation */
			struct CPUID0x0000000D_ECX_11
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0xD), "c"(0x11));
#endif // a64 || a32
				}

				/** @brief Processor Extended State Emulation */
				union
				{
					struct
					{
						uint32_t CetSupervisorSize : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief Processor Extended State Emulation */
				union
				{
					struct
					{
						uint32_t CetSupervisorOffset : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief Processor Extended State Emulation */
				union
				{
					struct
					{
						uint32_t Reserved : 31;
						uint32_t US : 1;
					};
					cpuid_t raw;
				} ECX;

				/** @brief Processor Extended State Emulation */
				union
				{
					struct
					{
						uint32_t Unused : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief Processor Extended State Emulation */
			struct CPUID0x0000000D_ECX_12
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0xD), "c"(0x12));
#endif // a64 || a32
				}

				/** @brief Processor Extended State Emulation */
				union
				{
					struct
					{
						uint32_t CetSupervisorSize : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief Processor Extended State Emulation */
				union
				{
					struct
					{
						uint32_t CetSupervisorOffset : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief Processor Extended State Emulation */
				union
				{
					struct
					{
						uint32_t Reserved : 31;
						uint32_t US : 1;
					};
					cpuid_t raw;
				} ECX;

				/** @brief Processor Extended State Emulation */
				union
				{
					struct
					{
						uint32_t Unused : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief Processor Extended State Enumeration */
			struct CPUID0x0000000D_ECX_3E
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0xD), "c"(0x3E));
#endif // a64 || a32
				}

				/** @brief Processor Extended State Enumeration */
				union
				{
					struct
					{
						uint32_t LwpSaveStateSize : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief Processor Extended State Enumeration */
				union
				{
					struct
					{
						uint32_t LwpSaveStateOffset : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief Processor Extended State Enumeration */
				union
				{
					struct
					{
						uint32_t Reserved : 32;
					};
					cpuid_t raw;
				} ECX;

				/** @brief Processor Extended State Enumeration */
				union
				{
					struct
					{
						uint32_t Reserved : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief Maximum Extended Function Number and Vendor String */
			struct CPUID0x80000000
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x80000000));
#endif // a64 || a32
				}

				/** @brief Largest Extended Function Number */
				union
				{
					struct
					{
						uint32_t LFuncExt : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief Processor Vendor */
				union
				{
					struct
					{
						char Vendor[4];
					};
					cpuid_t raw;
				} EBX;

				/** @brief Processor Vendor */
				union
				{
					struct
					{
						char Vendor[4];
					};
					cpuid_t raw;
				} ECX;

				/** @brief Processor Vendor */
				union
				{
					struct
					{
						char Vendor[4];
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief Extended Processor and Processor Feature Identifiers */
			struct CPUID0x80000001
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x80000001));
#endif // a64 || a32
				}

				/** @brief AMD Family, Model, Stepping */
				union
				{
					/* "This function is an identical copy of CPUID Fn0000_0001_EAX." */
					struct
					{
						uint32_t Stepping : 4;
						uint32_t BaseModel : 4;
						uint32_t BaseFamily : 4;
						uint32_t Reserved0 : 4;
						uint32_t ExtendedModel : 4;
						uint32_t ExtendedFamily : 8;
						uint32_t Reserved1 : 4;
					};
					cpuid_t raw;
				} EAX;

				/** @brief BrandId Identifier */
				union
				{
					struct
					{
						uint32_t BrandId : 16;
						uint32_t Reserved0 : 12;
						uint32_t PkgType : 4;
					};
					cpuid_t raw;
				} EBX;

				/** @brief Feature Identifiers */
				union
				{
					struct
					{
						uint32_t LahfSahf : 1;
						uint32_t CmpLegacy : 1;
						uint32_t SVM : 1;
						uint32_t ExtApicSpace : 1;
						uint32_t AltMovCr8 : 1;
						uint32_t ABM : 1;
						uint32_t SSE4A : 1;
						uint32_t MisAlignSse : 1;
						uint32_t ThreeDNowPrefetch : 1;
						uint32_t OSVW : 1;
						uint32_t IBS : 1;
						uint32_t XOP : 1;
						uint32_t SKINIT : 1;
						uint32_t WDT : 1;
						uint32_t Reserved0 : 1;
						uint32_t LWP : 1;
						uint32_t FMA4 : 1;
						uint32_t TCE : 1;
						uint32_t Reserved1 : 3;
						uint32_t TBM : 1;
						uint32_t TopologyExtensions : 1;
						uint32_t PerfCtrExtCore : 1;
						uint32_t PerfCtrExtNB : 1;
						uint32_t Reserved2 : 1;
						uint32_t DataBkptExt : 1;
						uint32_t PerfTsc : 1;
						uint32_t PerfCtrExtLLC : 1;
						uint32_t MONITORX : 1;
						uint32_t AddrMaskExt : 1;
						uint32_t Reserved3 : 1;
					};
					cpuid_t raw;
				} ECX;

				/** @brief Feature Identifiers */
				union
				{
					struct
					{
						uint32_t FPU : 1;
						uint32_t VME : 1;
						uint32_t DE : 1;
						uint32_t PSE : 1;
						uint32_t TSC : 1;
						uint32_t MSR : 1;
						uint32_t PAE : 1;
						uint32_t MCE : 1;
						uint32_t CMPXCHG8B : 1;
						uint32_t APIC : 1;
						uint32_t Reserved0 : 1;
						uint32_t SysCallSysRet : 1;
						uint32_t MTRR : 1;
						uint32_t PGE : 1;
						uint32_t MCA : 1;
						uint32_t CMOV : 1;
						uint32_t PAT : 1;
						uint32_t PSE36 : 1;
						uint32_t Reserved1 : 2;
						uint32_t NX : 1;
						uint32_t Reserved2 : 1;
						uint32_t MmxExt : 1;
						uint32_t MMX : 1;
						uint32_t FXSR : 1;
						uint32_t FFXSR : 1;
						uint32_t Page1GB : 1;
						uint32_t RDTSCP : 1;
						uint32_t Reserved3 : 1;
						uint32_t LM : 1;
						uint32_t ThreeDNowExtended : 1;
						uint32_t ThreeDNow : 1;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief Extended Processor Name String */
			struct CPUID0x80000002
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x80000002));
#endif // a64 || a32
				}

				union
				{
					struct
					{
						char Brand[4];
					};
					cpuid_t raw;
				} EAX;

				union
				{
					struct
					{
						char Brand[4];
					};
					cpuid_t raw;
				} EBX;

				union
				{
					struct
					{
						char Brand[4];
					};
					cpuid_t raw;
				} ECX;

				union
				{
					struct
					{
						char Brand[4];
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief Extended Processor Name String */
			struct CPUID0x80000003
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x80000003));
#endif // a64 || a32
				}

				union
				{
					struct
					{
						char Brand[4];
					};
					cpuid_t raw;
				} EAX;

				union
				{
					struct
					{
						char Brand[4];
					};
					cpuid_t raw;
				} EBX;

				union
				{
					struct
					{
						char Brand[4];
					};
					cpuid_t raw;
				} ECX;

				union
				{
					struct
					{
						char Brand[4];
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief Extended Processor Name String */
			struct CPUID0x80000004
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x80000004));
#endif // a64 || a32
				}

				union
				{
					struct
					{
						char Brand[4];
					};
					cpuid_t raw;
				} EAX;

				union
				{
					struct
					{
						char Brand[4];
					};
					cpuid_t raw;
				} EBX;

				union
				{
					struct
					{
						char Brand[4];
					};
					cpuid_t raw;
				} ECX;

				union
				{
					struct
					{
						char Brand[4];
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief L1 Cache and TLB Information */
			struct CPUID0x80000005
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x80000005));
#endif // a64 || a32
				}

				/** @brief L1 TLB 2M/4M Information */
				union
				{
					struct
					{
						uint32_t L1ITlb2and4MSize : 8;
						uint32_t L1ITlb2and4MAssoc : 8;
						uint32_t L1DTlb2and4MSize : 8;
						uint32_t L1DTlb2and4MAssoc : 8;
					};
					cpuid_t raw;
				} EAX;

				/** @brief L1 TLB 4K Information */
				union
				{
					struct
					{
						uint32_t L1ITlb4KSize : 8;
						uint32_t L1ITlb4KAssoc : 8;
						uint32_t L1DTlb4KSize : 8;
						uint32_t L1DTlb4KAssoc : 8;
					};
					cpuid_t raw;
				} EBX;

				/** @brief L1 Data Cache Information */
				union
				{
					struct
					{
						uint32_t L1DcLineSize : 8;
						uint32_t L1DcLinesPerTag : 8;
						uint32_t L1DcAssoc : 8;
						uint32_t L1DcSize : 8;
					};
					cpuid_t raw;
				} ECX;

				/** @brief L1 Instruction Cache Information */
				union
				{
					struct
					{
						uint32_t L1IcLineSize : 8;
						uint32_t L1IcLinesPerTag : 8;
						uint32_t L1IcAssoc : 8;
						uint32_t L1IcSize : 8;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief L2 Cache and TLB and L3 Cache Information */
			struct CPUID0x80000006
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x80000006));
#endif // a64 || a32
				}

				/** @brief L2 TLB 2M/4M Information */
				union
				{
					struct
					{
						uint32_t L2ITlb2and4MSize : 12;
						uint32_t L2ITlb2and4MAssoc : 4;
						uint32_t L2DTlb2and4MSize : 12;
						uint32_t L2DTlb2and4MAssoc : 4;
					};
					cpuid_t raw;
				} EAX;

				/** @brief L2 TLB 4K Information */
				union
				{
					struct
					{
						uint32_t L2ITlb4KSize : 12;
						uint32_t L2ITlb4KAssoc : 4;
						uint32_t L2DTlb4KSize : 12;
						uint32_t L2DTlb4KAssoc : 4;
					};
					cpuid_t raw;
				} EBX;

				/** @brief L2 Cache Information */
				union
				{
					struct
					{
						uint32_t L2LineSize : 8;
						uint32_t L2LinesPerTag : 4;
						uint32_t L2Assoc : 4;
						uint32_t L2Size : 16;
					};
					cpuid_t raw;
				} ECX;

				/** @brief L3 Cache Information */
				union
				{
					struct
					{
						uint32_t L3LineSize : 8;
						uint32_t L3LinesPerTag : 4;
						uint32_t L3Assoc : 4;
						uint32_t Reserved0 : 2;
						uint32_t L3Size : 14;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief Processor Power Management and RAS Capabilities */
			struct CPUID0x80000007
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x80000007));
#endif // a64 || a32
				}

				union
				{
					struct
					{
						uint32_t Reserved : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief RAS Capabilities */
				union
				{
					struct
					{
						uint32_t McaOverflowRecov : 1;
						uint32_t SUCCOR : 1;
						uint32_t HWA : 1;
						uint32_t ScalableMca : 1;
						uint32_t Reserved0 : 28;
					};
					cpuid_t raw;
				} EBX;

				/** @brief Processor Power Monitoring Interface */
				union
				{
					struct
					{
						uint32_t CpuPwrSampleTimeRatio : 32;
					};
					cpuid_t raw;
				} ECX;

				/** @brief Advanced Power Management Features */
				union
				{
					struct
					{
						uint32_t TS : 1;
						uint32_t FID : 1;
						uint32_t VID : 1;
						uint32_t TTP : 1;
						uint32_t TM : 1;
						uint32_t Reserved0 : 1;
						uint32_t OneHundredMHzSteps : 1;
						uint32_t HwPstate : 1;
						uint32_t TscInvariant : 1;
						uint32_t CPB : 1;
						uint32_t EffFreqRO : 1;
						uint32_t ProcFeedbackInterface : 1;
						uint32_t ProcPowerReporting : 1;
						uint32_t Reserved1 : 19;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief Processor Capacity Parameters and Extended Feature Identification */
			struct CPUID0x80000008
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x80000008));
#endif // a64 || a32
				}

				/** @brief Long Mode Size Identifiers */
				union
				{
					struct
					{
						uint32_t PhysAddrSize : 8;
						uint32_t LinAddrSize : 8;
						uint32_t GuestPhysAddrSize : 8;
						uint32_t Reserved0 : 8;
					};
					cpuid_t raw;
				} EAX;

				/** @brief Extended Feature Identifiers */
				union
				{
					struct
					{
						uint32_t CLZERO : 1;
						uint32_t InstRetCntMsr : 1;
						uint32_t RstrFpErrPtrs : 1;
						uint32_t INVLPGB : 1;
						uint32_t RDPRU : 1;
						uint32_t Reserved0 : 3;
						uint32_t MCOMMIT : 1;
						uint32_t WBNOINVD : 1;
						uint32_t Reserved1 : 2;
						uint32_t IBPB : 1;
						uint32_t INT_WBINVD : 1;
						uint32_t IBRS : 1;
						uint32_t STIBP : 1;
						uint32_t IbrsAlwaysOn : 1;
						uint32_t StibpAlwaysOn : 1;
						uint32_t IbrsPreferred : 1;
						uint32_t IbrsSameMode : 1;
						uint32_t EferLmsleUnsupported : 1;
						uint32_t INVLPGBnestedPages : 1;
						uint32_t Reserved2 : 2;
						uint32_t SSBD : 1;
						uint32_t SsbdVirtSpecCtrl : 1;
						uint32_t SsbdNotRequired : 1;
						uint32_t Reserved3 : 1;
						uint32_t PSFD : 1;
						uint32_t BTC_NO : 1;
						uint32_t Reserved4 : 2;
					};
					cpuid_t raw;
				} EBX;

				/** @brief Size Identifiers */
				union
				{
					struct
					{
						uint32_t NT : 8;
						uint32_t Reserved0 : 4;
						uint32_t ApicIdSize : 4;
						uint32_t PerfTscSize : 2;
						uint32_t Reserved1 : 14;
					};
					cpuid_t raw;
				} ECX;

				/** @brief RDPRU Register Identifier Range */
				union
				{
					struct
					{
						uint32_t InvlpgbCountMax : 16;
						uint32_t MaxRdpruID : 16;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief TODO */
			struct CPUID0x8000000A
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x8000000A));
#endif // a64 || a32
				}

				/** @brief  */
				union
				{
					struct
					{
						uint32_t SvmRev : 8;
						uint32_t Reserved : 24;
					};
					cpuid_t raw;
				} EAX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t NASID : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t Reserved : 32;
					};
					cpuid_t raw;
				} ECX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t NP : 1;
						uint32_t LbrVirt : 1;
						uint32_t SVML : 1;
						uint32_t NRIPS : 1;
						uint32_t TscRateMsr : 1;
						uint32_t VmcbClean : 1;
						uint32_t FlushByAsid : 1;
						uint32_t DecodeAssists : 1;
						uint32_t Reserved0 : 2;
						uint32_t PauseFilter : 1;
						uint32_t Reserved1 : 1;
						uint32_t PauseFilterThreshold : 1;
						uint32_t AVIC : 1;
						uint32_t Reserved2 : 1;
						uint32_t VMSAVEvirt : 1;
						uint32_t VGIF : 1;
						uint32_t GMET : 1;
						uint32_t x2AVIC : 1;
						uint32_t SSSCheck : 1;
						uint32_t SpecCtrl : 1;
						uint32_t ROGPT : 1;
						uint32_t Reserved3 : 1;
						uint32_t HOST_MCE_OVERRIDE : 1;
						uint32_t TlbiCtl : 1;
						uint32_t VNMI : 1;
						uint32_t IbsVirt : 1;
						uint32_t Reserved4 : 4;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief TODO */
			struct CPUID0x80000019
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x80000019));
#endif // a64 || a32
				}

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} ECX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief TODO */
			struct CPUID0x8000001A
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x8000001A));
#endif // a64 || a32
				}

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} ECX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief TODO */
			struct CPUID0x8000001B
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x8000001B));
#endif // a64 || a32
				}

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} ECX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief TODO */
			struct CPUID0x8000001C
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x8000001C));
#endif // a64 || a32
				}

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} ECX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief TODO */
			struct CPUID0x8000001D
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x8000001D));
#endif // a64 || a32
				}

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} ECX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief TODO */
			struct CPUID0x8000001E
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x8000001E));
#endif // a64 || a32
				}

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} ECX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief TODO */
			struct CPUID0x8000001F
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x8000001F));
#endif // a64 || a32
				}

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} ECX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief TODO */
			struct CPUID0x80000020
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x80000020));
#endif // a64 || a32
				}

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} ECX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief TODO */
			struct CPUID0x80000021
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x80000021));
#endif // a64 || a32
				}

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} ECX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief TODO */
			struct CPUID0x80000022
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x80000022));
#endif // a64 || a32
				}

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} ECX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief TODO */
			struct CPUID0x80000023
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x80000023));
#endif // a64 || a32
				}

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} ECX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/** @brief TODO */
			struct CPUID0x80000026
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x80000026));
#endif // a64 || a32
				}

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EAX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EBX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} ECX;

				/** @brief  */
				union
				{
					struct
					{
						uint32_t todo : 32;
					};
					cpuid_t raw;
				} EDX;
			};

			/**
			 * AMD Easter Egg
			 *
			 * AMD K6:     "NexGen‍erationAMD"
			 * AMD K8:     "IT'S HAMMER TIME"
			 * AMD JAGUAR: "HELLO KITTY! ^-^"
			 *
			 * @note This cpuid is not found on AMD's manual
			 */
			struct CPUID0x8FFFFFFF
			{
				__always_inline inline void Get()
				{
#if defined(a86)
					asmv("cpuid"
						 : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
						 : "a"(0x8FFFFFFF));
#endif // a64 || a32
				}

				/**
				 * String
				 */
				union
				{
					struct
					{
						char String[4];
					};
					cpuid_t raw;
				} EAX;

				/**
				 * String
				 */
				union
				{
					struct
					{
						char String[4];
					};
					cpuid_t raw;
				} EBX;

				/**
				 * String
				 */
				union
				{
					struct
					{
						char String[4];
					};
					cpuid_t raw;
				} ECX;

				/**
				 * String
				 */
				union
				{
					struct
					{
						char String[4];
					};
					cpuid_t raw;
				} EDX;
			};
		}
	}
}

#endif // !__FENNIX_KERNEL_CPU_x86_CPUID_AMD_H__
