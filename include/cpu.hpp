#ifndef __FENNIX_KERNEL_CPU_H__
#define __FENNIX_KERNEL_CPU_H__

#include <types.h>

#include <cstring>
#include <cpu/x86/x32/cpuid_amd.hpp>
#include <cpu/x86/x32/cpuid_intel.hpp>
#include <cpu/x86/x32/cr.hpp>
#include <cpu/x86/x32/msr.hpp>
#include <cpu/x86/x64/cpuid_amd.hpp>
#include <cpu/x86/x64/cpuid_intel.hpp>
#include <cpu/x86/x64/cr.hpp>
#include <cpu/x86/x64/msr.hpp>
#include <cpu/x86/exceptions.hpp>
#include <cpu/x86/interrupts.hpp>
#include <cpu/signatures.hpp>
#include <cpu/membar.hpp>

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

    enum x86SIMDType
    {
        SIMD_NONE,
        SIMD_SSE,
        SIMD_SSE2,
        SIMD_SSE3,
        SIMD_SSSE3,
        SIMD_SSE41,
        SIMD_SSE42,
        SIMD_AVX,
        SIMD_AVX2,
        SIMD_AVX512F,
        SIMD_AVX512BW,
        SIMD_AVX512CD,
        SIMD_AVX512DQ,
        SIMD_AVX512ER,
        SIMD_AVX512IFMA,
        SIMD_AVX512PF,
        SIMD_AVX512VBMI,
        SIMD_AVX512VL,
        SIMD_AVX512VNNI,
        SIMD_AVX512BITALG,
        SIMD_AVX512VPOPCNTDQ,
        SIMD_AVX512_4VNNIW,
        SIMD_AVX512_4FMAPS,
        SIMD_AVX512_VP2INTERSECT,
        SIMD_AVX512_BF16,
        SIMD_AVX512_VBMI2,
        SIMD_AVX512_GFNI,
        SIMD_AVX512_VAES,
        SIMD_AVX512_VPCLMULQDQ,
        SIMD_AVX512_VNNI,
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
     * @brief Check SIMD support. It will return the highest supported SIMD type.
     *
     * @return x86SIMDType
     */
    x86SIMDType CheckSIMD();

    /**
     * @brief Check SIMD support.
     *
     * @param Type SIMD type.
     * @return true if supported.
     * @return false if not supported.
     */
    bool CheckSIMD(x86SIMDType Type);

    /**
     * @brief Pause the CPU
     */
    SafeFunction static inline void Pause(bool Loop = false)
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
    SafeFunction static inline void Stop()
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
    SafeFunction static inline void Halt(bool Loop = false)
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
    void InitializeFeatures(long Core);

    /** @brief Get CPU counter value. */
    uintptr_t Counter();

    namespace x32
    {
        typedef union
        {
            struct
            {
                /** @brief Carry Flag */
                uint32_t CF : 1;
                /** @brief Reserved */
                uint32_t AlwaysOne : 1;
                /** @brief Parity Flag */
                uint32_t PF : 1;
                /** @brief Reserved */
                uint32_t Reserved0 : 1;
                /** @brief Auxiliary Carry Flag */
                uint32_t AF : 1;
                /** @brief Reserved */
                uint32_t Reserved1 : 1;
                /** @brief Zero Flag */
                uint32_t ZF : 1;
                /** @brief Sign Flag */
                uint32_t SF : 1;
                /** @brief Trap Flag */
                uint32_t TF : 1;
                /** @brief Interrupt Enable Flag */
                uint32_t IF : 1;
                /** @brief Direction Flag */
                uint32_t DF : 1;
                /** @brief Overflow Flag */
                uint32_t OF : 1;
                /** @brief I/O Privilege Level */
                uint32_t IOPL : 2;
                /** @brief Nested Task */
                uint32_t NT : 1;
                /** @brief Reserved */
                uint32_t Reserved2 : 1;
                /** @brief Resume Flag */
                uint32_t RF : 1;
                /** @brief Virtual 8086 Mode */
                uint32_t VM : 1;
                /** @brief Alignment Check */
                uint32_t AC : 1;
                /** @brief Virtual Interrupt Flag */
                uint32_t VIF : 1;
                /** @brief Virtual Interrupt Pending */
                uint32_t VIP : 1;
                /** @brief ID Flag */
                uint32_t ID : 1;
            };
            uint32_t raw;
        } EFLAGS;

        typedef struct TrapFrame
        {
            uint32_t ebp; // Base Pointer (meant for stack frames)
            uint32_t edi; // Destination index for string operations
            uint32_t esi; // Source index for string operations
            uint32_t edx; // Data (commonly extends the A register)
            uint32_t ecx; // Counter
            uint32_t ebx; // Base
            uint32_t eax; // Accumulator

            uint32_t InterruptNumber; // Interrupt Number
            uint32_t ErrorCode;       // Error code
            uint32_t eip;             // Instruction Pointer
            uint32_t cs;              // Code Segment
            EFLAGS eflags;            // Register Flags
            uint32_t esp;             // Stack Pointer
            uint32_t ss;              // Stack Segment
        } TrapFrame;

        /* TODO: Does EFER exists in x32? */
        typedef union EFER
        {
            struct
            {
                /** @brief Enable syscall & sysret instructions in 64-bit mode. */
                uint32_t SCE : 1;
                /** @brief Reserved */
                uint32_t Reserved0 : 7;
                /** @brief Enable long mode. */
                uint32_t LME : 1;
                /** @brief Reserved */
                uint32_t Reserved1 : 1;
                /** @brief Indicates long. */
                uint32_t LMA : 1;
                /** @brief Enable No-Execute Bit */
                uint32_t NXE : 1;
                /** @brief Enable Secure Virtual Machine */
                uint32_t SVME : 1;
                /** @brief Enable Long Mode Segment Limit */
                uint32_t LMSLE : 1;
                /** @brief Enable Fast FXSAVE/FXRSTOR */
                uint32_t FFXSR : 1;
                /** @brief Enable Translation Cache Extension */
                uint32_t TCE : 1;
                /** @brief Reserved */
                uint32_t Reserved2 : 32;
            };
            uint32_t raw;
        } __attribute__((packed)) EFER;

        // ! TODO: UNTESTED!
        typedef union DR7
        {
            struct
            {
                /** @brief Local DR0 Breakpoint (0) */
                uint32_t LocalDR0 : 1;
                /** @brief Global DR0 Breakpoint (1) */
                uint32_t GlobalDR0 : 1;
                /** @brief Local DR1 Breakpoint (2) */
                uint32_t LocalDR1 : 1;
                /** @brief Global DR1 Breakpoint (3) */
                uint32_t GlobalDR1 : 1;
                /** @brief Local DR2 Breakpoint (4) */
                uint32_t LocalDR2 : 1;
                /** @brief Global DR2 Breakpoint (5) */
                uint32_t GlobalDR2 : 1;
                /** @brief Local DR3 Breakpoint (6) */
                uint32_t LocalDR3 : 1;
                /** @brief Global DR3 Breakpoint (7) */
                uint32_t GlobalDR3 : 1;
                /** @brief Reserved [7 - (16-17)] */
                uint32_t Reserved : 9;
                /** @brief Conditions for DR0 (16-17) */
                uint32_t ConditionsDR0 : 1;
                /** @brief Size of DR0 Breakpoint (18-19) */
                uint32_t SizeDR0 : 1;
                /** @brief Conditions for DR1 (20-21) */
                uint32_t ConditionsDR1 : 1;
                /** @brief Size of DR1 Breakpoint (22-23) */
                uint32_t SizeDR1 : 1;
                /** @brief Conditions for DR2 (24-25) */
                uint32_t ConditionsDR2 : 1;
                /** @brief Size of DR2 Breakpoint (26-27) */
                uint32_t SizeDR2 : 1;
                /** @brief Conditions for DR3 (28-29) */
                uint32_t ConditionsDR3 : 1;
                /** @brief Size of DR3 Breakpoint (30-31) */
                uint32_t SizeDR3 : 1;
            };
            uint32_t raw;
        } DR7;

        struct FXState
        {
            /** @brief FPU control word */
            uint16_t fcw;
            /** @brief FPU status word */
            uint16_t fsw;
            /** @brief FPU tag words */
            uint8_t ftw;
            /** @brief Reserved (zero) */
            uint8_t Reserved;
            /** @brief FPU opcode */
            uint16_t fop;
            /** @brief PFU instruction pointer */
            uint64_t rip;
            /** @brief FPU data pointer */
            uint64_t rdp;
            /** @brief SSE control register */
            uint32_t mxcsr;
            /** @brief SSE control register mask */
            uint32_t mxcsrmask;
            /** @brief FPU registers (last 6 bytes reserved) */
            uint8_t st[8][16];
            /** @brief XMM registers */
            uint8_t xmm[16][16];
        } __attribute__((packed));

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
#if defined(__i386__)
            asmv("cpuid"
                 : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                 : "a"(Function));
#else
            UNUSED(Function);
            UNUSED(eax);
            UNUSED(ebx);
            UNUSED(ecx);
            UNUSED(edx);
#endif
        }

        SafeFunction static inline void invlpg(void *Address)
        {
#if defined(__i386__)
            asmv("invlpg (%0)"
                 :
                 : "r"(Address)
                 : "memory");
#else
            UNUSED(Address);
#endif
        }
    }

    namespace x64
    {
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

        struct FXState
        {
            /** @brief FPU control word */
            uint16_t fcw;
            /** @brief FPU status word */
            uint16_t fsw;
            /** @brief FPU tag words */
            uint8_t ftw;
            /** @brief Reserved (zero) */
            uint8_t Reserved;
            /** @brief FPU opcode */
            uint16_t fop;
            /** @brief PFU instruction pointer */
            uint64_t rip;
            /** @brief FPU data pointer */
            uint64_t rdp;
            /** @brief SSE control register */
            uint32_t mxcsr;
            /** @brief SSE control register mask */
            uint32_t mxcsrmask;
            /** @brief FPU registers (last 6 bytes reserved) */
            uint8_t st[8][16];
            /** @brief XMM registers */
            uint8_t xmm[16][16];
        } __attribute__((packed));

        SafeFunction static inline void lgdt(void *gdt)
        {
#if defined(__amd64__)
            asmv("lgdt (%0)"
                 :
                 : "r"(gdt));
#endif
        }

        SafeFunction static inline void lidt(void *idt)
        {
#if defined(__amd64__)
            asmv("lidt (%0)"
                 :
                 : "r"(idt));
#endif
        }

        SafeFunction static inline void ltr(uint16_t Segment)
        {
#if defined(__amd64__)
            asmv("ltr %0"
                 :
                 : "r"(Segment));
#endif
        }

        SafeFunction static inline void invlpg(void *Address)
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
        SafeFunction static inline void cpuid(uint32_t Function, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
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
        SafeFunction static inline uint32_t GetHighestLeaf()
        {
            uint32_t eax, ebx, ecx, edx;
            cpuid(0x0, &eax, &ebx, &ecx, &edx);
            return eax;
        }

        SafeFunction static inline void fxsave(void *FXSaveArea)
        {
#if defined(__amd64__)
            if (!FXSaveArea || FXSaveArea >= (char *)0xfffffffffffff000)
                return;

            asmv("fxsaveq (%0)"
                 :
                 : "r"(FXSaveArea)
                 : "memory");
#endif
        }

        SafeFunction static inline void fxrstor(void *FXRstorArea)
        {
#if defined(__amd64__)
            if (!FXRstorArea || FXRstorArea >= (char *)0xfffffffffffff000)
                return;

            asmv("fxrstorq (%0)"
                 :
                 : "r"(FXRstorArea)
                 : "memory");
#endif
        }
    }

    namespace aarch64
    {
    }
}

#endif // !__FENNIX_KERNEL_CPU_H__
