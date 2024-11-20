#ifndef __FENNIX_KERNEL_CPU_x64_CPUID_INTEL_H__
#define __FENNIX_KERNEL_CPU_x64_CPUID_INTEL_H__

#include <types.h>

namespace CPU
{
    namespace x64
    {
        /** @brief EXPERIMENTAL IMPLEMENTATION */
        namespace Intel
        {
            /** @brief Basic CPU information */
            struct CPUID0x0
            {
                union
                {
                    struct
                    {
                        uint64_t HighestFunctionSupported : 32;
                    };
                    uint64_t raw;
                } EAX;
                union
                {
                    struct
                    {
                        char rbx[4];
                    };
                    uint64_t raw;
                } EBX;
                union
                {
                    struct
                    {
                        char rcx[4];
                    };
                    uint64_t raw;
                } ECX;
                union
                {
                    struct
                    {
                        char rdx[4];
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Additional CPU information */
            struct CPUID0x1
            {
                union
                {
                    struct
                    {
                        uint64_t SteppingID : 4;
                        uint64_t ModelID : 4;
                        uint64_t FamilyID : 4;
                        uint64_t Type : 2;
                        uint64_t Reserved0 : 2;
                        uint64_t ExtendedModel : 4;
                        uint64_t ExtendedFamily : 8;
                        uint64_t Reserved1 : 4;
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint64_t BrandIndex : 8;
                        uint64_t CLFLUSHLineSize : 8;
                        uint64_t LogicalProcessorsPerPackage : 8;
                        uint64_t LocalAPICID : 8;
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint64_t SSE3 : 1;
                        uint64_t PCLMULQDQ : 1;
                        uint64_t DTES64 : 1;
                        uint64_t MONITOR : 1;
                        uint64_t DS_CPL : 1;
                        uint64_t VMX : 1;
                        uint64_t SMX : 1;
                        uint64_t EIST : 1;
                        uint64_t TM2 : 1;
                        uint64_t SSSE3 : 1;
                        uint64_t CNXT_ID : 1;
                        uint64_t Reserved0 : 1;
                        uint64_t FMA : 1;
                        uint64_t CMPXCHG16B : 1;
                        uint64_t xTPRUpdateControl : 1;
                        uint64_t PDCM : 1;
                        uint64_t Reserved1 : 1;
                        uint64_t PCID : 1;
                        uint64_t DCA : 1;
                        uint64_t SSE4_1 : 1;
                        uint64_t SSE4_2 : 1;
                        uint64_t x2APIC : 1;
                        uint64_t MOVBE : 1;
                        uint64_t POPCNT : 1;
                        uint64_t TSCDeadline : 1;
                        uint64_t AES : 1;
                        uint64_t XSAVE : 1;
                        uint64_t OSXSAVE : 1;
                        uint64_t AVX : 1;
                        uint64_t F16C : 1;
                        uint64_t RDRAND : 1;
                        uint64_t Reserved2 : 1;
                        uint64_t Hypervisor : 1;
                    };
                    uint64_t raw;
                } ECX;

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
                        uint64_t CX8 : 1;
                        uint64_t APIC : 1;
                        uint64_t Reserved0 : 1;
                        uint64_t SEP : 1;
                        uint64_t MTRR : 1;
                        uint64_t PGE : 1;
                        uint64_t MCA : 1;
                        uint64_t CMOV : 1;
                        uint64_t PAT : 1;
                        uint64_t PSE36 : 1;
                        uint64_t PSN : 1;
                        uint64_t CLFSH : 1;
                        uint64_t Reserved1 : 1;
                        uint64_t DS : 1;
                        uint64_t ACPI : 1;
                        uint64_t MMX : 1;
                        uint64_t FXSR : 1;
                        uint64_t SSE : 1;
                        uint64_t SSE2 : 1;
                        uint64_t SS : 1;
                        uint64_t HTT : 1;
                        uint64_t TM : 1;
                        uint64_t Reserved2 : 1;
                        uint64_t PBE : 1;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief CPU cache and TLB */
            struct CPUID0x2
            {
                union
                {
                    struct
                    {
                        uint64_t CacheLineSize : 8;
                        uint64_t CacheLinesPerTag : 8;
                        uint64_t Associativity : 8;
                        uint64_t CacheSize : 8;
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint64_t CacheLineSize : 8;
                        uint64_t CacheLinesPerTag : 8;
                        uint64_t Associativity : 8;
                        uint64_t CacheSize : 8;
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint64_t CacheLineSize : 8;
                        uint64_t CacheLinesPerTag : 8;
                        uint64_t Associativity : 8;
                        uint64_t CacheSize : 8;
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint64_t CacheLineSize : 8;
                        uint64_t CacheLinesPerTag : 8;
                        uint64_t Associativity : 8;
                        uint64_t CacheSize : 8;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief CPU serial number */
            struct CPUID0x3
            {
                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint64_t ProcessorSerialNumber : 32;
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint64_t ProcessorSerialNumber : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Cache information */
            struct CPUID0x4_1
            {
                union
                {
                    struct
                    {
                        uint64_t Type : 5;
                        uint64_t Level : 3;
                        uint64_t SelfInitializing : 1;
                        uint64_t FullyAssociative : 1;
                        uint64_t Reserved : 4;
                        uint64_t MaxAddressableIdsForLogicalProcessors : 12;
                        uint64_t CoresPerPackage : 6;
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint64_t SystemCoherencyLineSize : 12;
                        uint64_t PhysicalLinePartitions : 10;
                        uint64_t WaysOfAssociativity : 10;
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief MONITOR information */
            struct CPUID0x5
            {
                union
                {
                    struct
                    {
                        uint64_t SmallestMonitorLineSize : 16;
                        uint64_t Reserved : 16;
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint64_t LargestMonitorLineSize : 16;
                        uint64_t Reserved : 16;
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint64_t MWAITEnumerationSupported : 1;
                        uint64_t InterruptsAsBreakEvent : 1;
                        uint64_t Reserved : 30;
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint64_t C0 : 4;
                        uint64_t C1 : 4;
                        uint64_t C2 : 4;
                        uint64_t C3 : 4;
                        uint64_t C4 : 4;
                        uint64_t Reserved : 12;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Thermal and power management information */
            struct CPUID0x6
            {
                union
                {
                    struct
                    {
                        uint64_t SensorSupported : 1;
                        uint64_t Reserved : 31;
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint64_t InterruptThreshold : 4;
                        uint64_t Reserved : 26;
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint64_t ACNT_MCNT : 1;
                        uint64_t Reserved : 31;
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Extended feature flags enumeration */
            struct CPUID0x7_0
            {
                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        /** @brief Access to base of fs and gs */
                        uint64_t FSGSBase : 1;
                        /** @brief IA32_TSC_ADJUST MSR */
                        uint64_t IA32TSCAdjust : 1;
                        /** @brief Software Guard Extensions */
                        uint64_t SGX : 1;
                        /** @brief Bit Manipulation Instruction Set 1 */
                        uint64_t BMI1 : 1;
                        /** @brief TSX Hardware Lock Elision */
                        uint64_t HLE : 1;
                        /** @brief Advanced Vector Extensions 2 */
                        uint64_t AVX2 : 1;
                        /** @brief FDP_EXCPTN_ONLY */
                        uint64_t FDPExcptonOnly : 1;
                        /** @brief Supervisor Mode Execution Protection */
                        uint64_t SMEP : 1;
                        /** @brief Bit Manipulation Instruction Set 2 */
                        uint64_t BMI2 : 1;
                        /** @brief Enhanced REP MOVSB/STOSB */
                        uint64_t ERMS : 1;
                        /** @brief INVPCID */
                        uint64_t INVPCID : 1;
                        /** @brief RTM */
                        uint64_t RTM : 1;
                        /** @brief Intel Resource Director Monitoring */
                        uint64_t RDT_M : 1;
                        /** @brief Deprecates FPU CS and DS values */
                        uint64_t DeprecatesFPU : 1;
                        /** @brief Intel Memory Protection Extensions */
                        uint64_t MPX : 1;
                        /** @brief Intel Resource Director Allocation */
                        uint64_t RDT_A : 1;
                        /** @brief AVX-512 Foundation */
                        uint64_t AVX512F : 1;
                        /** @brief AVX-512 Doubleword and Quadword Instructions */
                        uint64_t AVX512DQ : 1;
                        /** @brief RDSEED */
                        uint64_t RDSEED : 1;
                        /** @brief Intel Multi-Precision Add-Carry Instruction Extensions */
                        uint64_t ADX : 1;
                        /** @brief Supervisor Mode Access Prevention */
                        uint64_t SMAP : 1;
                        /** @brief AVX-512 Integer Fused Multiply-Add Instructions */
                        uint64_t AVX512IFMA : 1;
                        /** @brief Reserved */
                        uint64_t Reserved : 1;
                        /** @brief CLFLUSHOPT */
                        uint64_t CLFLUSHOPT : 1;
                        /** @brief CLWB */
                        uint64_t CLWB : 1;
                        /** @brief Intel Processor Trace */
                        uint64_t IntelProcessorTrace : 1;
                        /** @brief AVX-512 Prefetch Instructions */
                        uint64_t AVX512PF : 1;
                        /** @brief AVX-512 Exponential and Reciprocal Instructions */
                        uint64_t AVX512ER : 1;
                        /** @brief AVX-512 Conflict Detection Instructions */
                        uint64_t AVX512CD : 1;
                        /** @brief SHA Extensions */
                        uint64_t SHA : 1;
                        /** @brief AVX-512 Byte and Word Instructions */
                        uint64_t AVX512BW : 1;
                        /** @brief AVX-512 Vector Length Extensions */
                        uint64_t AVX512VL : 1;
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        /** @brief PREFETCHWT1 */
                        uint64_t PREFETCHWT1 : 1;
                        /** @brief AVX-512 Vector Bit Manipulation Instructions */
                        uint64_t AVX512VBMI : 1;
                        /** @brief User Mode Instruction Prevention */
                        uint64_t UMIP : 1;
                        /** @brief Memory Protection Keys for User-mode pages */
                        uint64_t PKU : 1;
                        /** @brief PKU enabled by OS */
                        uint64_t OSPKE : 1;
                        /** @brief Timed pause and user-level monitor/wait */
                        uint64_t WaitPKG : 1;
                        /** @brief AVX-512 Vector Bit Manipulation Instructions 2 */
                        uint64_t AVX512VBMI2 : 1;
                        /** @brief Control flow enforcement (CET) shadow stack */
                        uint64_t CET_SS : 1;
                        /** @brief Galois Field instructions */
                        uint64_t GFNI : 1;
                        /** @brief Vector AES instruction set (VEX-256/EVEX) */
                        uint64_t VAES : 1;
                        /** @brief CLMUL instruction set (VEX-256/EVEX) */
                        uint64_t VPCLMULQDQ : 1;
                        /** @brief AVX-512 Vector Neural Network Instructions */
                        uint64_t AVX512VNNI : 1;
                        /** @brief AVX-512 Bit Algorithms Instructions */
                        uint64_t AVX512BITALG : 1;
                        /** @brief IA32_TME related MSRs  */
                        uint64_t TME : 1;
                        /** @brief AVX-512 Vector Population Count Double and Quad-word */
                        uint64_t AVX512VPOPCNTDQ : 1;
                        /** @brief Reserved */
                        uint64_t Reserved0 : 1;
                        /** @brief 5-level paging (57 address bits) */
                        uint64_t LA57 : 1;
                        /** @brief The value of userspace MPX Address-Width Adjust used by the BNDLDX and BNDSTX Intel MPX instructions in 64-bit mode */
                        uint64_t MAWAU : 5;
                        /** @brief Read Processor ID and IA32_TSC_AUX  */
                        uint64_t RDPID : 1;
                        /** @brief Key Locker */
                        uint64_t KL : 1;
                        /** @brief BUS_LOCK_DETECT */
                        uint64_t BusLockDetect : 1;
                        /** @brief Cache line demote */
                        uint64_t CLDEMOTE : 1;
                        /** @brief Reserved */
                        uint64_t Reserved1 : 1;
                        /** @brief MOVDIRI */
                        uint64_t MOVDIRI : 1;
                        /** @brief MOVDIR64B */
                        uint64_t MOVDIR64B : 1;
                        /** @brief SGX Launch Configuration */
                        uint64_t SGX_LC : 1;
                        /** @brief Protection Keys for Supervisor-mode pages */
                        uint64_t PKS : 1;
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        /** @brief Reserved */
                        uint64_t Reserved0 : 2;
                        /** @brief AVX-512 4-register Neural Network Instructions */
                        uint64_t AVX512_4VNNIW : 1;
                        /** @brief AVX-512 4-register Multiply Accumulation Single Precision */
                        uint64_t AVX512_4FMAPS : 1;
                        /** @brief Fast Short REP MOVSB/STOSB */
                        uint64_t FSRM : 1;
                        /** @brief User Inter-Processor Interrupts */
                        uint64_t UINTR : 1;
                        /** @brief Reserved */
                        uint64_t Reserved1 : 2;
                        /** @brief AVX-512 VP2INTERSECT Doubleword and Quadword Instructions */
                        uint64_t AVX512_VP2INTERSECT : 1;
                        /** @brief Special Register Buffer Data Sampling Mitigations */
                        uint64_t SRBDS_CTRL : 1;
                        /** @brief VERW instruction clears CPU buffers */
                        uint64_t MC_CLEAR : 1;
                        /** @brief All TSX transactions are aborted */
                        uint64_t TSX_FORCE_ABORT : 1;
                        /** @brief Reserved */
                        uint64_t Reserved2 : 1;
                        /** @brief TSX_FORCE_ABORT MSR is available */
                        uint64_t TsxForceAbortMsr : 1;
                        /** @brief SERIALIZE */
                        uint64_t SERIALIZE : 1;
                        /** @brief Mixture of CPU types in processor topology */
                        uint64_t HYBRID : 1;
                        /** @brief TSXLDTRK */
                        uint64_t TSXLDTRK : 1;
                        /** @brief Reserved */
                        uint64_t Reserved3 : 1;
                        /** @brief Platform configuration for Memory Encryption Technologies Instrctuions */
                        uint64_t PCONFIG : 1;
                        /** @brief Architectural Last Branch Records */
                        uint64_t LBR : 1;
                        /** @brief Control flow enforcement (CET) indirect branch tracking */
                        uint64_t CET_IBT : 1;
                        /** @brief Reserved */
                        uint64_t Reserved4 : 1;
                        /** @brief Tile computation on bfloat16 numbers */
                        uint64_t AMX_BF16 : 1;
                        /** @brief AVX512-FP16 half-precision floating-point instructions */
                        uint64_t AVX512_FP16 : 1;
                        /** @brief Tile architecture */
                        uint64_t AMX_TILE : 1;
                        /** @brief Tile computation on 8-bit integers */
                        uint64_t AMX_INT8 : 1;
                        /** @brief Speculation Control, part of Indirect Branch Control (IBC):
                                   Indirect Branch Restricted Speculation (IBRS) and
                                   Indirect Branch Prediction Barrier (IBPB) */
                        uint64_t SPEC_CTRL : 1;
                        /** @brief Single Thread Indirect Branch Predictor, part of IBC */
                        uint64_t STIBP : 1;
                        /** @brief IA32_FLUSH_CMD MSR */
                        uint64_t L1D_FLUSH : 1;
                        /** @brief IA32_ARCH_CAPABILITIES (lists speculative side channel mitigations */
                        uint64_t ArchCapabilities : 1;
                        /** @brief IA32_CORE_CAPABILITIES MSR (lists model-specific core capabilities) */
                        uint64_t CoreCapabilities : 1;
                        /** @brief Speculative Store Bypass Disable, as mitigation for Speculative Store Bypass (IA32_SPEC_CTRL) */
                        uint64_t SSBD : 1;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Extended feature flags enumeration */
            struct CPUID0x7_1
            {
                union
                {
                    struct
                    {
                        uint64_t Reserved0 : 3;
                        /** @brief RAO-INT */
                        uint64_t RAO_INT : 1;
                        /** @brief AVX Vector Neural Network Instructions (XNNI) (VEX encoded) */
                        uint64_t AVX_VNNI : 1;
                        /** @brief AVX-512 instructions for bfloat16 numbers */
                        uint64_t AVX512_BF16 : 1;
                        /** @brief Reserved */
                        uint64_t Reserved1 : 1;
                        /** @brief CMPccXADD */
                        uint64_t CMPCCXADD : 1;
                        /** @brief Architectural Performance Monitoring Extended Leaf (EAX=23h) */
                        uint64_t ARCHPERFMONEXT : 1;
                        /** @brief Reserved */
                        uint64_t Reserved2 : 1;
                        /** @brief Fast zero-length MOVSB */
                        uint64_t FAST_ZERO_REP_MOVSB : 1;
                        /** @brief Fast zero-length STOSB */
                        uint64_t FAST_SHORT_REP_STOSB : 1;
                        /** @brief Fast zero-length CMPSB and SCASB */
                        uint64_t FAST_SHORT_REP_CMPSB_SCASB : 1;
                        /** @brief Reserved */
                        uint64_t Reserved3 : 4;
                        /** @brief Flexible Return and Event Delivery */
                        uint64_t FRED : 1;
                        /** @brief LKGS Instruction */
                        uint64_t LKGS : 1;
                        /** @brief WRMSRNS instruction */
                        uint64_t WRMSRNS : 1;
                        /** @brief Reserved */
                        uint64_t Reserved4 : 1;
                        /** @brief AMX instructions for FP16 numbers */
                        uint64_t AMX_FP16 : 1;
                        /** @brief HRESET instruction, IA32_HRESET_ENABLE MSR, and Processor History Reset Leaf (EAX=20h) */
                        uint64_t HRESET : 1;
                        /** @brief AVX IFMA instructions */
                        uint64_t AVX_IFMA : 1;
                        /** @brief Reserved */
                        uint64_t Reserved5 : 2;
                        /** @brief Linear Address Masking */
                        uint64_t LAM : 1;
                        /** @brief RDMSRLIST and WRMSRLIST instructions, and the IA32_BARRIER MSR */
                        uint64_t MSRLIST : 1;
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        /** @brief IA32_PPIN and IA32_PPIN_CTL MSRs */
                        uint64_t PPIN : 1;
                        /** @brief Reserved */
                        uint64_t Reserved : 31;
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        /** @brief Reserved */
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        /** @brief Reserved */
                        uint64_t Reserved0 : 4;
                        /** @brief AVX VNNI INT8 instructions */
                        uint64_t AVX_VNNI_INT8 : 1;
                        /** @brief AVX NE CONVERT instructions */
                        uint64_t AVX_NE_CONVERT : 1;
                        /** @brief Reserved */
                        uint64_t Reserved1 : 8;
                        /** @brief PREFETCHIT0 and PREFETCHIT1 instructions */
                        uint64_t PREFETCHIT : 1;
                        /** @brief Reserved */
                        uint64_t Reserved2 : 17;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Performance monitors */
            struct CPUID0xA
            {
                union
                {
                    struct
                    {
                        uint64_t VersionID : 8;
                        uint64_t NumberCounters : 8;
                        uint64_t BitWidthOfCounters : 8;
                        uint64_t LengthOfEBXBitVector : 8;
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint64_t CoreCycles : 1;
                        uint64_t InstructionsRetired : 1;
                        uint64_t ReferenceCycles : 1;
                        uint64_t CacheReferences : 1;
                        uint64_t CacheMisses : 1;
                        uint64_t BranchInstructionsRetired : 1;
                        uint64_t BranchMissesRetired : 1;
                        uint64_t Reserved : 25;
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint64_t FixedFunctionCounters : 5;
                        uint64_t CounterWidth : 8;
                        uint64_t Reserved : 19;
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Get CPU frequency information */
            struct CPUID0x15
            {
                union
                {
                    struct
                    {
                        uint64_t VersionID : 8;
                        uint64_t NumberCounters : 8;
                        uint64_t BitWidthOfCounters : 8;
                        uint64_t LengthOfEBXBitVector : 8;
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint64_t CoreCycles : 1;
                        uint64_t InstructionsRetired : 1;
                        uint64_t ReferenceCycles : 1;
                        uint64_t CacheReferences : 1;
                        uint64_t CacheMisses : 1;
                        uint64_t BranchInstructionsRetired : 1;
                        uint64_t BranchMissesRetired : 1;
                        uint64_t Reserved : 25;
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint64_t FixedFunctionCounters : 5;
                        uint64_t CounterWidth : 8;
                        uint64_t Reserved : 19;
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Get CPU frequency information */
            struct CPUID0x16
            {
                union
                {
                    struct
                    {
                        /**
                         * @brief Denominator of the TSC frequency
                         *
                         * @note TSC frequency = core crystal clock frequency * EBX/EAX
                         */
                        uint64_t Denominator : 31;
                    };
                    uint64_t raw;
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
                        uint64_t Numerator : 31;
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        /** @brief Core crystal clock frequency in Hz */
                        uint64_t CoreCrystalClock : 31;
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        /** @brief Reserved */
                        uint64_t Reserved : 31;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Get CPU hypervisor information */
            struct CPUID0x40000000
            {
                union
                {
                    struct
                    {
                        /**
                         * @brief Maximum input value for hypervisor CPUID information.
                         * @note Can be from 0x40000001 to 0x400000FF
                         */
                        uint64_t MaximumInputValue : 32;
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        /** @brief Hypervisor vendor signature */
                        char Hypervisor[4];
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        /** @brief Hypervisor vendor signature */
                        char Hypervisor[4];
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        /** @brief Hypervisor vendor signature */
                        char Hypervisor[4];
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief Extended CPU information */
            struct CPUID0x80000000
            {
                union
                {
                    struct
                    {
                        uint64_t HighestExtendedFunctionSupported : 32;
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EDX;
                uint64_t raw;
            };

