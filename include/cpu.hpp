#ifndef __FENNIX_KERNEL_CPU_H__
#define __FENNIX_KERNEL_CPU_H__

#include <types.h>

#include <cstring>

#define x86_CPUID_VENDOR_OLDAMD "AMDisbetter!" /* Early engineering samples of AMD K5 processor */
#define x86_CPUID_VENDOR_AMD "AuthenticAMD"
#define x86_CPUID_VENDOR_INTEL "GenuineIntel"
#define x86_CPUID_VENDOR_VIA "CentaurHauls"
#define x86_CPUID_VENDOR_OLDTRANSMETA "TransmetaCPU"
#define x86_CPUID_VENDOR_TRANSMETA "GenuineTMx86"
#define x86_CPUID_VENDOR_CYRIX "CyrixInstead"
#define x86_CPUID_VENDOR_CENTAUR "CentaurHauls"
#define x86_CPUID_VENDOR_NEXGEN "NexGenDriven"
#define x86_CPUID_VENDOR_UMC "UMC UMC UMC "
#define x86_CPUID_VENDOR_SIS "SiS SiS SiS "
#define x86_CPUID_VENDOR_NSC "Geode by NSC"
#define x86_CPUID_VENDOR_RISE "RiseRiseRise"
#define x86_CPUID_VENDOR_VORTEX "Vortex86 SoC"
#define x86_CPUID_VENDOR_VIA2 "VIA VIA VIA "
#define x86_CPUID_VENDOR_HYGON "HygonGenuine"
#define x86_CPUID_VENDOR_E2K "E2K MACHINE"
#define x86_CPUID_VENDOR_MISTER "MiSTer AO486"

/* Vendor-strings from Virtual Machines. */
#define x86_CPUID_VENDOR_VMWARE "VMwareVMware"
#define x86_CPUID_VENDOR_XENHVM "XenVMMXenVMM"
#define x86_CPUID_VENDOR_MICROSOFT_HV "Microsoft Hv"
#define x86_CPUID_VENDOR_MICROSOFT_XTA "MicrosoftXTA"
#define x86_CPUID_VENDOR_PARALLELS " lrpepyh vr"
#define x86_CPUID_VENDOR_KVM "KVMKVMKVM"
#define x86_CPUID_VENDOR_VIRTUALBOX "VBoxVBoxVBox"
#define x86_CPUID_VENDOR_TCG "TCGTCGTCGTCG"
#define x86_CPUID_VENDOR_BHYVE "bhyve bhyve "
#define x86_CPUID_VENDOR_ACRN "ACRNACRNACRN"
#define x86_CPUID_VENDOR_QNX "QNXQVMBSQG"
#define x86_CPUID_VENDOR_APPLE "VirtualApple"

#define x86_CPUID_SIGNATURE_INTEL_b 0x756e6547
#define x86_CPUID_SIGNATURE_INTEL_c 0x6c65746e
#define x86_CPUID_SIGNATURE_INTEL_d 0x49656e69

#define x86_CPUID_SIGNATURE_AMD_b 0x68747541
#define x86_CPUID_SIGNATURE_AMD_c 0x444d4163
#define x86_CPUID_SIGNATURE_AMD_d 0x69746e65

#define x86_CPUID_SIGNATURE_CENTAUR_b 0x746e6543
#define x86_CPUID_SIGNATURE_CENTAUR_c 0x736c7561
#define x86_CPUID_SIGNATURE_CENTAUR_d 0x48727561

#define x86_CPUID_SIGNATURE_CYRIX_b 0x69727943
#define x86_CPUID_SIGNATURE_CYRIX_c 0x64616574
#define x86_CPUID_SIGNATURE_CYRIX_d 0x736e4978

#define x86_CPUID_SIGNATURE_TM1_b 0x6e617254
#define x86_CPUID_SIGNATURE_TM1_c 0x55504361
#define x86_CPUID_SIGNATURE_TM1_d 0x74656d73

#define x86_CPUID_SIGNATURE_TM2_b 0x756e6547
#define x86_CPUID_SIGNATURE_TM2_c 0x3638784d
#define x86_CPUID_SIGNATURE_TM2_d 0x54656e69

#define x86_CPUID_SIGNATURE_NSC_b 0x646f6547
#define x86_CPUID_SIGNATURE_NSC_c 0x43534e20
#define x86_CPUID_SIGNATURE_NSC_d 0x79622065

#define x86_CPUID_SIGNATURE_NEXGEN_b 0x4778654e
#define x86_CPUID_SIGNATURE_NEXGEN_c 0x6e657669
#define x86_CPUID_SIGNATURE_NEXGEN_d 0x72446e65

#define x86_CPUID_SIGNATURE_RISE_b 0x65736952
#define x86_CPUID_SIGNATURE_RISE_c 0x65736952
#define x86_CPUID_SIGNATURE_RISE_d 0x65736952

#define x86_CPUID_SIGNATURE_SIS_b 0x20536953
#define x86_CPUID_SIGNATURE_SIS_c 0x20536953
#define x86_CPUID_SIGNATURE_SIS_d 0x20536953

#define x86_CPUID_SIGNATURE_UMC_b 0x20434d55
#define x86_CPUID_SIGNATURE_UMC_c 0x20434d55
#define x86_CPUID_SIGNATURE_UMC_d 0x20434d55

#define x86_CPUID_SIGNATURE_VIA_b 0x20414956
#define x86_CPUID_SIGNATURE_VIA_c 0x20414956
#define x86_CPUID_SIGNATURE_VIA_d 0x20414956

#define x86_CPUID_SIGNATURE_VORTEX_b 0x74726f56
#define x86_CPUID_SIGNATURE_VORTEX_c 0x436f5320
#define x86_CPUID_SIGNATURE_VORTEX_d 0x36387865

/**
 * @brief CPU related functions.
 */
namespace CPU
{
    /**
     * @brief Enum for CPU::Interrupts() function.
     */
    enum InterruptsType
    {
        /**
         * @brief Check if interrupts are enabled.
         */
        Check,
        /**
         * @brief Enable interrupts.
         */
        Enable,
        /**
         * @brief Disable interrupts.
         */
        Disable
    };

    /**
     * @brief Get CPU vendor identifier.
     *
     * @return char* CPU Vendor ID.
     */
    char *Vendor();

    /**
     * @brief Get CPU name.
     *
     * @return char* CPU Name.
     */
    char *Name();

    /**
     * @brief Get CPU hypervisor vendor.
     *
     * @return char* Hypervisor vendor.
     */
    char *Hypervisor();

    /**
     * @brief Pause the CPU
     */
    __attribute__((no_stack_protector)) static inline void Pause(bool Loop = false)
    {
        do
        {
#if defined(__amd64__) || defined(__i386__)
            asmv("pause");
#elif defined(__aarch64__)
            asmv("yield");
#endif
        } while (Loop);
    }

    /**
     * @brief Stop the CPU (infinite loop)
     */
    __attribute__((no_stack_protector)) static inline void Stop()
    {
        while (1)
        {
#if defined(__amd64__) || defined(__i386__)
            asmv("CPUStopLoop:\n"
                 "cli\n"
                 "hlt\n"
                 "jmp CPUStopLoop");
#elif defined(__aarch64__)
            asmv("msr daifset, #2");
            asmv("wfe");
#endif
        }
    }

    /**
     * @brief Halt the CPU
     */
    __attribute__((no_stack_protector)) static inline void Halt(bool Loop = false)
    {
        do
        {
#if defined(__amd64__) || defined(__i386__)
            asmv("hlt");
#elif defined(__aarch64__)
            asmv("wfe");
#endif
        } while (Loop);
    }

    /**
     * @brief Check if interrupts are enabled
     *
     * @return true If InterruptsType::Check and interrupts are enabled, or if other InterruptsType were executed successfully
     * @return false If InterruptsType::Check and interrupts are disabled, or if other InterruptsType failed
     */
    bool Interrupts(InterruptsType Type = Check);

