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

#ifndef __FENNIX_KERNEL_CPU_x64_CPUID_AMD_H__
#define __FENNIX_KERNEL_CPU_x64_CPUID_AMD_H__

#include <types.h>

namespace CPU
{
    namespace x64
    {
        /** @brief EXPERIMENTAL IMPLEMENTATION */
        namespace AMD
        {
            /** @brief Basic CPU information */
            struct CPUID0x00000000
            {
                /** @brief Largest Standard Function Number */
                union
                {
                    struct
                    {
                        uint64_t LFuncStd : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief Processor Vendor */
                union
                {
                    struct
                    {
                        char Vendor[4];
                    };
                    uint64_t raw;
                } EBX;

                /** @brief Processor Vendor */
                union
                {
                    struct
                    {
                        char Vendor[4];
                    };
                    uint64_t raw;
                } ECX;

                /** @brief Processor Vendor */
                union
                {
                    struct
                    {
                        char Vendor[4];
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Additional CPU information */
            struct CPUID0x00000001
            {
                /** @brief Family, Model, Stepping Identifiers */
                union
                {
                    /* "This function is an identical copy of CPUID Fn8000_0001_EAX." */
                    struct
                    {
                        uint64_t Stepping : 4;
                        uint64_t BaseModel : 4;
                        uint64_t BaseFamily : 4;
                        uint64_t Reserved0 : 4;
                        uint64_t ExtModel : 4;
                        uint64_t ExtFamily : 8;
                        uint64_t Reserved1 : 4;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief LocalApicId, LogicalProcessorCount, CLFlush */
                union
                {
                    struct
                    {
                        uint64_t BrandId : 8;
                        uint64_t CLFlush : 8;
                        uint64_t LogicalProcessorCount : 8;
                        uint64_t LocalApicId : 8;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief Feature Identifiers */
                union
                {
                    struct
                    {
                        uint64_t SSE3 : 1;
                        uint64_t PCLMULQDQ : 1;
                        uint64_t Reserved0 : 1;
                        uint64_t MONITOR : 1;
                        uint64_t Reserved1 : 5;
                        uint64_t SSSE3 : 1;
                        uint64_t Reserved2 : 2;
                        uint64_t FMA : 1;
                        uint64_t CMPXCHG16B : 1;
                        uint64_t Reserved3 : 5;
                        uint64_t SSE41 : 1;
                        uint64_t SSE42 : 1;
                        uint64_t x2APIC : 1;
                        uint64_t MOVBE : 1;
                        uint64_t POPCNT : 1;
                        uint64_t Reserved4 : 1;
                        uint64_t AES : 1;
                        uint64_t XSAVE : 1;
                        uint64_t OSXSAVE : 1;
                        uint64_t AVX : 1;
                        uint64_t F16C : 1;
                        uint64_t RDRAND : 1;

