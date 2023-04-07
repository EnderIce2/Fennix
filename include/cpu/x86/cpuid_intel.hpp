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

#ifndef __FENNIX_KERNEL_CPU_x86_CPUID_INTEL_H__
#define __FENNIX_KERNEL_CPU_x86_CPUID_INTEL_H__

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
        namespace Intel
        {
            /** @brief Basic CPU information */
            struct CPUID0x00000000
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
#endif // a64 || a32
                }

                union
                {
                    struct
                    {
                        uint32_t HighestFunctionSupported : 32;
                    };
                    cpuid_t raw;
                } EAX;
                union
                {
                    struct
                    {
                        char rbx[4];
                    };
                    cpuid_t raw;
                } EBX;
                union
                {
                    struct
                    {
                        char rcx[4];
                    };
                    cpuid_t raw;
                } ECX;
                union
                {
                    struct
                    {
                        char rdx[4];
                    };
                    cpuid_t raw;
                } EDX;
            };

            /** @brief Additional CPU information */
            struct CPUID0x00000001
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
#endif // a64 || a32
                }

                union
                {
                    struct
                    {
                        uint32_t SteppingID : 4;
                        uint32_t ModelID : 4;
                        uint32_t FamilyID : 4;
                        uint32_t Type : 2;
                        uint32_t Reserved0 : 2;
                        uint32_t ExtendedModel : 4;
                        uint32_t ExtendedFamily : 8;
                        uint32_t Reserved1 : 4;
                    };
                    cpuid_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t BrandIndex : 8;
                        uint32_t CLFLUSHLineSize : 8;
                        uint32_t LogicalProcessorsPerPackage : 8;
                        uint32_t LocalAPICID : 8;
                    };
                    cpuid_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t SSE3 : 1;
                        uint32_t PCLMULQDQ : 1;
                        uint32_t DTES64 : 1;
                        uint32_t MONITOR : 1;
                        uint32_t DS_CPL : 1;
                        uint32_t VMX : 1;
                        uint32_t SMX : 1;
                        uint32_t EIST : 1;
                        uint32_t TM2 : 1;
                        uint32_t SSSE3 : 1;
                        uint32_t CNXT_ID : 1;
                        uint32_t Reserved0 : 1;
                        uint32_t FMA : 1;
                        uint32_t CMPXCHG16B : 1;
                        uint32_t xTPRUpdateControl : 1;
                        uint32_t PDCM : 1;
                        uint32_t Reserved1 : 1;
                        uint32_t PCID : 1;
                        uint32_t DCA : 1;
                        uint32_t SSE4_1 : 1;
                        uint32_t SSE4_2 : 1;
                        uint32_t x2APIC : 1;
                        uint32_t MOVBE : 1;
                        uint32_t POPCNT : 1;
                        uint32_t TSCDeadline : 1;
                        uint32_t AES : 1;
                        uint32_t XSAVE : 1;
                        uint32_t OSXSAVE : 1;
                        uint32_t AVX : 1;
                        uint32_t F16C : 1;
                        uint32_t RDRAND : 1;
                        uint32_t Reserved2 : 1;
                        uint32_t Hypervisor : 1;
                    };
                    cpuid_t raw;
                } ECX;

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
                        uint32_t CX8 : 1;
                        uint32_t APIC : 1;
                        uint32_t Reserved0 : 1;
                        uint32_t SEP : 1;
                        uint32_t MTRR : 1;
                        uint32_t PGE : 1;
                        uint32_t MCA : 1;
                        uint32_t CMOV : 1;
                        uint32_t PAT : 1;
                        uint32_t PSE36 : 1;
                        uint32_t PSN : 1;
                        uint32_t CLFSH : 1;
                        uint32_t Reserved1 : 1;
                        uint32_t DS : 1;
                        uint32_t ACPI : 1;
                        uint32_t MMX : 1;
                        uint32_t FXSR : 1;
                        uint32_t SSE : 1;
                        uint32_t SSE2 : 1;
                        uint32_t SS : 1;
                        uint32_t HTT : 1;
                        uint32_t TM : 1;
                        uint32_t Reserved2 : 1;
                        uint32_t PBE : 1;
                    };
                    cpuid_t raw;
                } EDX;
            };

            /** @brief CPU cache and TLB */
            struct CPUID0x00000002
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
#endif // a64 || a32
                }

                union
                {
                    struct
                    {
                        uint32_t CacheLineSize : 8;
                        uint32_t CacheLinesPerTag : 8;
                        uint32_t Associativity : 8;
                        uint32_t CacheSize : 8;
                    };
                    cpuid_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t CacheLineSize : 8;
                        uint32_t CacheLinesPerTag : 8;
                        uint32_t Associativity : 8;
                        uint32_t CacheSize : 8;
                    };
                    cpuid_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t CacheLineSize : 8;
                        uint32_t CacheLinesPerTag : 8;
                        uint32_t Associativity : 8;
                        uint32_t CacheSize : 8;
                    };
                    cpuid_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t CacheLineSize : 8;
                        uint32_t CacheLinesPerTag : 8;
                        uint32_t Associativity : 8;
                        uint32_t CacheSize : 8;
                    };
                    cpuid_t raw;
                } EDX;
            };

            /** @brief CPU serial number */
            struct CPUID0x00000003
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
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

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    cpuid_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t ProcessorSerialNumber : 32;
                    };
                    cpuid_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t ProcessorSerialNumber : 32;
                    };
                    cpuid_t raw;
                } EDX;
            };

            /** @brief Cache information */
            struct CPUID0x00000004_1
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
#endif // a64 || a32
                }

                union
                {
                    struct
                    {
                        uint32_t Type : 5;
                        uint32_t Level : 3;
                        uint32_t SelfInitializing : 1;
                        uint32_t FullyAssociative : 1;
                        uint32_t Reserved : 4;
                        uint32_t MaxAddressableIdsForLogicalProcessors : 12;
                        uint32_t CoresPerPackage : 6;
                    };
                    cpuid_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t SystemCoherencyLineSize : 12;
                        uint32_t PhysicalLinePartitions : 10;
                        uint32_t WaysOfAssociativity : 10;
                    };
                    cpuid_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    cpuid_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    cpuid_t raw;
                } EDX;
            };

            /** @brief MONITOR information */
            struct CPUID0x00000005
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
#endif // a64 || a32
                }

                union
                {
                    struct
                    {
                        uint32_t SmallestMonitorLineSize : 16;
                        uint32_t Reserved : 16;
                    };
                    cpuid_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t LargestMonitorLineSize : 16;
                        uint32_t Reserved : 16;
                    };
                    cpuid_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t MWAITEnumerationSupported : 1;
                        uint32_t InterruptsAsBreakEvent : 1;
                        uint32_t Reserved : 30;
                    };
                    cpuid_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t C0 : 4;
                        uint32_t C1 : 4;
                        uint32_t C2 : 4;
                        uint32_t C3 : 4;
                        uint32_t C4 : 4;
                        uint32_t Reserved : 12;
                    };
                    cpuid_t raw;
                } EDX;
            };

            /** @brief Thermal and power management information */
            struct CPUID0x00000006
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
#endif // a64 || a32
                }

                union
                {
                    struct
                    {
                        uint32_t SensorSupported : 1;
                        uint32_t Reserved : 31;
                    };
                    cpuid_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t InterruptThreshold : 4;
                        uint32_t Reserved : 26;
                    };
                    cpuid_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t ACNT_MCNT : 1;
                        uint32_t Reserved : 31;
                    };
                    cpuid_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    cpuid_t raw;
                } EDX;
            };

            /** @brief Extended feature flags enumeration */
            struct CPUID0x00000007_0
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
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

                union
                {
                    struct
                    {
                        /** @brief Access to base of fs and gs */
                        uint32_t FSGSBase : 1;
                        /** @brief IA32_TSC_ADJUST MSR */
                        uint32_t IA32TSCAdjust : 1;
                        /** @brief Software Guard Extensions */
                        uint32_t SGX : 1;
                        /** @brief Bit Manipulation Instruction Set 1 */
                        uint32_t BMI1 : 1;
                        /** @brief TSX Hardware Lock Elision */
                        uint32_t HLE : 1;
                        /** @brief Advanced Vector Extensions 2 */
                        uint32_t AVX2 : 1;
                        /** @brief FDP_EXCPTN_ONLY */
                        uint32_t FDPExcptonOnly : 1;
                        /** @brief Supervisor Mode Execution Protection */
                        uint32_t SMEP : 1;
                        /** @brief Bit Manipulation Instruction Set 2 */
                        uint32_t BMI2 : 1;
                        /** @brief Enhanced REP MOVSB/STOSB */
                        uint32_t ERMS : 1;
                        /** @brief INVPCID */
                        uint32_t INVPCID : 1;
                        /** @brief RTM */
                        uint32_t RTM : 1;
                        /** @brief Intel Resource Director Monitoring */
                        uint32_t RDT_M : 1;
                        /** @brief Deprecates FPU CS and DS values */
                        uint32_t DeprecatesFPU : 1;
                        /** @brief Intel Memory Protection Extensions */
                        uint32_t MPX : 1;
                        /** @brief Intel Resource Director Allocation */
                        uint32_t RDT_A : 1;
                        /** @brief AVX-512 Foundation */
                        uint32_t AVX512F : 1;
                        /** @brief AVX-512 Doubleword and Quadword Instructions */
                        uint32_t AVX512DQ : 1;
                        /** @brief RDSEED */
                        uint32_t RDSEED : 1;
                        /** @brief Intel Multi-Precision Add-Carry Instruction Extensions */
                        uint32_t ADX : 1;
                        /** @brief Supervisor Mode Access Prevention */
                        uint32_t SMAP : 1;
                        /** @brief AVX-512 Integer Fused Multiply-Add Instructions */
                        uint32_t AVX512IFMA : 1;
                        /** @brief Reserved */
                        uint32_t Reserved : 1;
                        /** @brief CLFLUSHOPT */
                        uint32_t CLFLUSHOPT : 1;
                        /** @brief CLWB */
                        uint32_t CLWB : 1;
                        /** @brief Intel Processor Trace */
                        uint32_t IntelProcessorTrace : 1;
                        /** @brief AVX-512 Prefetch Instructions */
                        uint32_t AVX512PF : 1;
                        /** @brief AVX-512 Exponential and Reciprocal Instructions */
                        uint32_t AVX512ER : 1;
                        /** @brief AVX-512 Conflict Detection Instructions */
                        uint32_t AVX512CD : 1;
                        /** @brief SHA Extensions */
                        uint32_t SHA : 1;
                        /** @brief AVX-512 Byte and Word Instructions */
                        uint32_t AVX512BW : 1;
                        /** @brief AVX-512 Vector Length Extensions */
                        uint32_t AVX512VL : 1;
                    };
                    cpuid_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        /** @brief PREFETCHWT1 */
                        uint32_t PREFETCHWT1 : 1;
                        /** @brief AVX-512 Vector Bit Manipulation Instructions */
                        uint32_t AVX512VBMI : 1;
                        /** @brief User Mode Instruction Prevention */
                        uint32_t UMIP : 1;
                        /** @brief Memory Protection Keys for User-mode pages */
                        uint32_t PKU : 1;
                        /** @brief PKU enabled by OS */
                        uint32_t OSPKE : 1;
                        /** @brief Timed pause and user-level monitor/wait */
                        uint32_t WaitPKG : 1;
                        /** @brief AVX-512 Vector Bit Manipulation Instructions 2 */
                        uint32_t AVX512VBMI2 : 1;
                        /** @brief Control flow enforcement (CET) shadow stack */
                        uint32_t CET_SS : 1;
                        /** @brief Galois Field instructions */
                        uint32_t GFNI : 1;
                        /** @brief Vector AES instruction set (VEX-256/EVEX) */
                        uint32_t VAES : 1;
                        /** @brief CLMUL instruction set (VEX-256/EVEX) */
                        uint32_t VPCLMULQDQ : 1;
                        /** @brief AVX-512 Vector Neural Network Instructions */
                        uint32_t AVX512VNNI : 1;
                        /** @brief AVX-512 Bit Algorithms Instructions */
                        uint32_t AVX512BITALG : 1;
                        /** @brief IA32_TME related MSRs  */
                        uint32_t TME : 1;
                        /** @brief AVX-512 Vector Population Count Double and Quad-word */
                        uint32_t AVX512VPOPCNTDQ : 1;
                        /** @brief Reserved */
                        uint32_t Reserved0 : 1;
                        /** @brief 5-level paging (57 address bits) */
                        uint32_t LA57 : 1;
                        /** @brief The value of userspace MPX Address-Width Adjust used by the BNDLDX and BNDSTX Intel MPX instructions in 64-bit mode */
                        uint32_t MAWAU : 5;
                        /** @brief Read Processor ID and IA32_TSC_AUX  */
                        uint32_t RDPID : 1;
                        /** @brief Key Locker */
                        uint32_t KL : 1;
                        /** @brief BUS_LOCK_DETECT */
                        uint32_t BusLockDetect : 1;
                        /** @brief Cache line demote */
                        uint32_t CLDEMOTE : 1;
                        /** @brief Reserved */
                        uint32_t Reserved1 : 1;
                        /** @brief MOVDIRI */
                        uint32_t MOVDIRI : 1;
                        /** @brief MOVDIR64B */
                        uint32_t MOVDIR64B : 1;
                        /** @brief SGX Launch Configuration */
                        uint32_t SGX_LC : 1;
                        /** @brief Protection Keys for Supervisor-mode pages */
                        uint32_t PKS : 1;
                    };
                    cpuid_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        /** @brief Reserved */
                        uint32_t Reserved0 : 2;
                        /** @brief AVX-512 4-register Neural Network Instructions */
                        uint32_t AVX512_4VNNIW : 1;
                        /** @brief AVX-512 4-register Multiply Accumulation Single Precision */
                        uint32_t AVX512_4FMAPS : 1;
                        /** @brief Fast Short REP MOVSB/STOSB */
                        uint32_t FSRM : 1;
                        /** @brief User Inter-Processor Interrupts */
                        uint32_t UINTR : 1;
                        /** @brief Reserved */
                        uint32_t Reserved1 : 2;
                        /** @brief AVX-512 VP2INTERSECT Doubleword and Quadword Instructions */
                        uint32_t AVX512_VP2INTERSECT : 1;
                        /** @brief Special Register Buffer Data Sampling Mitigations */
                        uint32_t SRBDS_CTRL : 1;
                        /** @brief VERW instruction clears CPU buffers */
                        uint32_t MC_CLEAR : 1;
                        /** @brief All TSX transactions are aborted */
                        uint32_t TSX_FORCE_ABORT : 1;
                        /** @brief Reserved */
                        uint32_t Reserved2 : 1;
                        /** @brief TSX_FORCE_ABORT MSR is available */
                        uint32_t TsxForceAbortMsr : 1;
                        /** @brief SERIALIZE */
                        uint32_t SERIALIZE : 1;
                        /** @brief Mixture of CPU types in processor topology */
                        uint32_t HYBRID : 1;
                        /** @brief TSXLDTRK */
                        uint32_t TSXLDTRK : 1;
                        /** @brief Reserved */
                        uint32_t Reserved3 : 1;
                        /** @brief Platform configuration for Memory Encryption Technologies Instrctuions */
                        uint32_t PCONFIG : 1;
                        /** @brief Architectural Last Branch Records */
                        uint32_t LBR : 1;
                        /** @brief Control flow enforcement (CET) indirect branch tracking */
                        uint32_t CET_IBT : 1;
                        /** @brief Reserved */
                        uint32_t Reserved4 : 1;
                        /** @brief Tile computation on bfloat16 numbers */
                        uint32_t AMX_BF16 : 1;
                        /** @brief AVX512-FP16 half-precision floating-point instructions */
                        uint32_t AVX512_FP16 : 1;
                        /** @brief Tile architecture */
                        uint32_t AMX_TILE : 1;
                        /** @brief Tile computation on 8-bit integers */
                        uint32_t AMX_INT8 : 1;
                        /** @brief Speculation Control, part of Indirect Branch Control (IBC):
                                   Indirect Branch Restricted Speculation (IBRS) and
                                   Indirect Branch Prediction Barrier (IBPB) */
                        uint32_t SPEC_CTRL : 1;
                        /** @brief Single Thread Indirect Branch Predictor, part of IBC */
                        uint32_t STIBP : 1;
                        /** @brief IA32_FLUSH_CMD MSR */
                        uint32_t L1D_FLUSH : 1;
                        /** @brief IA32_ARCH_CAPABILITIES (lists speculative side channel mitigations */
                        uint32_t ArchCapabilities : 1;
                        /** @brief IA32_CORE_CAPABILITIES MSR (lists model-specific core capabilities) */
                        uint32_t CoreCapabilities : 1;
                        /** @brief Speculative Store Bypass Disable, as mitigation for Speculative Store Bypass (IA32_SPEC_CTRL) */
                        uint32_t SSBD : 1;
                    };
                    cpuid_t raw;
                } EDX;
            };

            /** @brief Extended feature flags enumeration */
            struct CPUID0x00000007_1
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
#endif // a64 || a32
                }

                union
                {
                    struct
                    {
                        uint32_t Reserved0 : 3;
                        /** @brief RAO-INT */
                        uint32_t RAO_INT : 1;
                        /** @brief AVX Vector Neural Network Instructions (XNNI) (VEX encoded) */
                        uint32_t AVX_VNNI : 1;
                        /** @brief AVX-512 instructions for bfloat16 numbers */
                        uint32_t AVX512_BF16 : 1;
                        /** @brief Reserved */
                        uint32_t Reserved1 : 1;
                        /** @brief CMPccXADD */
                        uint32_t CMPCCXADD : 1;
                        /** @brief Architectural Performance Monitoring Extended Leaf (EAX=23h) */
                        uint32_t ARCHPERFMONEXT : 1;
                        /** @brief Reserved */
                        uint32_t Reserved2 : 1;
                        /** @brief Fast zero-length MOVSB */
                        uint32_t FAST_ZERO_REP_MOVSB : 1;
                        /** @brief Fast zero-length STOSB */
                        uint32_t FAST_SHORT_REP_STOSB : 1;
                        /** @brief Fast zero-length CMPSB and SCASB */
                        uint32_t FAST_SHORT_REP_CMPSB_SCASB : 1;
                        /** @brief Reserved */
                        uint32_t Reserved3 : 4;
                        /** @brief Flexible Return and Event Delivery */
                        uint32_t FRED : 1;
                        /** @brief LKGS Instruction */
                        uint32_t LKGS : 1;
                        /** @brief WRMSRNS instruction */
                        uint32_t WRMSRNS : 1;
                        /** @brief Reserved */
                        uint32_t Reserved4 : 1;
                        /** @brief AMX instructions for FP16 numbers */
                        uint32_t AMX_FP16 : 1;
                        /** @brief HRESET instruction, IA32_HRESET_ENABLE MSR, and Processor History Reset Leaf (EAX=20h) */
                        uint32_t HRESET : 1;
                        /** @brief AVX IFMA instructions */
                        uint32_t AVX_IFMA : 1;
                        /** @brief Reserved */
                        uint32_t Reserved5 : 2;
                        /** @brief Linear Address Masking */
                        uint32_t LAM : 1;
                        /** @brief RDMSRLIST and WRMSRLIST instructions, and the IA32_BARRIER MSR */
                        uint32_t MSRLIST : 1;
                    };
                    cpuid_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        /** @brief IA32_PPIN and IA32_PPIN_CTL MSRs */
                        uint32_t PPIN : 1;
                        /** @brief Reserved */
                        uint32_t Reserved : 31;
                    };
                    cpuid_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        /** @brief Reserved */
                        uint32_t Reserved : 32;
                    };
                    cpuid_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        /** @brief Reserved */
                        uint32_t Reserved0 : 4;
                        /** @brief AVX VNNI INT8 instructions */
                        uint32_t AVX_VNNI_INT8 : 1;
                        /** @brief AVX NE CONVERT instructions */
                        uint32_t AVX_NE_CONVERT : 1;
                        /** @brief Reserved */
                        uint32_t Reserved1 : 8;
                        /** @brief PREFETCHIT0 and PREFETCHIT1 instructions */
                        uint32_t PREFETCHIT : 1;
                        /** @brief Reserved */
                        uint32_t Reserved2 : 17;
                    };
                    cpuid_t raw;
                } EDX;
            };

            /** @brief Performance monitors */
            struct CPUID0x0000000A
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
#endif // a64 || a32
                }

                union
                {
                    struct
                    {
                        uint32_t VersionID : 8;
                        uint32_t NumberCounters : 8;
                        uint32_t BitWidthOfCounters : 8;
                        uint32_t LengthOfEBXBitVector : 8;
                    };
                    cpuid_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t CoreCycles : 1;
                        uint32_t InstructionsRetired : 1;
                        uint32_t ReferenceCycles : 1;
                        uint32_t CacheReferences : 1;
                        uint32_t CacheMisses : 1;
                        uint32_t BranchInstructionsRetired : 1;
                        uint32_t BranchMissesRetired : 1;
                        uint32_t Reserved : 25;
                    };
                    cpuid_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t FixedFunctionCounters : 5;
                        uint32_t CounterWidth : 8;
                        uint32_t Reserved : 19;
                    };
                    cpuid_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    cpuid_t raw;
                } EDX;
            };

            /** @brief Get CPU frequency information */
            struct CPUID0x00000015
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
#endif // a64 || a32
                }

                union
                {
                    struct
                    {
                        uint32_t VersionID : 8;
                        uint32_t NumberCounters : 8;
                        uint32_t BitWidthOfCounters : 8;
                        uint32_t LengthOfEBXBitVector : 8;
                    };
                    cpuid_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t CoreCycles : 1;
                        uint32_t InstructionsRetired : 1;
                        uint32_t ReferenceCycles : 1;
                        uint32_t CacheReferences : 1;
                        uint32_t CacheMisses : 1;
                        uint32_t BranchInstructionsRetired : 1;
                        uint32_t BranchMissesRetired : 1;
                        uint32_t Reserved : 25;
                    };
                    cpuid_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t FixedFunctionCounters : 5;
                        uint32_t CounterWidth : 8;
                        uint32_t Reserved : 19;
                    };
                    cpuid_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    cpuid_t raw;
                } EDX;
            };

            /** @brief Get CPU frequency information */
            struct CPUID0x00000016
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
#endif // a64 || a32
                }

                union
                {
                    struct
                    {
                        /**
                         * @brief Denominator of the TSC frequency
                         *
                         * @note TSC frequency = core crystal clock frequency * EBX/EAX
                         */
                        uint32_t Denominator : 31;
                    };
                    cpuid_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        /**
                         * @brief Numerator of the TSC frequency
                         *
                         * @note TSC frequency = core crystal clock frequency * EBX/EAX
                         */
                        uint32_t Numerator : 31;
                    };
                    cpuid_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        /** @brief Core crystal clock frequency in Hz */
                        uint32_t CoreCrystalClock : 31;
                    };
                    cpuid_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        /** @brief Reserved */
                        uint32_t Reserved : 31;
                    };
                    cpuid_t raw;
                } EDX;
            };

            /** @brief Extended CPU information */
            struct CPUID0x80000000
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
#endif // a64 || a32
                }

                union
                {
                    struct
                    {
                        uint32_t HighestExtendedFunctionSupported : 32;
                    };
                    cpuid_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    cpuid_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    cpuid_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    cpuid_t raw;
                } EDX;
                cpuid_t raw;
            };

            /** @brief Extended CPU information */
            struct CPUID0x80000001
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
#endif // a64 || a32
                }

                union
                {
                    struct
                    {
                        uint32_t Unknown : 32;
                    };
                    cpuid_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    cpuid_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t LAHF_SAHF : 1;
                        uint32_t Reserved : 31;
                    };
                    cpuid_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t Reserved0 : 11;
                        uint32_t SYSCALL : 1;
                        uint32_t Reserved1 : 8;
                        uint32_t ExecuteDisable : 1;
                        uint32_t Reserved2 : 8;
                        uint32_t EMT64T : 1;
                        uint32_t Reserved3 : 2;
                    };
                    cpuid_t raw;
                } EDX;
                cpuid_t raw;
            };

            /** @brief CPU brand string */
            struct CPUID0x80000002
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
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
                cpuid_t raw;
            };

            /** @brief CPU brand string */
            struct CPUID0x80000003
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
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
                cpuid_t raw;
            };

            /** @brief CPU brand string */
            struct CPUID0x80000004
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
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
                cpuid_t raw;
            };

            /** @brief CPU cache line information */
            struct CPUID0x80000006
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
#endif // a64 || a32
                }

                union
                {
                    struct
                    {
                        uint32_t InstructionCount : 12;
                        uint32_t InstructionAssociativity : 4;
                        uint32_t DataCount : 12;
                        uint32_t DataAssociativity : 4;
                    };
                    cpuid_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t InstructionCount : 12;
                        uint32_t InstructionAssociativity : 4;
                        uint32_t DataCount : 12;
                        uint32_t DataAssociativity : 4;
                    };
                    cpuid_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t LineSize : 8;
                        uint32_t LinePerTag : 4;
                        uint32_t Associativity : 4;
                        uint32_t CacheSize : 16;
                    };
                    cpuid_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    cpuid_t raw;
                } EDX;
                cpuid_t raw;
            };

            /** @brief Virtual and physical memory size */
            struct CPUID0x80000008
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
#endif // a64 || a32
                }

                union
                {
                    struct
                    {
                        uint32_t PhysicalAddressBits : 8;
                        uint32_t LinearAddressBits : 8;
                        uint32_t Reserved : 16;
                    };
                    cpuid_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    cpuid_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    cpuid_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    cpuid_t raw;
                } EDX;
                cpuid_t raw;
            };

            /** @brief Secure virtual machine parameters */
            struct CPUID0x8000000A
            {
                void Get()
                {
#if defined(a64) || defined(a32)
                    asmv("cpuid"
                         : "=a"(EAX.raw), "=b"(EBX.raw), "=c"(ECX.raw), "=d"(EDX.raw)
                         : "a"(0x1));
#endif // a64 || a32
                }

                union
                {
                    struct
                    {
                        uint32_t SVMRevision : 8;
                        uint32_t Reserved : 24;
                    };
                    cpuid_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    cpuid_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    cpuid_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    cpuid_t raw;
                } EDX;
                cpuid_t raw;
            };
        }
    }
}

#endif // !__FENNIX_KERNEL_CPU_x86_CPUID_INTEL_H__
