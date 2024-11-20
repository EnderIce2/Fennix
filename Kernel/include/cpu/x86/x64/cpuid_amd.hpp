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
                        char Vendor[4];
                    };
                    uint64_t raw;
                } EBX;
                union
                {
                    struct
                    {
                        char Vendor[4];
                    };
                    uint64_t raw;
                } ECX;
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
            struct CPUID0x1
            {
                union
                {
                    struct
                    {
                        uint64_t SteppingID : 4;
                        uint64_t ModelID : 4;
                        uint64_t FamilyID : 4;
                        uint64_t Reserved0 : 4;
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
                        uint64_t Reserved0 : 1;
                        uint64_t MONITOR : 1;
                        uint64_t Reserved1 : 1;
                        uint64_t DS_CPL : 1;
                        uint64_t Reserved2 : 1;
                        uint64_t SMX : 1;
                        uint64_t Reserved3 : 1;
                        uint64_t TM2 : 1;
                        uint64_t Reserved4 : 1;
                        uint64_t CNXT_ID : 1;
                        uint64_t Reserved5 : 1;
                        uint64_t CMPXCHG16B : 1;
                        uint64_t Reserved6 : 1;
                        uint64_t xTPRUpdateControl : 1;
                        uint64_t Reserved7 : 1;
                        uint64_t Reserved8 : 1;
                        uint64_t DCA : 1;
                        uint64_t Reserved9 : 1;
                        uint64_t SSE4_1 : 1;
                        uint64_t SSE4_2 : 1;
                        uint64_t Reserved10 : 1;
                        uint64_t MOVBE : 1;
                        uint64_t POPCNT : 1;
                        uint64_t Reserved11 : 1;
                        uint64_t AES : 1;
                        uint64_t Reserved12 : 1;
                        uint64_t XSAVE : 1;
                        uint64_t OSXSAVE : 1;
                        uint64_t AVX : 1;
                        uint64_t Reserved13 : 1;
                        uint64_t RDRAND : 1;
                        uint64_t Reserved14 : 1;
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
                        uint64_t L1DataCacheSize : 8;
                        uint64_t L1DataCacheAssociativity : 8;
                        uint64_t L1DataCacheLineSize : 8;
                        uint64_t L1DataCachePartitions : 8;
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint64_t L1InstructionCacheSize : 8;
                        uint64_t L1InstructionCacheAssociativity : 8;
                        uint64_t L1InstructionCacheLineSize : 8;
                        uint64_t L1InstructionCachePartitions : 8;
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint64_t L2UnifiedCacheSize : 16;
                        uint64_t L2UnifiedCacheAssociativity : 8;
                        uint64_t L2UnifiedCacheLineSize : 8;
                        uint64_t L2UnifiedCachePartitions : 8;
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint64_t L3UnifiedCacheSize : 18;
                        uint64_t L3UnifiedCacheAssociativity : 8;
                        uint64_t L3UnifiedCacheLineSize : 8;
                        uint64_t L3UnifiedCachePartitions : 8;
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
            struct CPUID0x80000001
            {
                union
                {
                    struct
                    {
                        uint64_t SteppingID : 4;
                        uint64_t ModelID : 4;
                        uint64_t FamilyID : 4;
                        uint64_t Reserved0 : 4;
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
                        uint64_t BrandId : 16;
                        uint64_t Reserved0 : 12;
                        uint64_t PkgType : 4;
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint64_t LAHF_SAHF : 1;
                        uint64_t CmpLegacy : 1;
                        uint64_t SVM : 1;
                        uint64_t ExtApicSpace : 1;
                        uint64_t AltMovCr8 : 1;
                        uint64_t ABM : 1;
                        uint64_t SSE4A : 1;
                        uint64_t MisalignedSSE : 1;
                        uint64_t ThreeDNowPrefetch : 1;
                        uint64_t OSVW : 1;
                        uint64_t IBS : 1;
                        uint64_t XOP : 1;
                        uint64_t SKINIT : 1;
                        uint64_t WDT : 1;
                        uint64_t Reserved0 : 1;
                        uint64_t LWP : 1;
                        uint64_t FMA4 : 1;
                        uint64_t Reserved1 : 1;
                        uint64_t Reserved2 : 1;
                        uint64_t NodeID : 1;
                        uint64_t Reserved3 : 1;
                        uint64_t TBM : 1;
                        uint64_t TopologyExtensions : 1;
                        uint64_t Reserved4 : 9;
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
                        uint64_t CMPXCHG8B : 1;
                        uint64_t APIC : 1;
                        uint64_t Reserved0 : 1;
                        uint64_t SYSCALL : 1;
                        uint64_t MTRR : 1;
                        uint64_t PGE : 1;
                        uint64_t MCA : 1;
                        uint64_t CMOV : 1;
                        uint64_t PAT : 1;
                        uint64_t PSE36 : 1;
                        uint64_t Reserved1 : 1;
                        uint64_t ExeDisable : 1;
                        uint64_t Reserved2 : 1;
                        uint64_t MMXExtended : 1;
                        uint64_t MMX : 1;
                        uint64_t FXSR : 1;
                        uint64_t FFXSR : 1;
                        uint64_t Reserved3 : 1;
                        uint64_t RDTSCP : 1;
                        uint64_t Reserved4 : 1;
                        uint64_t LongMode : 1;
                        uint64_t ThreeDNowExtended : 1;
                        uint64_t ThreeDNow : 1;
                    };
                    uint64_t raw;
                } EDX;
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
            };

            /** @brief Cache and TLB information */
            struct CPUID0x80000005
            {
                union
                {
                    struct
                    {
                        uint64_t InstructionCount : 8;
                        uint64_t InstructionAssociativity : 8;
                        uint64_t DataCount : 8;
                        uint64_t DataAssociativity : 8;
                    };
                    uint64_t raw;
                } EAX;

                union
                {
                    struct
                    {
                        uint64_t InstructionCount : 8;
                        uint64_t InstructionAssociativity : 8;
                        uint64_t DataCount : 8;
                        uint64_t DataAssociativity : 8;
                    };
                    uint64_t raw;
                } EBX;

                union
                {
                    struct
                    {
                        uint64_t LineSize : 8;
                        uint64_t LinePerTag : 8;
                        uint64_t Associativity : 8;
                        uint64_t CacheSize : 8;
                    };
                    uint64_t raw;
                } ECX;

                union
                {
                    struct
                    {
                        uint64_t LineSize : 8;
                        uint64_t LinePerTag : 8;
                        uint64_t Associativity : 8;
                        uint64_t CacheSize : 8;
                    };
                    uint64_t raw;
                } EDX;
            };

            /** @brief CPU cache line information */
            struct CPUID0x80000006
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
            };

            /** @brief APM */
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
                        uint64_t TemperatureSensor : 1;
                        uint64_t FrequencyID : 1;
                        uint64_t VoltageID : 1;
                        uint64_t ThermaTrip : 1;
                        uint64_t HarwareThermalControl : 1;
                        uint64_t SoftwareThermalControl : 1;
                        uint64_t Reserved0 : 2;
                        uint64_t TSCInvariant : 1;
                        uint64_t Reserved1 : 23;
                    };
                    uint64_t raw;
                } EDX;
            };
        }
    }
}

#endif // !__FENNIX_KERNEL_CPU_x64_CPUID_AMD_H__