                        /**
                         * @brief Reserved for use by hypervisor to indicate guest status.
                         */
                        uint64_t Hypervisor : 1;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief Feature Identifiers */
                union
                {
                    struct
                    {
                        uint64_t FPU : 1;
                        uint64_t VME : 1;
                        uint64_t DE : 1;
                        uint64_t PSE : 1;
                        uint64_t TSC : 1;
                        uint64_t MSR : 1;
                        uint64_t PAE : 1;
                        uint64_t MCE : 1;
                        uint64_t CMPXCHG8B : 1;
                        uint64_t APIC : 1;
                        uint64_t Reserved0 : 1;
                        uint64_t SysEnterSysExit : 1;
                        uint64_t MTRR : 1;
                        uint64_t PGE : 1;
                        uint64_t MCA : 1;
                        uint64_t CMOV : 1;
                        uint64_t PAT : 1;
                        uint64_t PSE36 : 1;
                        uint64_t Reserved1 : 1;
                        uint64_t CLFSH : 1;
                        uint64_t Reserved2 : 3;
                        uint64_t MMX : 1;
                        uint64_t FXSR : 1;
                        uint64_t SSE : 1;
                        uint64_t SSE2 : 1;
                        uint64_t Reserved3 : 1;
                        uint64_t HTT : 1;
                        uint64_t Reserved4 : 3;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Monitor and MWait Features */
            struct CPUID0x00000005
            {
                /** @brief Monitor/MWait */
                union
                {
                    struct
                    {
                        uint64_t MonLineSizeMin : 16;
                        uint64_t Reserved0 : 16;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief Monitor/MWait */
                union
                {
                    struct
                    {
                        uint64_t MonLineSizeMax : 16;
                        uint64_t Reserved0 : 16;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief Monitor/MWait */
                union
                {
                    struct
                    {
                        uint64_t EMX : 1;
                        uint64_t IBE : 1;
                        uint64_t Reserved0 : 30;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief Reserved */
                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Power Management Related Features */
            struct CPUID0x00000006
            {
                /** @brief Local APIC Timer Invariance */
                union
                {
                    struct
                    {
                        uint64_t Reserved0 : 2;
                        uint64_t ARAT : 1;
                        uint64_t Reserved1 : 29;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief Reserved */
                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief Effective Processor Frequency Interface */
                union
                {
                    struct
                    {
                        uint64_t EffFreq : 1;
                        uint64_t Reserved0 : 31;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief Reserved */
                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Structured Extended Feature Identifiers */
            struct CPUID0x00000007
            {
                /** @brief Structured Extended Feature Identifiers */
                union
                {
                    struct
                    {
                        uint64_t MaxSubFn : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief Structured Extended Feature Identifiers */
                union
                {
                    struct
                    {
                        uint64_t FSGSBASE : 1;
                        uint64_t Reserved0 : 2;
                        uint64_t BMI1 : 1;
                        uint64_t Reserved1 : 1;
                        uint64_t AVX2 : 1;
                        uint64_t Reserved2 : 1;
                        uint64_t SMEP : 1;
                        uint64_t BMI2 : 1;
                        uint64_t Reserved3 : 1;
                        uint64_t INVPCID : 1;
                        uint64_t Reserved4 : 1;
                        uint64_t PQM : 1;
                        uint64_t Reserved5 : 2;
                        uint64_t PQE : 1;
                        uint64_t Reserved6 : 2;
                        uint64_t RDSEED : 1;
                        uint64_t ADX : 1;
                        uint64_t SMAP : 1;
                        uint64_t Reserved7 : 1;
                        uint64_t RDPID : 1;
                        uint64_t CLFLUSHOPT : 1;
                        uint64_t CLWB : 1;
                        uint64_t Reserved8 : 4;
                        uint64_t SHA : 1;
                        uint64_t Reserved9 : 2;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief Structured Extended Feature Identifiers */
                union
                {
                    struct
                    {
                        uint64_t Reserved0 : 2;
                        uint64_t UMIP : 1;
                        uint64_t PKU : 1;
                        uint64_t OSPKE : 1;
                        uint64_t Reserved1 : 2;
                        uint64_t CET_SS : 1;
                        uint64_t Reserved2 : 1;
                        uint64_t VAES : 1;
                        uint64_t VPCLMULQDQ : 1;
                        uint64_t Reserved3 : 5;
                        uint64_t LA57 : 1;
                        uint64_t Reserved4 : 15;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief Structured Extended Feature Identifiers */
                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Thread Level - Extended Topology Enumeration */
            struct CPUID0x0000000B_ECX_0
            {
                /** @brief Extended Topology Enumeration */
                union
                {
                    struct
                    {
                        uint64_t ThreadMaskWidth : 5;
                        uint64_t Reserved0 : 27;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief Extended Topology Enumeration */
                union
                {
                    struct
                    {
                        uint64_t NumberOfThreadsInACore : 5; /* No field name */
                        uint64_t Reserved0 : 27;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief Extended Topology Enumeration */
                union
                {
                    struct
                    {
                        uint64_t ECXInputValue : 8;
                        uint64_t LevelNumber : 8;
                        uint64_t Reserved0 : 16;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief Extended Topology Enumeration */
                union
                {
                    struct
                    {
                        uint64_t x2APID_ID : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Core Level - Extended Topology Enumeration */
            struct CPUID0x0000000B_ECX_1
            {
                /** @brief Extended Topology Enumeration */
                union
                {
                    struct
                    {
                        uint64_t CoreMaskWidth : 5;
                        uint64_t Reserved0 : 27;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief Extended Topology Enumeration */
                union
                {
                    struct
                    {
                        uint64_t NumberOfLogicalCoresInSocket : 5; /* No field name */
                        uint64_t Reserved0 : 27;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief Extended Topology Enumeration */
                union
                {
                    struct
                    {
                        uint64_t ECXInputValue : 8;
                        uint64_t LevelNumber : 8;
                        uint64_t Reserved0 : 16;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief Extended Topology Enumeration */
                union
                {
                    struct
                    {
                        uint64_t x2APID_ID : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Processor Extended State Enumeration */
            struct CPUID0x0000000D_ECX_0
            {
                /** @brief Processor Extended State Enumeration */
                union
                {
                    struct
                    {
                        uint64_t XFeatureSupportedMask : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief Processor Extended State Enumeration */
                union
                {
                    struct
                    {
                        uint64_t XFeatureEnabledSizeMax : 32;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief Processor Extended State Enumeration */
                union
                {
                    struct
                    {
                        uint64_t XFeatureSupportedSizeMax : 32;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief Processor Extended State Enumeration */
                union
                {
                    struct
                    {
                        uint64_t XFeatureSupportedMask : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Processor Extended State Enumeration */
            struct CPUID0x0000000D_ECX_1
            {
                /** @brief Processor Extended State Enumeration */
                union
                {
                    struct
                    {
                        uint64_t XSAVEOPT : 1;
                        uint64_t XSAVEC : 1;
                        uint64_t XGETBV : 1;
                        uint64_t XSAVES : 1;
                        uint64_t Reserved0 : 28;
                    };
                    uint64_t raw;
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
                        uint64_t ebx : 32;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief Processor Extended State Enumeration */
                union
                {
                    struct
                    {
                        uint64_t Reserved0 : 11;
                        uint64_t CET_U : 1;
                        uint64_t CET_S : 1;
                        uint64_t Reserved1 : 19;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief Processor Extended State Enumeration */
                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Processor Extended State Enumeration */
            struct CPUID0x0000000D_ECX_2
            {
                /** @brief Processor Extended State Enumeration */
                union
                {
                    struct
                    {
                        uint64_t YmmSaveStateSize : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief Processor Extended State Enumeration */
                union
                {
                    struct
                    {
                        uint64_t YmmSaveStateOffset : 32;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief Processor Extended State Enumeration */
                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief Processor Extended State Enumeration */
                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Processor Extended State Emulation */
            struct CPUID0x0000000D_ECX_11
            {
                /** @brief Processor Extended State Emulation */
                union
                {
                    struct
                    {
                        uint64_t CetSupervisorSize : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief Processor Extended State Emulation */
                union
                {
                    struct
                    {
                        uint64_t CetSupervisorOffset : 32;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief Processor Extended State Emulation */
                union
                {
                    struct
                    {
                        uint64_t Reserved : 31;
                        uint64_t US : 1;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief Processor Extended State Emulation */
                union
                {
                    struct
                    {
                        uint64_t Unused : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Processor Extended State Enumeration */
            struct CPUID0x0000000D_ECX_3H
            {
                /** @brief Processor Extended State Enumeration */
                union
                {
                    struct
                    {
                        uint64_t LwpSaveStateSize : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief Processor Extended State Enumeration */
                union
                {
                    struct
                    {
                        uint64_t LwpSaveStateOffset : 32;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief Processor Extended State Enumeration */
                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief Processor Extended State Enumeration */
                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Maximum Extended Function Number and Vendor String */
            struct CPUID0x80000000
            {
                /** @brief Largest Extended Function Number */
                union
                {
                    struct
                    {
                        uint64_t LFuncExt : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief Processor Vendor */
                union
                {
                    struct
                    {
                        char Vendor[4];
                    };
                    uint64_t raw;
                } EBX;