    /**
     * @brief Get/Set the CPU's page table
     *
     * @param PT The new page table, if empty, the current page table will be returned
     * @return void* The current page table
     */
    void *PageTable(void *PT = nullptr);

    /** @brief To be used only once. */
    void InitializeFeatures();

    namespace MemBar
    {
        __attribute__((no_stack_protector)) static inline void Barrier()
        {
#if defined(__amd64__) || defined(__i386__)
            asmv("" ::
                     : "memory");
#elif defined(__aarch64__)
            asmv("dmb ish" ::
                     : "memory");
#endif
        }

        __attribute__((no_stack_protector)) static inline void Fence()
        {
#if defined(__amd64__) || defined(__i386__)
            asmv("mfence" ::
                     : "memory");
#elif defined(__aarch64__)
            asmv("dmb ish" ::
                     : "memory");
#endif
        }

        __attribute__((no_stack_protector)) static inline void StoreFence()
        {
#if defined(__amd64__) || defined(__i386__)
            asmv("sfence" ::
                     : "memory");
#elif defined(__aarch64__)
            asmv("dmb ishst" ::
                     : "memory");
#endif
        }

        __attribute__((no_stack_protector)) static inline void LoadFence()
        {
#if defined(__amd64__) || defined(__i386__)
            asmv("lfence" ::
                     : "memory");
#elif defined(__aarch64__)
            asmv("dmb ishld" ::
                     : "memory");
#endif
        }
    }

    namespace x32
    {
    }

    namespace x64
    {
        enum CPUIDFeatures
        {
            CPUID_FEAT_RCX_SSE3 = 1 << 0,
            CPUID_FEAT_RCX_PCLMULQDQ = 1 << 1,
            CPUID_FEAT_RCX_DTES64 = 1 << 2,
            CPUID_FEAT_RCX_MONITOR = 1 << 3,
            CPUID_FEAT_RCX_DS_CPL = 1 << 4,
            CPUID_FEAT_RCX_VMX = 1 << 5,
            CPUID_FEAT_RCX_SMX = 1 << 6,
            CPUID_FEAT_RCX_EST = 1 << 7,
            CPUID_FEAT_RCX_TM2 = 1 << 8,
            CPUID_FEAT_RCX_SSSE3 = 1 << 9,
            CPUID_FEAT_RCX_CID = 1 << 10,
            CPUID_FEAT_RCX_FMA = 1 << 12,
            CPUID_FEAT_RCX_CX16 = 1 << 13,
            CPUID_FEAT_RCX_ETPRD = 1 << 14,
            CPUID_FEAT_RCX_PDCM = 1 << 15,
            CPUID_FEAT_RCX_PCIDE = 1 << 17,
            CPUID_FEAT_RCX_DCA = 1 << 18,
            CPUID_FEAT_RCX_SSE4_1 = 1 << 19,
            CPUID_FEAT_RCX_SSE4_2 = 1 << 20,
            CPUID_FEAT_RCX_x2APIC = 1 << 21,
            CPUID_FEAT_RCX_MOVBE = 1 << 22,
            CPUID_FEAT_RCX_POPCNT = 1 << 23,
            CPUID_FEAT_RCX_AES = 1 << 25,
            CPUID_FEAT_RCX_XSAVE = 1 << 26,
            CPUID_FEAT_RCX_OSXSAVE = 1 << 27,
            CPUID_FEAT_RCX_AVX = 1 << 28,
            CPUID_FEAT_RCX_F16C = 1 << 29,
            CPUID_FEAT_RCX_RDRAND = 1 << 30,

            CPUID_FEAT_RDX_FPU = 1 << 0,
            CPUID_FEAT_RDX_VME = 1 << 1,
            CPUID_FEAT_RDX_DE = 1 << 2,
            CPUID_FEAT_RDX_PSE = 1 << 3,
            CPUID_FEAT_RDX_TSC = 1 << 4,
            CPUID_FEAT_RDX_MSR = 1 << 5,
            CPUID_FEAT_RDX_PAE = 1 << 6,
            CPUID_FEAT_RDX_MCE = 1 << 7,
            CPUID_FEAT_RDX_CX8 = 1 << 8,
            CPUID_FEAT_RDX_APIC = 1 << 9,
            CPUID_FEAT_RDX_SEP = 1 << 11,
            CPUID_FEAT_RDX_MTRR = 1 << 12,
            CPUID_FEAT_RDX_PGE = 1 << 13,
            CPUID_FEAT_RDX_MCA = 1 << 14,
            CPUID_FEAT_RDX_CMOV = 1 << 15,
            CPUID_FEAT_RDX_PAT = 1 << 16,
            CPUID_FEAT_RDX_PSE36 = 1 << 17,
            CPUID_FEAT_RDX_PSN = 1 << 18,
            CPUID_FEAT_RDX_CLF = 1 << 19,
            CPUID_FEAT_RDX_DTES = 1 << 21,
            CPUID_FEAT_RDX_ACPI = 1 << 22,
            CPUID_FEAT_RDX_MMX = 1 << 23,
            CPUID_FEAT_RDX_FXSR = 1 << 24,
            CPUID_FEAT_RDX_SSE = 1 << 25,
            CPUID_FEAT_RDX_SSE2 = 1 << 26,
            CPUID_FEAT_RDX_SS = 1 << 27,
            CPUID_FEAT_RDX_HTT = 1 << 28,
            CPUID_FEAT_RDX_TM1 = 1 << 29,
            CPUID_FEAT_RDX_IA64 = 1 << 30,
            CPUID_FEAT_RDX_PBE = 1 << 31,

            // ? Not sure how to get it.
            CPUID_FEAT_RDX_SMEP = 1 << 7,
            CPUID_FEAT_RDX_UMIP = 1 << 2,
            CPUID_FEAT_RDX_SYSCALL = 1 << 11,
            CPUID_FEAT_XD = 1 << 20,
            CPUID_FEAT_1GB_PAGE = 1 << 26,
            CPUID_FEAT_RDTSCP = 1 << 27,
            CPUID_FEAT_LONG_MODE = 1 << 29,
            CPUID_FEAT_RDX_SMAP = (1 << 20)
        };

