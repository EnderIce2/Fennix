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
     * @brief Stop the CPU (infinite loop)
     */
    void Stop();

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
        typedef enum
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
        } ISRExceptions;

        typedef enum
        {
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

            IRQ0 = 0x20,  //	Programmable Interrupt Timer Interrupt
            IRQ1 = 0x21,  //	Keyboard Interrupt
            IRQ2 = 0x22,  //	Cascade (used internally by the two PICs. never raised)
            IRQ3 = 0x23,  //	COM2 (if enabled)
            IRQ4 = 0x24,  //	COM1 (if enabled)
            IRQ5 = 0x25,  //	LPT2 (if enabled)
            IRQ6 = 0x26,  //	Floppy Disk
            IRQ7 = 0x27,  //	LPT1 / Unreliable "spurious" interrupt (usually)
            IRQ8 = 0x28,  //	CMOS real-time clock (if enabled)
            IRQ9 = 0x29,  //	Free for peripherals / legacy SCSI / NIC
            IRQ10 = 0x2a, //	Free for peripherals / SCSI / NIC
            IRQ11 = 0x2b, //	Free for peripherals / SCSI / NIC
            IRQ12 = 0x2c, //	PS2 Mouse
            IRQ13 = 0x2d, //	FPU / Coprocessor / Inter-processor
            IRQ14 = 0x2e, //	Primary ATA Hard Disk
            IRQ15 = 0x2f, //	Secondary ATA Hard Disk

            IRQ16 = 0x30, //    Reserved for multitasking
            IRQ17 = 0x31, //    Reserved for monotasking

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
        } Interrupts;

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