                /** @brief Processor Vendor */
                union
                {
                    struct
                    {
                        char Vendor[4];
                    };
                    uint64_t raw;
                } ECX;

                /** @brief Processor Vendor */
                union
                {
                    struct
                    {
                        char Vendor[4];
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Extended Processor and Processor Feature Identifiers */
            struct CPUID0x80000001
            {
                /** @brief AMD Family, Model, Stepping */
                union
                {
                    /* "This function is an identical copy of CPUID Fn0000_0001_EAX." */
                    struct
                    {
                        uint64_t Stepping : 4;
                        uint64_t BaseModel : 4;
                        uint64_t BaseFamily : 4;
                        uint64_t Reserved0 : 4;
                        uint64_t ExtendedModel : 4;
                        uint64_t ExtendedFamily : 8;
                        uint64_t Reserved1 : 4;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief BrandId Identifier */
                union
                {
                    struct
                    {
                        uint64_t BrandId : 16;
                        uint64_t Reserved0 : 12;
                        uint64_t PkgType : 4;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief Feature Identifiers */
                union
                {
                    struct
                    {
                        uint64_t LahfSahf : 1;
                        uint64_t CmpLegacy : 1;
                        uint64_t SVM : 1;
                        uint64_t ExtApicSpace : 1;
                        uint64_t AltMovCr8 : 1;
                        uint64_t ABM : 1;
                        uint64_t SSE4A : 1;
                        uint64_t MisAlignSse : 1;
                        uint64_t ThreeDNowPrefetch : 1;
                        uint64_t OSVW : 1;
                        uint64_t IBS : 1;
                        uint64_t XOP : 1;
                        uint64_t SKINIT : 1;
                        uint64_t WDT : 1;
                        uint64_t Reserved0 : 1;
                        uint64_t LWP : 1;
                        uint64_t FMA4 : 1;
                        uint64_t TCE : 1;
                        uint64_t Reserved1 : 3;
                        uint64_t TBM : 1;
                        uint64_t TopologyExtensions : 1;
                        uint64_t PerfCtrExtCore : 1;
                        uint64_t PerfCtrExtNB : 1;
                        uint64_t Reserved2 : 1;
                        uint64_t DataBkptExt : 1;
                        uint64_t PerfTsc : 1;
                        uint64_t PerfCtrExtLLC : 1;
                        uint64_t MONITORX : 1;
                        uint64_t AddrMaskExt : 1;
                        uint64_t Reserved3 : 1;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief Feature Identifiers */
                union
                {
                    struct
                    {
                        uint64_t FPU : 1;
                        uint64_t VME : 1;
                        uint64_t DE : 1;
                        uint64_t PSE : 1;
                        uint64_t TSC : 1;
                        uint64_t MSR : 1;
                        uint64_t PAE : 1;
                        uint64_t MCE : 1;
                        uint64_t CMPXCHG8B : 1;
                        uint64_t APIC : 1;
                        uint64_t Reserved0 : 1;
                        uint64_t SysCallSysRet : 1;
                        uint64_t MTRR : 1;
                        uint64_t PGE : 1;
                        uint64_t MCA : 1;
                        uint64_t CMOV : 1;
                        uint64_t PAT : 1;
                        uint64_t PSE36 : 1;
                        uint64_t Reserved1 : 2;
                        uint64_t NX : 1;
                        uint64_t Reserved2 : 1;
                        uint64_t MmxExt : 1;
                        uint64_t MMX : 1;
                        uint64_t FXSR : 1;
                        uint64_t FFXSR : 1;
                        uint64_t Page1GB : 1;
                        uint64_t RDTSCP : 1;
                        uint64_t Reserved3 : 1;
                        uint64_t LM : 1;
                        uint64_t ThreeDNowExtended : 1;
                        uint64_t ThreeDNow : 1;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Extended Processor Name String */
            struct CPUID0x80000002
            {
                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Extended Processor Name String */
            struct CPUID0x80000003
            {
                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Extended Processor Name String */
            struct CPUID0x80000004
            {
                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief L1 Cache and TLB Information */
            struct CPUID0x80000005
            {
                /** @brief L1 TLB 2M/4M Information */
                union
                {
                    struct
                    {
                        uint64_t L1ITlb2and4MSize : 8;
                        uint64_t L1ITlb2and4MAssoc : 8;
                        uint64_t L1DTlb2and4MSize : 8;
                        uint64_t L1DTlb2and4MAssoc : 8;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief L1 TLB 4K Information */
                union
                {
                    struct
                    {
                        uint64_t L1ITlb4KSize : 8;
                        uint64_t L1ITlb4KAssoc : 8;
                        uint64_t L1DTlb4KSize : 8;
                        uint64_t L1DTlb4KAssoc : 8;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief L1 Data Cache Information */
                union
                {
                    struct
                    {
                        uint64_t L1DcLineSize : 8;
                        uint64_t L1DcLinesPerTag : 8;
                        uint64_t L1DcAssoc : 8;
                        uint64_t L1DcSize : 8;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief L1 Instruction Cache Information */
                union
                {
                    struct
                    {
                        uint64_t L1IcLineSize : 8;
                        uint64_t L1IcLinesPerTag : 8;
                        uint64_t L1IcAssoc : 8;
                        uint64_t L1IcSize : 8;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief L2 Cache and TLB and L3 Cache Information */
            struct CPUID0x80000006
            {
                /** @brief L2 TLB 2M/4M Information */
                union
                {
                    struct
                    {
                        uint64_t L2ITlb2and4MSize : 12;
                        uint64_t L2ITlb2and4MAssoc : 4;
                        uint64_t L2DTlb2and4MSize : 12;
                        uint64_t L2DTlb2and4MAssoc : 4;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief L2 TLB 4K Information */
                union
                {
                    struct
                    {
                        uint64_t L2ITlb4KSize : 12;
                        uint64_t L2ITlb4KAssoc : 4;
                        uint64_t L2DTlb4KSize : 12;
                        uint64_t L2DTlb4KAssoc : 4;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief L2 Cache Information */
                union
                {
                    struct
                    {
                        uint64_t L2LineSize : 8;
                        uint64_t L2LinesPerTag : 4;
                        uint64_t L2Assoc : 4;
                        uint64_t L2Size : 16;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief L3 Cache Information */
                union
                {
                    struct
                    {
                        uint64_t L3LineSize : 8;
                        uint64_t L3LinesPerTag : 4;
                        uint64_t L3Assoc : 4;
                        uint64_t Reserved0 : 2;
                        uint64_t L3Size : 14;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Processor Power Management and RAS Capabilities */
            struct CPUID0x80000007
            {
                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief RAS Capabilities */
                union
                {
                    struct
                    {
                        uint64_t McaOverflowRecov : 1;
                        uint64_t SUCCOR : 1;
                        uint64_t HWA : 1;
                        uint64_t ScalableMca : 1;
                        uint64_t Reserved0 : 28;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief Processor Power Monitoring Interface */
                union
                {
                    struct
                    {
                        uint64_t CpuPwrSampleTimeRatio : 32;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief Advanced Power Management Features */
                union
                {
                    struct
                    {
                        uint64_t TS : 1;
                        uint64_t FID : 1;
                        uint64_t VID : 1;
                        uint64_t TTP : 1;
                        uint64_t TM : 1;
                        uint64_t Reserved0 : 1;
                        uint64_t OneHundredMHzSteps : 1;
                        uint64_t HwPstate : 1;
                        uint64_t TscInvariant : 1;
                        uint64_t CPB : 1;
                        uint64_t EffFreqRO : 1;
                        uint64_t ProcFeedbackInterface : 1;
                        uint64_t ProcPowerReporting : 1;
                        uint64_t Reserved1 : 19;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Processor Capacity Parameters and Extended Feature Identification */
            struct CPUID0x80000008
            {
                /** @brief Long Mode Size Identifiers */
                union
                {
                    struct
                    {
                        uint64_t PhysAddrSize : 8;
                        uint64_t LinAddrSize : 8;
                        uint64_t GuestPhysAddrSize : 8;
                        uint64_t Reserved0 : 8;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief Extended Feature Identifiers */
                union
                {
                    struct
                    {
                        uint64_t CLZERO : 1;
                        uint64_t InstRetCntMsr : 1;
                        uint64_t RstrFpErrPtrs : 1;
                        uint64_t INVLPGB : 1;
                        uint64_t RDPRU : 1;
                        uint64_t Reserved0 : 3;
                        uint64_t MCOMMIT : 1;
                        uint64_t WBNOINVD : 1;
                        uint64_t Reserved1 : 2;
                        uint64_t IBPB : 1;
                        uint64_t INT_WBINVD : 1;
                        uint64_t IBRS : 1;
                        uint64_t STIBP : 1;
                        uint64_t IbrsAlwaysOn : 1;
                        uint64_t StibpAlwaysOn : 1;
                        uint64_t IbrsPreferred : 1;
                        uint64_t IbrsSameMode : 1;
                        uint64_t EferLmsleUnsupported : 1;
                        uint64_t INVLPGBnestedPages : 1;
                        uint64_t Reserved2 : 2;
                        uint64_t SSBD : 1;
                        uint64_t SsbdVirtSpecCtrl : 1;
                        uint64_t SsbdNotRequired : 1;
                        uint64_t Reserved3 : 1;
                        uint64_t PSFD : 1;
                        uint64_t BTC_NO : 1;
                        uint64_t Reserved4 : 2;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief Size Identifiers */
                union
                {
                    struct
                    {
                        uint64_t NT : 8;
                        uint64_t Reserved0 : 4;
                        uint64_t ApicIdSize : 4;
                        uint64_t PerfTscSize : 2;
                        uint64_t Reserved1 : 14;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief RDPRU Register Identifier Range */
                union
                {
                    struct
                    {
                        uint64_t InvlpgbCountMax : 16;
                        uint64_t MaxRdpruID : 16;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief TODO */
            struct CPUID0x8000000A
            {
                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief TODO */
            struct CPUID0x80000019
            {
                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief TODO */
            struct CPUID0x8000001A
            {
                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief TODO */
            struct CPUID0x8000001B
            {
                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief TODO */
            struct CPUID0x8000001C
            {
                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief TODO */
            struct CPUID0x8000001D
            {
                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief TODO */
            struct CPUID0x8000001E
            {
                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief TODO */
            struct CPUID0x8000001F
            {
                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief TODO */
            struct CPUID0x80000020
            {
                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief TODO */
            struct CPUID0x80000021
            {
                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief TODO */
            struct CPUID0x80000022
            {
                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief TODO */
            struct CPUID0x80000023
            {
                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief TODO */
            struct CPUID0x80000026
            {
                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EAX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EBX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } ECX;

                /** @brief  */
                union
                {
                    struct
                    {
                        uint64_t todo : 32;
                    };
                    uint64_t raw;
                } EDX;
            };
        }
    }
}

#endif // !__FENNIX_KERNEL_CPU_x64_CPUID_AMD_H__