        enum MSRID
        {
            MSR_MONITOR_FILTER_SIZE = 0x6,
            MSR_TIME_STAMP_COUNTER = 0x10,
            MSR_PLATFORM_ID = 0x17,
            MSR_APIC_BASE = 0x1B,
            MSR_FEATURE_CONTROL = 0x3A,
            MSR_TSC_ADJUST = 0x3B,
            MSR_SPEC_CTRL = 0x48,
            MSR_PRED_CMD = 0x49,
            MSR_BIOS_UPDT_TRIG = 0x79,
            MSR_BIOS_SIGN_ID = 0x8B,
            MSR_SGXLEPUBKEYHASH0 = 0x8C,
            MSR_SGXLEPUBKEYHASH1 = 0x8D,
            MSR_SGXLEPUBKEYHASH2 = 0x8E,
            MSR_SGXLEPUBKEYHASH3 = 0x8F,
            MSR_SMM_MONITOR_CTL = 0x9B,
            MSR_SMBASE = 0x9E,
            MSR_PMC0 = 0xC1,
            MSR_PMC1 = 0xC2,
            MSR_PMC2 = 0xC3,
            MSR_PMC3 = 0xC4,
            MSR_PMC4 = 0xC5,
            MSR_PMC5 = 0xC6,
            MSR_PMC6 = 0xC7,
            MSR_PMC7 = 0xC8,
            MSR_UMWAIT_CONTROL = 0xE1,
            MSR_MPERF = 0xE7,
            MSR_APERF = 0xE8,
            MSR_MTRRCAP = 0xFE,
            MSR_ARCH_CAPABILITIES = 0x10A,
            MSR_FLUSH_CMD = 0x10B,
            MSR_SYSENTER_CS = 0x17A,
            MSR_SYSENTER_ESP = 0x175,
            MSR_SYSENTER_EIP = 0x176,
            MSR_MCG_CAP = 0x179,
            MSR_MCG_STATUS = 0x17A,
            MSR_MCG_CTL = 0x17B,
            MSR_PERFEVTSEL0 = 0x186,
            MSR_PERFEVTSEL1 = 0x187,
            MSR_PERFEVTSEL2 = 0x188,
            MSR_PERFEVTSEL3 = 0x189,
            MSR_PERF_STATUS = 0x198,
            MSR_PERF_CTL = 0x199,
            MSR_CLOCK_MODULATION = 0x19A,
            MSR_THERM_INTERRUPT = 0x19B,
            MSR_THERM_STATUS = 0x19C,
            MSR_MISC_ENABLE = 0x1A0,
            MSR_ENERGY_PERF_BIAS = 0x1B0,
            MSR_PACKAGE_THERM_STATUS = 0x1B1,
            MSR_PACKAGE_THERM_INTERRUPT = 0x1B2,
            MSR_DEBUGCTL = 0x1D9,
            MSR_SMRR_PHYSBASE = 0x1F2,
            MSR_SMRR_PHYSMASK = 0x1F3,
            MSR_PLATFORM_DCA_CAP = 0x1F8,
            MSR_CPU_DCA_CAP = 0x1F9,
            MSR_DCA_0_CAP = 0x1FA,
            MSR_MTRR_PHYSBASE0 = 0x200,
            MSR_MTRR_PHYSMASK0 = 0x201,
            MSR_MTRR_PHYSBASE1 = 0x202,
            MSR_MTRR_PHYSMASK1 = 0x203,
            MSR_MTRR_PHYSBASE2 = 0x204,
            MSR_MTRR_PHYSMASK2 = 0x205,
            MSR_MTRR_PHYSBASE3 = 0x206,
            MSR_MTRR_PHYSMASK3 = 0x207,
            MSR_MTRR_PHYSBASE4 = 0x208,
            MSR_MTRR_PHYSMASK4 = 0x209,
            MSR_MTRR_PHYSBASE5 = 0x20A,
            MSR_MTRR_PHYSMASK5 = 0x20B,
            MSR_MTRR_PHYSBASE6 = 0x20C,
            MSR_MTRR_PHYSMASK6 = 0x20D,
            MSR_MTRR_PHYSBASE7 = 0x20E,
            MSR_MTRR_PHYSMASK7 = 0x20F,
            MSR_MTRR_PHYSBASE8 = 0x210,
            MSR_MTRR_PHYSMASK8 = 0x211,
            MSR_MTRR_PHYSBASE9 = 0x212,
            MSR_MTRR_PHYSMASK9 = 0x213,
            MSR_MTRR_FIX64K_00000 = 0x250,
            MSR_MTRR_FIX16K_80000 = 0x258,
            MSR_MTRR_FIX16K_A0000 = 0x259,
            MSR_MTRR_FIX4K_C0000 = 0x268,
            MSR_MTRR_FIX4K_C8000 = 0x269,
            MSR_MTRR_FIX4K_D0000 = 0x26A,
            MSR_MTRR_FIX4K_D8000 = 0x26B,
            MSR_MTRR_FIX4K_E0000 = 0x26C,
            MSR_MTRR_FIX4K_E8000 = 0x26D,
            MSR_MTRR_FIX4K_F0000 = 0x26E,
            MSR_MTRR_FIX4K_F8000 = 0x26F,
            MSR_PAT = 0x277,
            MSR_MC0_CTL2 = 0x280,
            MSR_MC1_CTL2 = 0x281,
            MSR_MC2_CTL2 = 0x282,
            MSR_MC3_CTL2 = 0x283,
            MSR_MC4_CTL2 = 0x284,
            MSR_MC5_CTL2 = 0x285,
            MSR_MC6_CTL2 = 0x286,
            MSR_MC7_CTL2 = 0x287,
            MSR_MC8_CTL2 = 0x288,
            MSR_MC9_CTL2 = 0x289,
            MSR_MC10_CTL2 = 0x28A,
            MSR_MC11_CTL2 = 0x28B,
            MSR_MC12_CTL2 = 0x28C,
            MSR_MC13_CTL2 = 0x28D,
            MSR_MC14_CTL2 = 0x28E,
            MSR_MC15_CTL2 = 0x28F,
            MSR_MC16_CTL2 = 0x290,
            MSR_MC17_CTL2 = 0x291,
            MSR_MC18_CTL2 = 0x292,
            MSR_MC19_CTL2 = 0x293,
            MSR_MC20_CTL2 = 0x294,
            MSR_MC21_CTL2 = 0x295,
            MSR_MC22_CTL2 = 0x296,
            MSR_MC23_CTL2 = 0x297,
            MSR_MC24_CTL2 = 0x298,
            MSR_MC25_CTL2 = 0x299,
            MSR_MC26_CTL2 = 0x29A,
            MSR_MC27_CTL2 = 0x29B,
            MSR_MC28_CTL2 = 0x29C,
            MSR_MC29_CTL2 = 0x29D,
            MSR_MC30_CTL2 = 0x29E,
            MSR_MC31_CTL2 = 0x29F,
            MSR_MTRR_DEF_TYPE = 0x2FF,
            MSR_FIXED_CTR0 = 0x309,
            MSR_FIXED_CTR1 = 0x30A,
            MSR_FIXED_CTR2 = 0x30B,
            MSR_PERF_CAPABILITIES = 0x345,
            MSR_FIXED_CTR_CTRL = 0x38D,
            MSR_PERF_GLOBAL_STATUS = 0x38E,
            MSR_PERF_GLOBAL_CTRL = 0x38F,
            MSR_PERF_GLOBAL_STATUS_RESET = 0x390,
            MSR_PERF_GLOBAL_STATUS_SET = 0x391,
            MSR_PERF_GLOBAL_INUSE = 0x392,
            MSR_PEBS_ENABLE = 0x3F1,
            MSR_MC0_CTL = 0x400,
            MSR_MC0_STATUS = 0x401,
            MSR_MC0_ADDR = 0x402,
            MSR_MC0_MISC = 0x403,
            MSR_MC1_CTL = 0x404,
            MSR_MC1_STATUS = 0x405,
            MSR_MC1_ADDR = 0x406,
            MSR_MC1_MISC = 0x407,
            MSR_MC2_CTL = 0x408,
            MSR_MC2_STATUS = 0x409,
            MSR_MC2_ADDR = 0x40A,
            MSR_MC2_MISC = 0x40B,
            MSR_MC3_CTL = 0x40C,
            MSR_MC3_STATUS = 0x40D,
            MSR_MC3_ADDR = 0x40E,
            MSR_MC3_MISC = 0x40F,
            MSR_MC4_CTL = 0x410,
            MSR_MC4_STATUS = 0x411,
            MSR_MC4_ADDR = 0x412,
            MSR_MC4_MISC = 0x413,
            MSR_MC5_CTL = 0x414,
            MSR_MC5_STATUS = 0x415,
            MSR_MC5_ADDR = 0x416,
            MSR_MC5_MISC = 0x417,
            MSR_MC6_CTL = 0x418,
            MSR_MC6_STATUS = 0x419,
            MSR_MC6_ADDR = 0x41A,
            MSR_MC6_MISC = 0x41B,
            MSR_MC7_CTL = 0x41C,
            MSR_MC7_STATUS = 0x41D,
            MSR_MC7_ADDR = 0x41E,
            MSR_MC7_MISC = 0x41F,
            MSR_MC8_CTL = 0x420,
            MSR_MC8_STATUS = 0x421,
            MSR_MC8_ADDR = 0x422,
            MSR_MC8_MISC = 0x423,
            MSR_MC9_CTL = 0x424,
            MSR_MC9_STATUS = 0x425,
            MSR_MC9_ADDR = 0x426,
            MSR_MC9_MISC = 0x427,
            MSR_MC10_CTL = 0x428,
            MSR_MC10_STATUS = 0x429,
            MSR_MC10_ADDR = 0x42A,
            MSR_MC10_MISC = 0x42B,
            MSR_MC11_CTL = 0x42C,
            MSR_MC11_STATUS = 0x42D,
            MSR_MC11_ADDR = 0x42E,
            MSR_MC11_MISC = 0x42F,
            MSR_MC12_CTL = 0x430,
            MSR_MC12_STATUS = 0x431,
            MSR_MC12_ADDR = 0x432,
            MSR_MC12_MISC = 0x433,
            MSR_MC13_CTL = 0x434,
            MSR_MC13_STATUS = 0x435,
            MSR_MC13_ADDR = 0x436,
            MSR_MC13_MISC = 0x437,
            MSR_MC14_CTL = 0x438,
            MSR_MC14_STATUS = 0x439,
            MSR_MC14_ADDR = 0x43A,
            MSR_MC14_MISC = 0x43B,
            MSR_MC15_CTL = 0x43C,
            MSR_MC15_STATUS = 0x43D,
            MSR_MC15_ADDR = 0x43E,
            MSR_MC15_MISC = 0x43F,
            MSR_MC16_CTL = 0x440,
            MSR_MC16_STATUS = 0x441,
            MSR_MC16_ADDR = 0x442,
            MSR_MC16_MISC = 0x443,
            MSR_MC17_CTL = 0x444,
            MSR_MC17_STATUS = 0x445,
            MSR_MC17_ADDR = 0x446,
            MSR_MC17_MISC = 0x447,
            MSR_MC18_CTL = 0x448,
            MSR_MC18_STATUS = 0x449,
            MSR_MC18_ADDR = 0x44A,
            MSR_MC18_MISC = 0x44B,
            MSR_MC19_CTL = 0x44C,
            MSR_MC19_STATUS = 0x44D,
            MSR_MC19_ADDR = 0x44E,
            MSR_MC19_MISC = 0x44F,
            MSR_MC20_CTL = 0x450,
            MSR_MC20_STATUS = 0x451,
            MSR_MC20_ADDR = 0x452,
            MSR_MC20_MISC = 0x453,
            MSR_MC21_CTL = 0x454,
            MSR_MC21_STATUS = 0x455,
            MSR_MC21_ADDR = 0x456,
            MSR_MC21_MISC = 0x457,
            MSR_MC22_CTL = 0x458,
            MSR_MC22_STATUS = 0x459,
            MSR_MC22_ADDR = 0x45A,
            MSR_MC22_MISC = 0x45B,
            MSR_MC23_CTL = 0x45C,
            MSR_MC23_STATUS = 0x45D,
            MSR_MC23_ADDR = 0x45E,
            MSR_MC23_MISC = 0x45F,
            MSR_MC24_CTL = 0x460,
            MSR_MC24_STATUS = 0x461,
            MSR_MC24_ADDR = 0x462,
            MSR_MC24_MISC = 0x463,
            MSR_MC25_CTL = 0x464,
            MSR_MC25_STATUS = 0x465,
            MSR_MC25_ADDR = 0x466,
            MSR_MC25_MISC = 0x467,
            MSR_MC26_CTL = 0x468,
            MSR_MC26_STATUS = 0x469,
            MSR_MC26_ADDR = 0x46A,
            MSR_MC26_MISC = 0x46B,
            MSR_MC27_CTL = 0x46C,
            MSR_MC27_STATUS = 0x46D,
            MSR_MC27_ADDR = 0x46E,
            MSR_MC27_MISC = 0x46F,
            MSR_MC28_CTL = 0x470,
            MSR_MC28_STATUS = 0x471,
            MSR_MC28_ADDR = 0x472,
            MSR_MC28_MISC = 0x473,
            MSR_VMX_BASIC = 0x480,
            MSR_VMX_PINBASED_CTLS = 0x481,
            MSR_VMX_PROCBASED_CTLS = 0x482,
            MSR_VMX_EXIT_CTLS = 0x483,
            MSR_VMX_ENTRY_CTLS = 0x484,
            MSR_VMX_MISC = 0x485,
            MSR_VMX_CR0_FIXED0 = 0x486,
            MSR_VMX_CR0_FIXED1 = 0x487,
            MSR_VMX_CR4_FIXED0 = 0x488,
            MSR_VMX_CR4_FIXED1 = 0x489,
            MSR_VMX_VMCS_ENUM = 0x48A,
            MSR_VMX_PROCBASED_CTLS2 = 0x48B,
            MSR_VMX_EPT_VPID_CAP = 0x48C,
            MSR_VMX_TRUE_PINBASED_CTLS = 0x48D,
            MSR_VMX_TRUE_PROCBASED_CTLS = 0x48E,
            MSR_VMX_TRUE_EXIT_CTLS = 0x48F,
            MSR_VMX_TRUE_ENTRY_CTLS = 0x490,
            MSR_VMX_VMFUNC = 0x491,
            MSR_A_PMC0 = 0x4C1,
            MSR_A_PMC1 = 0x4C2,
            MSR_A_PMC2 = 0x4C3,
            MSR_A_PMC3 = 0x4C4,
            MSR_A_PMC4 = 0x4C5,
            MSR_A_PMC5 = 0x4C6,
            MSR_A_PMC6 = 0x4C7,
            MSR_A_PMC7 = 0x4C8,
            MSR_MCG_EXT_CTL = 0x4D0,
            MSR_SGX_SVN_STATUS = 0x500,
            MSR_RTIT_OUTPUT_BASE = 0x560,
            MSR_RTIT_OUTPUT_MASK_PTRS = 0x561,
            MSR_RTIT_CTL = 0x570,
            MSR_RTIT_STATUS = 0x571,
            MSR_RTIT_CR3_MATCH = 0x572,
            MSR_RTIT_ADDR0_A = 0x580,
            MSR_RTIT_ADDR0_B = 0x581,
            MSR_RTIT_ADDR1_A = 0x582,
            MSR_RTIT_ADDR1_B = 0x583,
            MSR_RTIT_ADDR2_A = 0x584,
            MSR_RTIT_ADDR2_B = 0x585,
            MSR_RTIT_ADDR3_A = 0x586,
            MSR_RTIT_ADDR3_B = 0x587,
            MSR_DS_AREA = 0x600,
            MSR_TSC_DEADLINE = 0x6E0,
            MSR_PM_ENABLE = 0x770,
            MSR_HWP_CAPABILITIES = 0x771,
            MSR_HWP_REQUEST_PKG = 0x772,
            MSR_HWP_INTERRUPT = 0x773,
            MSR_HWP_REQUEST = 0x774,
            MSR_HWP_STATUS = 0x777,
            MSR_X2APIC_APICID = 0x802,
            MSR_X2APIC_VERSION = 0x803,
            MSR_X2APIC_TPR = 0x808,
            MSR_X2APIC_PPR = 0x80A,
            MSR_X2APIC_EOI = 0x80B,
            MSR_X2APIC_LDR = 0x80D,
            MSR_X2APIC_SIVR = 0x80F,
            MSR_X2APIC_ISR0 = 0x810,
            MSR_X2APIC_ISR1 = 0x811,
            MSR_X2APIC_ISR2 = 0x812,
            MSR_X2APIC_ISR3 = 0x813,
            MSR_X2APIC_ISR4 = 0x814,
            MSR_X2APIC_ISR5 = 0x815,
            MSR_X2APIC_ISR6 = 0x816,
            MSR_X2APIC_ISR7 = 0x817,
            MSR_X2APIC_TMR0 = 0x818,
            MSR_X2APIC_TMR1 = 0x819,
            MSR_X2APIC_TMR2 = 0x81A,
            MSR_X2APIC_TMR3 = 0x81B,
            MSR_X2APIC_TMR4 = 0x81C,
            MSR_X2APIC_TMR5 = 0x81D,
            MSR_X2APIC_TMR6 = 0x81E,
            MSR_X2APIC_TMR7 = 0x81F,
            MSR_X2APIC_IRR0 = 0x820,
            MSR_X2APIC_IRR1 = 0x821,
            MSR_X2APIC_IRR2 = 0x822,
            MSR_X2APIC_IRR3 = 0x823,
            MSR_X2APIC_IRR4 = 0x824,
            MSR_X2APIC_IRR5 = 0x825,
            MSR_X2APIC_IRR6 = 0x826,
            MSR_X2APIC_IRR7 = 0x827,
            MSR_X2APIC_ESR = 0x828,
            MSR_X2APIC_LVT_CMCI = 0x82F,
            MSR_X2APIC_ICR = 0x830,
            MSR_X2APIC_LVT_TIMER = 0x832,
            MSR_X2APIC_LVT_THERMAL = 0x833,
            MSR_X2APIC_LVT_PMI = 0x834,
            MSR_X2APIC_LVT_LINT0 = 0x835,
            MSR_X2APIC_LVT_LINT1 = 0x836,
            MSR_X2APIC_LVT_ERROR = 0x837,
            MSR_X2APIC_INIT_COUNT = 0x838,
            MSR_X2APIC_CUR_COUNT = 0x839,
            MSR_X2APIC_DIV_CONF = 0x83E,
            MSR_X2APIC_SELF_IPI = 0x83F,
            MSR_DEBUG_INTERFACE = 0xC80,
            MSR_L3_QOS_CFG = 0xC81,
            MSR_L2_QOS_CFG = 0xC82,
            MSR_QM_EVTSEL = 0xC8D,
            MSR_QM_CTR = 0xC8E,
            MSR_PQR_ASSOC = 0xC8F,
            MSR_L3_MASK_0 = 0xC90,
            MSR_L2_MASK_0 = 0xD10,
            MSR_BNDCFGS = 0xD90,
            MSR_XSS = 0xDA0,
            MSR_PKG_HDC_CTL = 0xDB0,
            MSR_PM_CTL1 = 0xDB1,
            MSR_THREAD_STALL = 0xDB2,
            /** @brief Extended Feature Enable Register (0xc0000080) */
            MSR_EFER = 0xC0000080,
            /** @brief legacy SYSCALL (0xC0000081) */
            MSR_STAR = 0xC0000081,
            /** @brief 64bit SYSCALL (0xC0000082) */
            MSR_LSTAR = 0xC0000082,
            /** @brief compatibility mode SYSCALL (0xC0000083) */
            MSR_CSTAR = 0xC0000083,
            /** @brief EFLAGS mask for syscall (0xC0000084) */
            MSR_SYSCALL_MASK = 0xC0000084,
            /** @brief 64bit FS base (0xC0000100) */
            MSR_FS_BASE = 0xC0000100,
            /** @brief 64bit GS base (0xC0000101) */
            MSR_GS_BASE = 0xC0000101,
            /** @brief SwapGS GS shadow (0xC0000102) */
            MSR_SHADOW_GS_BASE = 0xC0000102,
            /** @brief Auxiliary TSC (0xC0000103) */
            MSR_TSC_AUX = 0xC0000103,
            MSR_CR_PAT = 0x00000277,
            MSR_CR_PAT_RESET = 0x0007040600070406ULL
        };

