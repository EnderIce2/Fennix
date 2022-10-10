#ifndef __FENNIX_KERNEL_CPU_H__
#define __FENNIX_KERNEL_CPU_H__

#include <types.h>

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
    void Pause();

    /**
     * @brief Halt the CPU
     */
    void Halt();

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

    namespace MemBar
    {
        static inline void Barrier()
        {
#if defined(__amd64__) || defined(__i386__)
            asmv("" ::
                     : "memory");
#elif defined(__aarch64__)
            asmv("dmb ish" ::
                     : "memory");
#endif
        }

        static inline void Fence()
        {
#if defined(__amd64__) || defined(__i386__)
            asmv("mfence" ::
                     : "memory");
#elif defined(__aarch64__)
            asmv("dmb ish" ::
                     : "memory");
#endif
        }

        static inline void StoreFence()
        {
#if defined(__amd64__) || defined(__i386__)
            asmv("sfence" ::
                     : "memory");
#elif defined(__aarch64__)
            asmv("dmb ishst" ::
                     : "memory");
#endif
        }

        static inline void LoadFence()
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
        typedef union
        {
            struct
            {
                /** @brief Carry Flag */
                uint64_t CF : 1;
                /** @brief Reserved */
                uint64_t always_one : 1;
                /** @brief Parity Flag */
                uint64_t PF : 1;
                /** @brief Reserved */
                uint64_t _reserved0 : 1;
                /** @brief Auxiliary Carry Flag */
                uint64_t AF : 1;
                /** @brief Reserved */
                uint64_t _reserved1 : 1;
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
                uint64_t _reserved2 : 1;
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
                uint64_t _reserved3 : 10;
            };
            uint64_t raw;
        } RFLAGS;

        typedef struct _TrapFrame
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

            uint64_t int_num;    // Interrupt Number
            uint64_t error_code; // Error code
            uint64_t rip;        // Instruction Pointer
            uint64_t cs;         // Code Segment
            RFLAGS rflags;       // Register Flags
            uint64_t rsp;        // Stack Pointer
            uint64_t ss;         // Stack Segment
        } TrapFrame;

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

        static inline void cpuid(uint32_t Function, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
        {
#if defined(__amd64__)
            asmv("cpuid"
                 : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                 : "a"(Function));
#endif
        }
    }
}

#endif // !__FENNIX_KERNEL_CPU_H__
