#ifndef __FENNIX_KERNEL_TASKING_H__
#define __FENNIX_KERNEL_TASKING_H__

#include <types.h>

#include <vector.hpp>
#include <memory.hpp>

namespace Tasking
{
    typedef unsigned long IP;
    typedef unsigned long IPOffset;
    typedef unsigned long UPID;
    typedef unsigned long UTID;
    typedef unsigned long Token;

    struct ThreadFrame
    {
#if defined(__amd64__)
        // uint64_t gs;         // General-purpose Segment
        // uint64_t fs;         // General-purpose Segment
        // uint64_t es;         // Extra Segment (used for string operations)
        uint64_t ds;         // Data Segment
        uint64_t r15;        // General purpose
        uint64_t r14;        // General purpose
        uint64_t r13;        // General purpose
        uint64_t r12;        // General purpose
        uint64_t r11;        // General purpose
        uint64_t r10;        // General purpose
        uint64_t r9;         // General purpose
        uint64_t r8;         // General purpose
        uint64_t rbp;        // Base Pointer (meant for stack frames)
        uint64_t rdi;        // First Argument
        uint64_t rsi;        // Second Argument
        uint64_t rdx;        // Data (commonly extends the A register)
        uint64_t rcx;        // Counter
        uint64_t rbx;        // Base
        uint64_t rax;        // Accumulator
        uint64_t int_num;    // Interrupt Number
        uint64_t error_code; // Error code
        uint64_t rip;        // Instruction Pointer
        uint64_t cs;         // Code Segment
        union
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
        } rflags;     // Register Flags
        uint64_t rsp; // Stack Pointer
        uint64_t ss;  // Stack Segment / Data Segment
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
    };

    enum TaskArchitecture
    {
        UnknownArchitecture,
        x86,
        x64,
        ARM,
        ARM64
    };

    enum TaskPlatform
    {
        UnknownPlatform,
        Native,
        Linux,
        Windows
    };

    enum TaskElevation
    {
        UnknownElevation,
        Kernel,
        System,
        Idle,
        User
    };

    enum TaskStatus
    {
        UnknownStatus,
        Running,
        Sleeping,
        Waiting,
        Stopped,
        Terminated
    };

    struct TaskSecurity
    {
        TaskElevation Elevation;
        Token UniqueToken;
    };

    struct TaskInfo
    {
        uint64_t SpawnTime = 0, UsedTime = 0, OldUsedTime = 0;
        uint64_t OldSystemTime = 0, CurrentSystemTime = 0;
        uint64_t Year, Month, Day, Hour, Minute, Second;
        uint64_t Usage[256]; // MAX_CPU
        TaskArchitecture Architecture;
        TaskPlatform Platform;
        int Priority;
        bool Affinity[256]; // MAX_CPU
    };

    struct TCB
    {
        UTID ID;
        char Name[256];
        struct PCB *Parent;
        IP EntryPoint;
        IPOffset Offset;
        int ExitCode;
        void *Stack;
        ThreadFrame Registers;
        TaskSecurity Security;
        TaskInfo Info;
        TaskStatus Status;
        TaskElevation Elevation;

        void Rename(const char *name)
        {
            for (int i = 0; i < 256; i++)
            {
                Name[i] = name[i];
                if (name[i] == '\0')
                    break;
            }
        }
    };

    struct PCB
    {
        UPID ID;
        char Name[256];
        PCB *Parent;
        int ExitCode;
        TaskSecurity Security;
        TaskInfo Info;
        TaskStatus Status;
        TaskElevation Elevation;
        Vector<TCB *> Threads;
        Vector<PCB *> Children;
        // Memory::PageTable PageTable;
    };

    class Task
    {
    public:
        /**
         * @brief Get the Current Process object
         * @return PCB*
         */
        PCB *GetCurrentProcess();

        /**
         * @brief Get the Current Thread object
         * @return TCB*
         */
        TCB *GetCurrentThread();

        PCB *CreateProcess(PCB *Parent,
                           const char *Name,
                           TaskElevation Elevation);

        TCB *CreateThread(PCB *Parent,
                          IP EntryPoint);

        Task(const IP EntryPoint);
        ~Task();
    };
}

#endif // !__FENNIX_KERNEL_TASKING_H__