        enum ISRExceptions
        {
            DivideByZero = 0x0,
            Debug = 0x1,
            NonMaskableInterrupt = 0x2,
            Breakpoint = 0x3,
            Overflow = 0x4,
            BoundRange = 0x5,
            InvalidOpcode = 0x6,
            DeviceNotAvailable = 0x7,
            DoubleFault = 0x8,
            CoprocessorSegmentOverrun = 0x9,
            InvalidTSS = 0xa,
            SegmentNotPresent = 0xb,
            StackSegmentFault = 0xc,
            GeneralProtectionFault = 0xd,
            PageFault = 0xe,
            x87FloatingPoint = 0x10,
            AlignmentCheck = 0x11,
            MachineCheck = 0x12,
            SIMDFloatingPoint = 0x13,
            Virtualization = 0x14,
            Security = 0x1e
        };

        enum CPUInterrupts
        {
            /* ISR */

            ISR0 = 0x0,   // Divide-by-zero Error
            ISR1 = 0x1,   // Debug
            ISR2 = 0x2,   // Non-maskable Interrupt
            ISR3 = 0x3,   // Breakpoint
            ISR4 = 0x4,   // Overflow
            ISR5 = 0x5,   // Bound Range Exceeded
            ISR6 = 0x6,   // Invalid Opcode
            ISR7 = 0x7,   // Device Not Available
            ISR8 = 0x8,   // Double Fault
            ISR9 = 0x9,   // Coprocessor Segment Overrun
            ISR10 = 0xa,  // Invalid TSS
            ISR11 = 0xb,  // Segment Not P
            ISR12 = 0xc,  // Stack-Segment Fault
            ISR13 = 0xd,  // General Protection Fault
            ISR14 = 0xe,  // Page Fault
            ISR15 = 0xf,  // Reserved
            ISR16 = 0x10, // x87 Floating-Point Exception
            ISR17 = 0x11, // Alignment Check
            ISR18 = 0x12, // Machine Check
            ISR19 = 0x13, // SIMD Floating-Point Exception
            ISR20 = 0x14, // Virtualization Exception
            ISR21 = 0x15, // Reserved
            ISR22 = 0x16, // Reserved
            ISR23 = 0x17, // Reserved
            ISR24 = 0x18, // Reserved
            ISR25 = 0x19, // Reserved
            ISR26 = 0x1a, // Reserved
            ISR27 = 0x1b, // Reserved
            ISR28 = 0x1c, // Reserved
            ISR29 = 0x1d, // Reserved
            ISR30 = 0x1e, // Security Exception
            ISR31 = 0x1f, // Reserved

