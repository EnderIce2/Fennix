#ifndef __FENNIX_KERNEL_CPU_x32_CPUID_INTEL_H__
#define __FENNIX_KERNEL_CPU_x32_CPUID_INTEL_H__

#include <types.h>

namespace CPU
{
    namespace x32
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
                        uint32_t HighestFunctionSupported : 32;
                    };
                    uint32_t raw;
                } EAX;
                union
                {
                    struct
                    {
                        char rbx[4];
                    };
                    uint32_t raw;
                } EBX;
                union
                {
                    struct
                    {
                        char rcx[4];
                    };
                    uint32_t raw;
                } ECX;
                union
                {
                    struct
                    {
                        char rdx[4];
                    };
                    uint32_t raw;
                } EDX;
            };

            /** @brief Additional CPU information */
            struct CPUID0x1
            {
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
                    uint32_t raw;
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
                    uint32_t raw;
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
                    uint32_t raw;
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
                    uint32_t raw;
                } EDX;
            };

            /** @brief CPU cache and TLB */
            struct CPUID0x2
            {
                union
                {
                    struct
                    {
                        uint32_t CacheLineSize : 8;
                        uint32_t CacheLinesPerTag : 8;
                        uint32_t Associativity : 8;
                        uint32_t CacheSize : 8;
                    };
                    uint32_t raw;
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
                    uint32_t raw;
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
                    uint32_t raw;
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
                    uint32_t raw;
                } EDX;
            };

            /** @brief CPU serial number */
            struct CPUID0x3
            {
                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    uint32_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    uint32_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t ProcessorSerialNumber : 32;
                    };
                    uint32_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t ProcessorSerialNumber : 32;
                    };
                    uint32_t raw;
                } EDX;
            };

            /** @brief Cache information */
            struct CPUID0x4_1
            {
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
                    uint32_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t SystemCoherencyLineSize : 12;
                        uint32_t PhysicalLinePartitions : 10;
                        uint32_t WaysOfAssociativity : 10;
                    };
                    uint32_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    uint32_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    uint32_t raw;
                } EDX;
            };

            /** @brief MONITOR information */
            struct CPUID0x5
            {
                union
                {
                    struct
                    {
                        uint32_t SmallestMonitorLineSize : 16;
                        uint32_t Reserved : 16;
                    };
                    uint32_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t LargestMonitorLineSize : 16;
                        uint32_t Reserved : 16;
                    };
                    uint32_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t MWAITEnumerationSupported : 1;
                        uint32_t InterruptsAsBreakEvent : 1;
                        uint32_t Reserved : 30;
                    };
                    uint32_t raw;
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
                    uint32_t raw;
                } EDX;
            };

            /** @brief Thermal and power management information */
            struct CPUID0x6
            {
                union
                {
                    struct
                    {
                        uint32_t SensorSupported : 1;
                        uint32_t Reserved : 31;
                    };
                    uint32_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t InterruptThreshold : 4;
                        uint32_t Reserved : 26;
                    };
                    uint32_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t ACNT_MCNT : 1;
                        uint32_t Reserved : 31;
                    };
                    uint32_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    uint32_t raw;
                } EDX;
            };

            /** @brief Performance monitors */
            struct CPUID0xA
            {
                union
                {
                    struct
                    {
                        uint32_t VersionID : 8;
                        uint32_t NumberCounters : 8;
                        uint32_t BitWidthOfCounters : 8;
                        uint32_t LengthOfEBXBitVector : 8;
                    };
                    uint32_t raw;
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
                    uint32_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t FixedFunctionCounters : 5;
                        uint32_t CounterWidth : 8;
                        uint32_t Reserved : 19;
                    };
                    uint32_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    uint32_t raw;
                } EDX;
            };

            /** @brief Get CPU frequency information */
            struct CPUID0x15
            {
                union
                {
                    struct
                    {
                        uint32_t VersionID : 8;
                        uint32_t NumberCounters : 8;
                        uint32_t BitWidthOfCounters : 8;
                        uint32_t LengthOfEBXBitVector : 8;
                    };
                    uint32_t raw;
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
                    uint32_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t FixedFunctionCounters : 5;
                        uint32_t CounterWidth : 8;
                        uint32_t Reserved : 19;
                    };
                    uint32_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    uint32_t raw;
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
                        uint32_t Denominator : 31;
                    };
                    uint32_t raw;
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
                    uint32_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        /** @brief Core crystal clock frequency in Hz */
                        uint32_t CoreCrystalClock : 31;
                    };
                    uint32_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        /** @brief Reserved */
                        uint32_t Reserved : 31;
                    };
                    uint32_t raw;
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
                        uint32_t MaximumInputValue : 32;
                    };
                    uint32_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        /** @brief Hypervisor vendor signature */
                        char Hypervisor[4];
                    };
                    uint32_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        /** @brief Hypervisor vendor signature */
                        char Hypervisor[4];
                    };
                    uint32_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        /** @brief Hypervisor vendor signature */
                        char Hypervisor[4];
                    };
                    uint32_t raw;
                } EDX;
            };

            /** @brief Extended CPU information */
            struct CPUID0x80000000
            {
                union
                {
                    struct
                    {
                        uint32_t HighestExtendedFunctionSupported : 32;
                    };
                    uint32_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    uint32_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    uint32_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    uint32_t raw;
                } EDX;
                uint32_t raw;
            };

            /** @brief Extended CPU information */
            struct CPUID0x80000001
            {
                union
                {
                    struct
                    {
                        uint32_t Unknown : 32;
                    };
                    uint32_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    uint32_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t LAHF_SAHF : 1;
                        uint32_t Reserved : 31;
                    };
                    uint32_t raw;
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
                    uint32_t raw;
                } EDX;
                uint32_t raw;
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
                    uint32_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint32_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint32_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint32_t raw;
                } EDX;
                uint32_t raw;
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
                    uint32_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint32_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint32_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint32_t raw;
                } EDX;
                uint32_t raw;
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
                    uint32_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint32_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint32_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        char Brand[4];
                    };
                    uint32_t raw;
                } EDX;
                uint32_t raw;
            };

            /** @brief CPU cache line information */
            struct CPUID0x80000006
            {
                union
                {
                    struct
                    {
                        uint32_t InstructionCount : 12;
                        uint32_t InstructionAssociativity : 4;
                        uint32_t DataCount : 12;
                        uint32_t DataAssociativity : 4;
                    };
                    uint32_t raw;
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
                    uint32_t raw;
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
                    uint32_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    uint32_t raw;
                } EDX;
                uint32_t raw;
            };

            /** @brief Virtual and physical memory size */
            struct CPUID0x80000008
            {
                union
                {
                    struct
                    {
                        uint32_t PhysicalAddressBits : 8;
                        uint32_t LinearAddressBits : 8;
                        uint32_t Reserved : 16;
                    };
                    uint32_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    uint32_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    uint32_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    uint32_t raw;
                } EDX;
                uint32_t raw;
            };

            /** @brief Secure virtual machine parameters */
            struct CPUID0x8000000A
            {
                union
                {
                    struct
                    {
                        uint32_t SVMRevision : 8;
                        uint32_t Reserved : 24;
                    };
                    uint32_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    uint32_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    uint32_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint32_t Reserved : 32;
                    };
                    uint32_t raw;
                } EDX;
                uint32_t raw;
            };
        }
    }
}

#endif // !__FENNIX_KERNEL_CPU_x32_CPUID_INTEL_H__