            /** @brief Extended CPU information */
            struct CPUID0x80000001
            {
                union
                {
                    struct
                    {
                        uint64_t Unknown : 32;
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint64_t LAHF_SAHF : 1;
                        uint64_t Reserved : 31;
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint64_t Reserved0 : 11;
                        uint64_t SYSCALL : 1;
                        uint64_t Reserved1 : 8;
                        uint64_t ExecuteDisable : 1;
                        uint64_t Reserved2 : 8;
                        uint64_t EMT64T : 1;
                        uint64_t Reserved3 : 2;
                    };
                    uint64_t raw;
                } EDX;
                uint64_t raw;
            };

            /** @brief CPU brand string */
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
                uint64_t raw;
            };

            /** @brief CPU brand string */
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
                uint64_t raw;
            };

            /** @brief CPU brand string */
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
                uint64_t raw;
            };

            /** @brief CPU cache line information */
            struct CPUID0x80000006
            {
                union
                {
                    struct
                    {
                        uint64_t InstructionCount : 12;
                        uint64_t InstructionAssociativity : 4;
                        uint64_t DataCount : 12;
                        uint64_t DataAssociativity : 4;
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint64_t InstructionCount : 12;
                        uint64_t InstructionAssociativity : 4;
                        uint64_t DataCount : 12;
                        uint64_t DataAssociativity : 4;
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint64_t LineSize : 8;
                        uint64_t LinePerTag : 4;
                        uint64_t Associativity : 4;
                        uint64_t CacheSize : 16;
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EDX;
                uint64_t raw;
            };

            /** @brief Virtual and physical memory size */
            struct CPUID0x80000008
            {
                union
                {
                    struct
                    {
                        uint64_t PhysicalAddressBits : 8;
                        uint64_t LinearAddressBits : 8;
                        uint64_t Reserved : 16;
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EDX;
                uint64_t raw;
            };

            /** @brief Secure virtual machine parameters */
            struct CPUID0x8000000A
            {
                union
                {
                    struct
                    {
                        uint64_t SVMRevision : 8;
                        uint64_t Reserved : 24;
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint64_t Reserved : 32;
                    };
                    uint64_t raw;
                } EDX;
                uint64_t raw;
            };
        }
    }
}

#endif // !__FENNIX_KERNEL_CPU_x64_CPUID_INTEL_H__