            /* IRQ */

            IRQ0 = 0x20,  // Programmable Interrupt Timer Interrupt
            IRQ1 = 0x21,  // Keyboard Interrupt
            IRQ2 = 0x22,  // Cascade (used internally by the two PICs. never raised)
            IRQ3 = 0x23,  // COM2 (if enabled)
            IRQ4 = 0x24,  // COM1 (if enabled)
            IRQ5 = 0x25,  // LPT2 (if enabled)
            IRQ6 = 0x26,  // Floppy Disk
            IRQ7 = 0x27,  // LPT1 / Unreliable "spurious" interrupt (usually)
            IRQ8 = 0x28,  // CMOS real-time clock (if enabled)
            IRQ9 = 0x29,  // Free for peripherals / legacy SCSI / NIC
            IRQ10 = 0x2a, // Free for peripherals / SCSI / NIC
            IRQ11 = 0x2b, // Free for peripherals / SCSI / NIC
            IRQ12 = 0x2c, // PS2 Mouse
            IRQ13 = 0x2d, // FPU / Coprocessor / Inter-processor
            IRQ14 = 0x2e, // Primary ATA Hard Disk
            IRQ15 = 0x2f, // Secondary ATA Hard Disk

            /* Reserved by OS */

            IRQ16 = 0x30, // Reserved for multitasking
            IRQ17 = 0x31,
            IRQ18 = 0x32,
            IRQ19 = 0x33,
            IRQ20 = 0x34,
            IRQ21 = 0x35,
            IRQ22 = 0x36,
            IRQ23 = 0x37,
            IRQ24 = 0x38,
            IRQ25 = 0x39,
            IRQ26 = 0x3a,
            IRQ27 = 0x3b,
            IRQ28 = 0x3c,
            IRQ29 = 0x3d,

            /* Free */

            IRQ30 = 0x3e,
            IRQ31 = 0x3f,
            IRQ32 = 0x40,
            IRQ33 = 0x41,
            IRQ34 = 0x42,
            IRQ35 = 0x43,
            IRQ36 = 0x44,
            IRQ37 = 0x45,
            IRQ38 = 0x46,
            IRQ39 = 0x47,
            IRQ40 = 0x48,
            IRQ41 = 0x49,
            IRQ42 = 0x4a,
            IRQ43 = 0x4b,
            IRQ44 = 0x4c,
            IRQ45 = 0x4d,
            IRQ46 = 0x4e,
            IRQ47 = 0x4f,
            IRQ48 = 0x50,
            IRQ49 = 0x51,
            IRQ50 = 0x52,
            IRQ51 = 0x53,
            IRQ52 = 0x54,
            IRQ53 = 0x55,
            IRQ54 = 0x56,
            IRQ55 = 0x57,
            IRQ56 = 0x58,
            IRQ57 = 0x59,
            IRQ58 = 0x5a,
            IRQ59 = 0x5b,
            IRQ60 = 0x5c,
            IRQ61 = 0x5d,
            IRQ62 = 0x5e,
            IRQ63 = 0x5f,
            IRQ64 = 0x60,
            IRQ65 = 0x61,
            IRQ66 = 0x62,
            IRQ67 = 0x63,
            IRQ68 = 0x64,
            IRQ69 = 0x65,
            IRQ70 = 0x66,
            IRQ71 = 0x67,
            IRQ72 = 0x68,
            IRQ73 = 0x69,
            IRQ74 = 0x6a,
            IRQ75 = 0x6b,
            IRQ76 = 0x6c,
            IRQ77 = 0x6d,
            IRQ78 = 0x6e,
            IRQ79 = 0x6f,
            IRQ80 = 0x70,
            IRQ81 = 0x71,
            IRQ82 = 0x72,
            IRQ83 = 0x73,
            IRQ84 = 0x74,
            IRQ85 = 0x75,
            IRQ86 = 0x76,
            IRQ87 = 0x77,
            IRQ88 = 0x78,
            IRQ89 = 0x79,
            IRQ90 = 0x7a,
            IRQ91 = 0x7b,
            IRQ92 = 0x7c,
            IRQ93 = 0x7d,
            IRQ94 = 0x7e,
            IRQ95 = 0x7f,
            IRQ96 = 0x80,
            IRQ97 = 0x81,
            IRQ98 = 0x82,
            IRQ99 = 0x83,
            IRQ100 = 0x84,
            IRQ101 = 0x85,
            IRQ102 = 0x86,
            IRQ103 = 0x87,
            IRQ104 = 0x88,
            IRQ105 = 0x89,
            IRQ106 = 0x8a,
            IRQ107 = 0x8b,
            IRQ108 = 0x8c,
            IRQ109 = 0x8d,
            IRQ110 = 0x8e,
            IRQ111 = 0x8f,
            IRQ112 = 0x90,
            IRQ113 = 0x91,
            IRQ114 = 0x92,
            IRQ115 = 0x93,
            IRQ116 = 0x94,
            IRQ117 = 0x95,
            IRQ118 = 0x96,
            IRQ119 = 0x97,
            IRQ120 = 0x98,
            IRQ121 = 0x99,
            IRQ122 = 0x9a,
            IRQ123 = 0x9b,
            IRQ124 = 0x9c,
            IRQ125 = 0x9d,
            IRQ126 = 0x9e,
            IRQ127 = 0x9f,
            IRQ128 = 0xa0,
            IRQ129 = 0xa1,
            IRQ130 = 0xa2,
            IRQ131 = 0xa3,
            IRQ132 = 0xa4,
            IRQ133 = 0xa5,
            IRQ134 = 0xa6,
            IRQ135 = 0xa7,
            IRQ136 = 0xa8,
            IRQ137 = 0xa9,
            IRQ138 = 0xaa,
            IRQ139 = 0xab,
            IRQ140 = 0xac,
            IRQ141 = 0xad,
            IRQ142 = 0xae,
            IRQ143 = 0xaf,
            IRQ144 = 0xb0,
            IRQ145 = 0xb1,
            IRQ146 = 0xb2,
            IRQ147 = 0xb3,
            IRQ148 = 0xb4,
            IRQ149 = 0xb5,
            IRQ150 = 0xb6,
            IRQ151 = 0xb7,
            IRQ152 = 0xb8,
            IRQ153 = 0xb9,
            IRQ154 = 0xba,
            IRQ155 = 0xbb,
            IRQ156 = 0xbc,
            IRQ157 = 0xbd,
            IRQ158 = 0xbe,
            IRQ159 = 0xbf,
            IRQ160 = 0xc0,
            IRQ161 = 0xc1,
            IRQ162 = 0xc2,
            IRQ163 = 0xc3,
            IRQ164 = 0xc4,
            IRQ165 = 0xc5,
            IRQ166 = 0xc6,
            IRQ167 = 0xc7,
            IRQ168 = 0xc8,
            IRQ169 = 0xc9,
            IRQ170 = 0xca,
            IRQ171 = 0xcb,
            IRQ172 = 0xcc,
            IRQ173 = 0xcd,
            IRQ174 = 0xce,
            IRQ175 = 0xcf,
            IRQ176 = 0xd0,
            IRQ177 = 0xd1,
            IRQ178 = 0xd2,
            IRQ179 = 0xd3,
            IRQ180 = 0xd4,
            IRQ181 = 0xd5,
            IRQ182 = 0xd6,
            IRQ183 = 0xd7,
            IRQ184 = 0xd8,
            IRQ185 = 0xd9,
            IRQ186 = 0xda,
            IRQ187 = 0xdb,
            IRQ188 = 0xdc,
            IRQ189 = 0xdd,
            IRQ190 = 0xde,
            IRQ191 = 0xdf,
            IRQ192 = 0xe0,
            IRQ193 = 0xe1,
            IRQ194 = 0xe2,
            IRQ195 = 0xe3,
            IRQ196 = 0xe4,
            IRQ197 = 0xe5,
            IRQ198 = 0xe6,
            IRQ199 = 0xe7,
            IRQ200 = 0xe8,
            IRQ201 = 0xe9,
            IRQ202 = 0xea,
            IRQ203 = 0xeb,
            IRQ204 = 0xec,
            IRQ205 = 0xed,
            IRQ206 = 0xee,
            IRQ207 = 0xef,
            IRQ208 = 0xf0,
            IRQ209 = 0xf1,
            IRQ210 = 0xf2,
            IRQ211 = 0xf3,
            IRQ212 = 0xf4,
            IRQ213 = 0xf5,
            IRQ214 = 0xf6,
            IRQ215 = 0xf7,
            IRQ216 = 0xf8,
            IRQ217 = 0xf9,
            IRQ218 = 0xfa,
            IRQ219 = 0xfb,
            IRQ220 = 0xfc,
            IRQ221 = 0xfd,
            IRQ222 = 0xfe,
            IRQ223 = 0xff,
        };

        /**
         * @brief MSR_APIC_BASE structure
         * @see MSR_APIC_BASE
         */
        typedef union
        {
            struct
            {
                /** @brief Reserved */
                uint32_t Reserved0 : 8;
                /**
                 * @brief BSP Flag
                 * @details If the BSP flag is set to 1, the processor is the bootstrap processor.
                 */
                uint32_t BSP : 1;
                /** @brief Reserved */
                uint32_t Reserved1 : 1;
                /** @brief Enable x2APIC mode */
                uint32_t EXTD : 1;
                /** @brief APIC Global Enable */
                uint32_t EN : 1;
                /** @brief APIC Base Low Address */
                uint32_t ApicBaseLo : 20;
                /** @brief APIC Base High Address */
                uint32_t ApicBaseHi : 32;
            };
            uint64_t raw;
        } __attribute__((packed)) APIC_BASE;

        typedef union
        {
            struct
            {
                /** @brief Carry Flag */
                uint64_t CF : 1;
                /** @brief Reserved */
                uint64_t AlwaysOne : 1;
                /** @brief Parity Flag */
                uint64_t PF : 1;
                /** @brief Reserved */
                uint64_t Reserved0 : 1;
                /** @brief Auxiliary Carry Flag */
                uint64_t AF : 1;
                /** @brief Reserved */
                uint64_t Reserved1 : 1;
                /** @brief Zero Flag */
                uint64_t ZF : 1;
                /** @brief Sign Flag */
                uint64_t SF : 1;
                /** @brief Trap Flag */
                uint64_t TF : 1;
                /** @brief Interrupt Enable Flag */
                uint64_t IF : 1;
                /** @brief Direction Flag */
                uint64_t DF : 1;
                /** @brief Overflow Flag */
                uint64_t OF : 1;
                /** @brief I/O Privilege Level */
                uint64_t IOPL : 2;
                /** @brief Nested Task */
                uint64_t NT : 1;
                /** @brief Reserved */
                uint64_t Reserved2 : 1;
                /** @brief Resume Flag */
                uint64_t RF : 1;
                /** @brief Virtual 8086 Mode */
                uint64_t VM : 1;
                /** @brief Alignment Check */
                uint64_t AC : 1;
                /** @brief Virtual Interrupt Flag */
                uint64_t VIF : 1;
                /** @brief Virtual Interrupt Pending */
                uint64_t VIP : 1;
                /** @brief ID Flag */
                uint64_t ID : 1;
                /** @brief Reserved */
                uint64_t Reserved3 : 10;
            };
            uint64_t raw;
        } RFLAGS;

        typedef struct TrapFrame
        {
            // uint64_t gs;  // General-purpose Segment
            // uint64_t fs;  // General-purpose Segment
            // uint64_t es;  // Extra Segment (used for string operations)
            uint64_t ds; // Data Segment

            uint64_t r15; // General purpose
            uint64_t r14; // General purpose
            uint64_t r13; // General purpose
            uint64_t r12; // General purpose
            uint64_t r11; // General purpose
            uint64_t r10; // General purpose
            uint64_t r9;  // General purpose
            uint64_t r8;  // General purpose

            uint64_t rbp; // Base Pointer (meant for stack frames)
            uint64_t rdi; // Destination index for string operations
            uint64_t rsi; // Source index for string operations
            uint64_t rdx; // Data (commonly extends the A register)
            uint64_t rcx; // Counter
            uint64_t rbx; // Base
            uint64_t rax; // Accumulator

            uint64_t InterruptNumber; // Interrupt Number
            uint64_t ErrorCode;       // Error code
            uint64_t rip;             // Instruction Pointer
            uint64_t cs;              // Code Segment
            RFLAGS rflags;            // Register Flags
            uint64_t rsp;             // Stack Pointer
            uint64_t ss;              // Stack Segment
        } TrapFrame;

        typedef union CR0
        {
            struct
            {
                /** @brief Protection Enable */
                uint64_t PE : 1;
                /** @brief Monitor Coprocessor */
                uint64_t MP : 1;
                /** @brief Emulation */
                uint64_t EM : 1;
                /** @brief Task Switched */
                uint64_t TS : 1;
                /** @brief Extension Type */
                uint64_t ET : 1;
                /** @brief Numeric Error */
                uint64_t NE : 1;
                /** @brief Reserved */
                uint64_t Reserved0 : 10;
                /** @brief Write Protect */
                uint64_t WP : 1;
                /** @brief Reserved */
                uint64_t Reserved1 : 1;
                /** @brief Alignment Mask */
                uint64_t AM : 1;
                /** @brief Reserved */
                uint64_t Reserved2 : 10;
                /** @brief Mot Write-through */
                uint64_t NW : 1;
                /** @brief Cache Disable */
                uint64_t CD : 1;
                /** @brief Paging */
                uint64_t PG : 1;
            };
            uint64_t raw;
        } CR0;

        typedef union CR2
        {
            struct
            {
                /** @brief Page Fault Linear Address */
                uint64_t PFLA;
            };
            uint64_t raw;
        } CR2;

        typedef union CR3
        {
            struct
            {
                /** @brief Not used if bit 17 of CR4 is 1 */
                uint64_t PWT : 1;
                /** @brief Not used if bit 17 of CR4 is 1 */
                uint64_t PCD : 1;
                /** @brief Base of PML4T/PML5T */
                uint64_t PDBR;
            };
            uint64_t raw;
        } CR3;

        typedef union CR4
        {
            struct
            {
                /** @brief Virtual-8086 Mode Extensions */
                uint64_t VME : 1;
                /** @brief Protected-Mode Virtual Interrupts */
                uint64_t PVI : 1;
                /** @brief Time Stamp Disable */
                uint64_t TSD : 1;
                /** @brief Debugging Extensions */
                uint64_t DE : 1;
                /** @brief Page Size Extensions */
                uint64_t PSE : 1;
                /** @brief Physical Address Extension */
                uint64_t PAE : 1;
                /** @brief Machine Check Enable */
                uint64_t MCE : 1;
                /** @brief Page Global Enable */
                uint64_t PGE : 1;
                /** @brief Performance Monitoring Counter */
                uint64_t PCE : 1;
                /** @brief Operating System Support */
                uint64_t OSFXSR : 1;
                /** @brief Operating System Support */
                uint64_t OSXMMEXCPT : 1;
                /** @brief User-Mode Instruction Prevention */
                uint64_t UMIP : 1;
                /** @brief Linear Address 57bit */
                uint64_t LA57 : 1;
                /** @brief VMX Enable */
                uint64_t VMXE : 1;
                /** @brief SMX Enable */
                uint64_t SMXE : 1;
                /** @brief Reserved */
                uint64_t Reserved0 : 1;
                /** @brief FSGSBASE Enable */
                uint64_t FSGSBASE : 1;
                /** @brief PCID Enable */
                uint64_t PCIDE : 1;
                /** @brief XSAVE and Processor Extended States Enable */
                uint64_t OSXSAVE : 1;
                /** @brief Reserved */
                uint64_t Reserved1 : 1;
                /** @brief SMEP Enable */
                uint64_t SMEP : 1;
                /** @brief SMAP Enable */
                uint64_t SMAP : 1;
                /** @brief Protection-Key Enable */
                uint64_t PKE : 1;
                /** @brief Reserved */
                uint64_t Reserved2 : 9;
            };
            uint64_t raw;
        } CR4;

        typedef union CR8
        {
            struct
            {
                /** @brief Task Priority Level */
                uint64_t TPL : 1;
            };
            uint64_t raw;
        } CR8;

        typedef union EFER
        {
            struct
            {
                /** @brief Enable syscall & sysret instructions in 64-bit mode. */
                uint64_t SCE : 1;
                /** @brief Reserved */
                uint64_t Reserved0 : 7;
                /** @brief Enable long mode. */
                uint64_t LME : 1;
                /** @brief Reserved */
                uint64_t Reserved1 : 1;
                /** @brief Indicates long. */
                uint64_t LMA : 1;
                /** @brief Enable No-Execute Bit */
                uint64_t NXE : 1;
                /** @brief Enable Secure Virtual Machine */
                uint64_t SVME : 1;
                /** @brief Enable Long Mode Segment Limit */
                uint64_t LMSLE : 1;
                /** @brief Enable Fast FXSAVE/FXRSTOR */
                uint64_t FFXSR : 1;
                /** @brief Enable Translation Cache Extension */
                uint64_t TCE : 1;
                /** @brief Reserved */
                uint64_t Reserved2 : 32;
            };
            uint64_t raw;
        } __attribute__((packed)) EFER;

        // ! TODO: UNTESTED!
        typedef union DR7
        {
            struct
            {
                /** @brief Local DR0 Breakpoint (0) */
                uint64_t LocalDR0 : 1;
                /** @brief Global DR0 Breakpoint (1) */
                uint64_t GlobalDR0 : 1;
                /** @brief Local DR1 Breakpoint (2) */
                uint64_t LocalDR1 : 1;
                /** @brief Global DR1 Breakpoint (3) */
                uint64_t GlobalDR1 : 1;
                /** @brief Local DR2 Breakpoint (4) */
                uint64_t LocalDR2 : 1;
                /** @brief Global DR2 Breakpoint (5) */
                uint64_t GlobalDR2 : 1;
                /** @brief Local DR3 Breakpoint (6) */
                uint64_t LocalDR3 : 1;
                /** @brief Global DR3 Breakpoint (7) */
                uint64_t GlobalDR3 : 1;
                /** @brief Reserved [7 - (16-17)] */
                uint64_t Reserved : 9;
                /** @brief Conditions for DR0 (16-17) */
                uint64_t ConditionsDR0 : 1;
                /** @brief Size of DR0 Breakpoint (18-19) */
                uint64_t SizeDR0 : 1;
                /** @brief Conditions for DR1 (20-21) */
                uint64_t ConditionsDR1 : 1;
                /** @brief Size of DR1 Breakpoint (22-23) */
                uint64_t SizeDR1 : 1;
                /** @brief Conditions for DR2 (24-25) */
                uint64_t ConditionsDR2 : 1;
                /** @brief Size of DR2 Breakpoint (26-27) */
                uint64_t SizeDR2 : 1;
                /** @brief Conditions for DR3 (28-29) */
                uint64_t ConditionsDR3 : 1;
                /** @brief Size of DR3 Breakpoint (30-31) */
                uint64_t SizeDR3 : 1;
            };
            uint64_t raw;
        } DR7;

        typedef union PageFaultErrorCode
        {
            struct
            {
                /** @brief When set, the page fault was caused by a page-protection violation. When not set, it was caused by a non-present page. */
                uint64_t P : 1;
                /** @brief When set, the page fault was caused by a write access. When not set, it was caused by a read access. */
                uint64_t W : 1;
                /** @brief When set, the page fault was caused while CPL = 3. This does not necessarily mean that the page fault was a privilege violation. */
                uint64_t U : 1;
                /** @brief When set, one or more page directory entries contain reserved bits which are set to 1. This only applies when the PSE or PAE flags in CR4 are set to 1. */
                uint64_t R : 1;
                /** @brief When set, the page fault was caused by an instruction fetch. This only applies when the No-Execute bit is supported and enabled. */
                uint64_t I : 1;
                /** @brief When set, the page fault was caused by a protection-key violation. The PKRU register (for user-mode accesses) or PKRS MSR (for supervisor-mode accesses) specifies the protection key rights. */
                uint64_t PK : 1;
                /** @brief When set, the page fault was caused by a shadow stack access. */
                uint64_t SS : 1;
                /** @brief Reserved */
                uint64_t Reserved0 : 8;
                /** @brief When set, the fault was due to an SGX violation. The fault is unrelated to ordinary paging. */
                uint64_t SGX : 1;
                /** @brief Reserved */
                uint64_t Reserved1 : 16;
            };
            uint64_t raw;
        } PageFaultErrorCode;

        // ! TODO: UNTESTED!
        typedef union SelectorErrorCode
        {
            struct
            {
                /** @brief When set, the exception originated externally to the processor. */
                uint64_t External : 1;
                /** @brief IDT/GDT/LDT Table
                 *  @details 0b00 - The Selector Index references a descriptor in the GDT.
                 *  @details 0b01 - The Selector Index references a descriptor in the IDT.
                 *  @details 0b10 - The Selector Index references a descriptor in the LDT.
                 *  @details 0b11 - The Selector Index references a descriptor in the IDT.
                 */
                uint64_t Table : 2;
                /** @brief The index in the GDT, IDT or LDT. */
                uint64_t Idx : 13;
                /** @brief Reserved */
                uint64_t Reserved : 16;
            };
            uint64_t raw;
        } SelectorErrorCode;

        static inline void lgdt(void *gdt)
        {
#if defined(__amd64__)
            asmv("lgdt (%0)"
                 :
                 : "r"(gdt));
#endif
        }

        static inline void lidt(void *idt)
        {
#if defined(__amd64__)
            asmv("lidt (%0)"
                 :
                 : "r"(idt));
#endif
        }

        static inline void ltr(uint16_t Segment)
        {
#if defined(__amd64__)
            asmv("ltr %0"
                 :
                 : "r"(Segment));
#endif
        }

        static inline void invlpg(void *Address)
        {
#if defined(__amd64__)
            asmv("invlpg (%0)"
                 :
                 : "r"(Address)
                 : "memory");
#endif
        }

        /**
         * @brief CPUID
         *
         * @param Function Leaf
         * @param eax EAX
         * @param ebx EBX
         * @param ecx ECX
         * @param edx EDX
         */
        static inline void cpuid(uint32_t Function, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
        {
#if defined(__amd64__)
            asmv("cpuid"
                 : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                 : "a"(Function));
#endif
        }

        /**
         * @brief Get the highest leaf function supported by CPUID
         *
         * @example if (GetHighestLeaf() < 0x15) { error("CPU doesn't support leaf 0x15!"); }
         *
         * @return uint32_t
         */
        static inline uint32_t GetHighestLeaf()
        {
            uint32_t eax, ebx, ecx, edx;
            cpuid(0x0, &eax, &ebx, &ecx, &edx);
            return eax;
        }

        static inline uint64_t rdmsr(uint32_t msr)
        {
            uint32_t Low, High;
#if defined(__amd64__)
            asmv("rdmsr"
                 : "=a"(Low), "=d"(High)
                 : "c"(msr)
                 : "memory");
#endif
            return ((uint64_t)Low) | (((uint64_t)High) << 32);
        }

        static inline void wrmsr(uint32_t msr, uint64_t Value)
        {
            uint32_t Low = Value, High = Value >> 32;
#if defined(__amd64__)
            asmv("wrmsr"
                 :
                 : "c"(msr), "a"(Low), "d"(High)
                 : "memory");
#endif
        }

        static inline CR0 readcr0()
        {
            uint64_t Result;
#if defined(__amd64__)
            asmv("mov %%cr0, %[Result]"
                 : [Result] "=q"(Result));
#endif
            return (CR0){.raw = Result};
        }

        static inline CR2 readcr2()
        {
            uint64_t Result;
#if defined(__amd64__)
            asmv("mov %%cr2, %[Result]"
                 : [Result] "=q"(Result));
#endif
            return (CR2){.raw = Result};
        }

        static inline CR3 readcr3()
        {
            uint64_t Result;
#if defined(__amd64__)
            asmv("mov %%cr3, %[Result]"
                 : [Result] "=q"(Result));
#endif
            return (CR3){.raw = Result};
        }

        static inline CR4 readcr4()
        {
            uint64_t Result;
#if defined(__amd64__)
            asmv("mov %%cr4, %[Result]"
                 : [Result] "=q"(Result));
#endif
            return (CR4){.raw = Result};
        }

        static inline CR8 readcr8()
        {
            uint64_t Result;
#if defined(__amd64__)
            asmv("mov %%cr8, %[Result]"
                 : [Result] "=q"(Result));
#endif
            return (CR8){.raw = Result};
        }

        static inline void writecr0(CR0 ControlRegister)
        {
#if defined(__amd64__)
            asmv("mov %[ControlRegister], %%cr0"
                 :
                 : [ControlRegister] "q"(ControlRegister.raw)
                 : "memory");
#endif
        }

        static inline void writecr2(CR2 ControlRegister)
        {
#if defined(__amd64__)
            asmv("mov %[ControlRegister], %%cr2"
                 :
                 : [ControlRegister] "q"(ControlRegister.raw)
                 : "memory");
#endif
        }

        static inline void writecr3(CR3 ControlRegister)
        {
#if defined(__amd64__)
            asmv("mov %[ControlRegister], %%cr3"
                 :
                 : [ControlRegister] "q"(ControlRegister.raw)
                 : "memory");
#endif
        }

        static inline void writecr4(CR4 ControlRegister)
        {
#if defined(__amd64__)
            asmv("mov %[ControlRegister], %%cr4"
                 :
                 : [ControlRegister] "q"(ControlRegister.raw)
                 : "memory");
#endif
        }

        static inline void writecr8(CR8 ControlRegister)
        {
#if defined(__amd64__)
            asmv("mov %[ControlRegister], %%cr8"
                 :
                 : [ControlRegister] "q"(ControlRegister.raw)
                 : "memory");
#endif
        }

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
                        uint64_t BranchID : 16;
                        uint64_t Reserved0 : 16;
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
                        uint64_t Reserved0 : 1;
                        uint64_t AltMovCr8 : 1;
                        uint64_t Reserved1 : 26;
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

#endif // !__FENNIX_KERNEL_CPU_H__
