#include "../crashhandler.hpp"
#include "chfcts.hpp"

#include <display.hpp>
#include <printf.h>
#include <debug.h>
#include <smp.hpp>
#include <cpu.hpp>

#if defined(a64)
#include "../../Architecture/amd64/cpu/gdt.hpp"
#elif defined(a32)
#elif defined(aa64)
#endif

#include "../../kernel.h"

static const char *PageFaultDescriptions[8] = {
    "Supervisory process tried to read a non-present page entry\n",
    "Supervisory process tried to read a page and caused a protection fault\n",
    "Supervisory process tried to write to a non-present page entry\n",
    "Supervisory process tried to write a page and caused a protection fault\n",
    "User process tried to read a non-present page entry\n",
    "User process tried to read a page and caused a protection fault\n",
    "User process tried to write to a non-present page entry\n",
    "User process tried to write a page and caused a protection fault\n"};

SafeFunction void UserModeExceptionHandler(CHArchTrapFrame *Frame)
{
    CriticalSection cs;
    debug("Interrupts? %s.", cs.IsInterruptsEnabled() ? "Yes" : "No");
    fixme("Handling user mode exception");
    TaskManager->GetCurrentThread()->Status = Tasking::TaskStatus::Stopped;
    CPUData *CurCPU = GetCurrentCPU();

    {
#if defined(a64)
        CPU::x64::CR0 cr0 = CPU::x64::readcr0();
        CPU::x64::CR2 cr2 = CPU::x64::CR2{.PFLA = CrashHandler::PageFaultAddress};
        CPU::x64::CR3 cr3 = CPU::x64::readcr3();
        CPU::x64::CR4 cr4 = CPU::x64::readcr4();
        CPU::x64::CR8 cr8 = CPU::x64::readcr8();
        CPU::x64::EFER efer;
        efer.raw = CPU::x64::rdmsr(CPU::x64::MSR_EFER);

        error("Technical Informations on CPU %lld:", CurCPU->ID);
        uintptr_t ds;
        asmv("mov %%ds, %0"
             : "=r"(ds));
#elif defined(a32)
        CPU::x32::CR0 cr0 = CPU::x32::readcr0();
        CPU::x32::CR2 cr2 = CPU::x32::CR2{.PFLA = CrashHandler::PageFaultAddress};
        CPU::x32::CR3 cr3 = CPU::x32::readcr3();
        CPU::x32::CR4 cr4 = CPU::x32::readcr4();
        CPU::x32::CR8 cr8 = CPU::x32::readcr8();
        CPU::x32::EFER efer;
        efer.raw = CPU::x32::rdmsr(CPU::x32::MSR_EFER);

        error("Technical Informations on CPU %lld:", CurCPU->ID);
        uintptr_t ds;
        asmv("mov %%ds, %0"
             : "=r"(ds));
#elif defined(aa64)
#endif

#if defined(a64)
        error("FS=%#llx GS=%#llx SS=%#llx CS=%#llx DS=%#llx",
              CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE), CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE),
              Frame->ss, Frame->cs, ds);
        error("R8=%#llx R9=%#llx R10=%#llx R11=%#llx", Frame->r8, Frame->r9, Frame->r10, Frame->r11);
        error("R12=%#llx R13=%#llx R14=%#llx R15=%#llx", Frame->r12, Frame->r13, Frame->r14, Frame->r15);
        error("RAX=%#llx RBX=%#llx RCX=%#llx RDX=%#llx", Frame->rax, Frame->rbx, Frame->rcx, Frame->rdx);
        error("RSI=%#llx RDI=%#llx RBP=%#llx RSP=%#llx", Frame->rsi, Frame->rdi, Frame->rbp, Frame->rsp);
        error("RIP=%#llx RFL=%#llx INT=%#llx ERR=%#llx EFER=%#llx", Frame->rip, Frame->rflags.raw, Frame->InterruptNumber, Frame->ErrorCode, efer.raw);
#elif defined(a32)
        error("FS=%#llx GS=%#llx SS=%#llx CS=%#llx DS=%#llx",
              CPU::x32::rdmsr(CPU::x32::MSR_FS_BASE), CPU::x32::rdmsr(CPU::x32::MSR_GS_BASE),
              Frame->ss, Frame->cs, ds);
        error("EAX=%#llx EBX=%#llx ECX=%#llx EDX=%#llx", Frame->eax, Frame->ebx, Frame->ecx, Frame->edx);
        error("ESI=%#llx EDI=%#llx EBP=%#llx ESP=%#llx", Frame->esi, Frame->edi, Frame->ebp, Frame->esp);
        error("EIP=%#llx EFL=%#llx INT=%#llx ERR=%#llx EFER=%#llx", Frame->eip, Frame->eflags.raw, Frame->InterruptNumber, Frame->ErrorCode, efer.raw);
#elif defined(aa64)
#endif
        error("CR0=%#llx CR2=%#llx CR3=%#llx CR4=%#llx CR8=%#llx", cr0.raw, cr2.raw, cr3.raw, cr4.raw, cr8.raw);

        error("CR0: PE:%s MP:%s EM:%s TS:%s ET:%s NE:%s WP:%s AM:%s NW:%s CD:%s PG:%s R0:%#x R1:%#x R2:%#x",
              cr0.PE ? "True " : "False", cr0.MP ? "True " : "False", cr0.EM ? "True " : "False", cr0.TS ? "True " : "False",
              cr0.ET ? "True " : "False", cr0.NE ? "True " : "False", cr0.WP ? "True " : "False", cr0.AM ? "True " : "False",
              cr0.NW ? "True " : "False", cr0.CD ? "True " : "False", cr0.PG ? "True " : "False",
              cr0.Reserved0, cr0.Reserved1, cr0.Reserved2);

        error("CR2: PFLA: %#llx",
              cr2.PFLA);

        error("CR3: PWT:%s PCD:%s PDBR:%#llx",
              cr3.PWT ? "True " : "False", cr3.PCD ? "True " : "False", cr3.PDBR);

#if defined(a64)
        error("CR4: VME:%s PVI:%s TSD:%s DE:%s PSE:%s PAE:%s MCE:%s PGE:%s PCE:%s UMIP:%s OSFXSR:%s OSXMMEXCPT:%s    LA57:%s    VMXE:%s    SMXE:%s   PCIDE:%s OSXSAVE:%s    SMEP:%s    SMAP:%s PKE:%s R0:%#x R1:%#x R2:%#x",
              cr4.VME ? "True " : "False", cr4.PVI ? "True " : "False", cr4.TSD ? "True " : "False", cr4.DE ? "True " : "False",
              cr4.PSE ? "True " : "False", cr4.PAE ? "True " : "False", cr4.MCE ? "True " : "False", cr4.PGE ? "True " : "False",
              cr4.PCE ? "True " : "False", cr4.UMIP ? "True " : "False", cr4.OSFXSR ? "True " : "False", cr4.OSXMMEXCPT ? "True " : "False",
              cr4.LA57 ? "True " : "False", cr4.VMXE ? "True " : "False", cr4.SMXE ? "True " : "False", cr4.PCIDE ? "True " : "False",
              cr4.OSXSAVE ? "True " : "False", cr4.SMEP ? "True " : "False", cr4.SMAP ? "True " : "False", cr4.PKE ? "True " : "False",
              cr4.Reserved0, cr4.Reserved1, cr4.Reserved2);
#elif defined(a32)
                error("CR4: VME:%s PVI:%s TSD:%s DE:%s PSE:%s PAE:%s MCE:%s PGE:%s PCE:%s UMIP:%s OSFXSR:%s OSXMMEXCPT:%s    LA57:%s    VMXE:%s    SMXE:%s   PCIDE:%s OSXSAVE:%s    SMEP:%s    SMAP:%s PKE:%s R0:%#x R1:%#x",
              cr4.VME ? "True " : "False", cr4.PVI ? "True " : "False", cr4.TSD ? "True " : "False", cr4.DE ? "True " : "False",
              cr4.PSE ? "True " : "False", cr4.PAE ? "True " : "False", cr4.MCE ? "True " : "False", cr4.PGE ? "True " : "False",
              cr4.PCE ? "True " : "False", cr4.UMIP ? "True " : "False", cr4.OSFXSR ? "True " : "False", cr4.OSXMMEXCPT ? "True " : "False",
              cr4.LA57 ? "True " : "False", cr4.VMXE ? "True " : "False", cr4.SMXE ? "True " : "False", cr4.PCIDE ? "True " : "False",
              cr4.OSXSAVE ? "True " : "False", cr4.SMEP ? "True " : "False", cr4.SMAP ? "True " : "False", cr4.PKE ? "True " : "False",
              cr4.Reserved0, cr4.Reserved1);
#endif

        error("CR8: TPL:%d", cr8.TPL);

#if defined(a64)
        error("RFL: CF:%s PF:%s AF:%s ZF:%s SF:%s TF:%s IF:%s DF:%s OF:%s IOPL:%s NT:%s RF:%s VM:%s AC:%s VIF:%s VIP:%s ID:%s AlwaysOne:%d R0:%#x R1:%#x R2:%#x R3:%#x",
              Frame->rflags.CF ? "True " : "False", Frame->rflags.PF ? "True " : "False", Frame->rflags.AF ? "True " : "False", Frame->rflags.ZF ? "True " : "False",
              Frame->rflags.SF ? "True " : "False", Frame->rflags.TF ? "True " : "False", Frame->rflags.IF ? "True " : "False", Frame->rflags.DF ? "True " : "False",
              Frame->rflags.OF ? "True " : "False", Frame->rflags.IOPL ? "True " : "False", Frame->rflags.NT ? "True " : "False", Frame->rflags.RF ? "True " : "False",
              Frame->rflags.VM ? "True " : "False", Frame->rflags.AC ? "True " : "False", Frame->rflags.VIF ? "True " : "False", Frame->rflags.VIP ? "True " : "False",
              Frame->rflags.ID ? "True " : "False", Frame->rflags.AlwaysOne,
              Frame->rflags.Reserved0, Frame->rflags.Reserved1, Frame->rflags.Reserved2, Frame->rflags.Reserved3);
#elif defined(a32)
        error("EFL: CF:%s PF:%s AF:%s ZF:%s SF:%s TF:%s IF:%s DF:%s OF:%s IOPL:%s NT:%s RF:%s VM:%s AC:%s VIF:%s VIP:%s ID:%s AlwaysOne:%d R0:%#x R1:%#x R2:%#x",
              Frame->eflags.CF ? "True " : "False", Frame->eflags.PF ? "True " : "False", Frame->eflags.AF ? "True " : "False", Frame->eflags.ZF ? "True " : "False",
              Frame->eflags.SF ? "True " : "False", Frame->eflags.TF ? "True " : "False", Frame->eflags.IF ? "True " : "False", Frame->eflags.DF ? "True " : "False",
              Frame->eflags.OF ? "True " : "False", Frame->eflags.IOPL ? "True " : "False", Frame->eflags.NT ? "True " : "False", Frame->eflags.RF ? "True " : "False",
              Frame->eflags.VM ? "True " : "False", Frame->eflags.AC ? "True " : "False", Frame->eflags.VIF ? "True " : "False", Frame->eflags.VIP ? "True " : "False",
              Frame->eflags.ID ? "True " : "False", Frame->eflags.AlwaysOne,
              Frame->eflags.Reserved0, Frame->eflags.Reserved1, Frame->eflags.Reserved2);
#elif defined(aa64)
#endif

        error("EFER: SCE:%s LME:%s LMA:%s NXE:%s SVME:%s LMSLE:%s FFXSR:%s TCE:%s R0:%#x R1:%#x R2:%#x",
              efer.SCE ? "True " : "False", efer.LME ? "True " : "False", efer.LMA ? "True " : "False", efer.NXE ? "True " : "False",
              efer.SVME ? "True " : "False", efer.LMSLE ? "True " : "False", efer.FFXSR ? "True " : "False", efer.TCE ? "True " : "False",
              efer.Reserved0, efer.Reserved1, efer.Reserved2);
    }

    switch (Frame->InterruptNumber)
    {
    case CPU::x86::DivideByZero:
    {
        break;
    }
    case CPU::x86::Debug:
    {
        break;
    }
    case CPU::x86::NonMaskableInterrupt:
    {
        break;
    }
    case CPU::x86::Breakpoint:
    {
        break;
    }
    case CPU::x86::Overflow:
    {
        break;
    }
    case CPU::x86::BoundRange:
    {
        break;
    }
    case CPU::x86::InvalidOpcode:
    {
        break;
    }
    case CPU::x86::DeviceNotAvailable:
    {
        break;
    }
    case CPU::x86::DoubleFault:
    {
        break;
    }
    case CPU::x86::CoprocessorSegmentOverrun:
    {
        break;
    }
    case CPU::x86::InvalidTSS:
    {
        break;
    }
    case CPU::x86::SegmentNotPresent:
    {
        break;
    }
    case CPU::x86::StackSegmentFault:
    {
        break;
    }
    case CPU::x86::GeneralProtectionFault:
    {
        break;
    }
    case CPU::x86::PageFault:
    {
        uintptr_t CheckPageFaultAddress = 0;
        CPU::x64::PageFaultErrorCode params = {.raw = (uint32_t)Frame->ErrorCode};
#if defined(a64)
        CheckPageFaultAddress = CrashHandler::PageFaultAddress;
        if (CheckPageFaultAddress == 0)
            CheckPageFaultAddress = Frame->rip;

        error("An exception occurred at %#lx by %#lx", CrashHandler::PageFaultAddress, Frame->rip);
#elif defined(a32)
        error("An exception occurred at %#lx by %#lx", CrashHandler::PageFaultAddress, Frame->eip);
#elif defined(aa64)
#endif
        error("Page: %s", params.P ? "Present" : "Not Present");
        error("Write Operation: %s", params.W ? "Read-Only" : "Read-Write");
        error("Processor Mode: %s", params.U ? "User-Mode" : "Kernel-Mode");
        error("CPU Reserved Bits: %s", params.R ? "Reserved" : "Unreserved");
        error("Caused By An Instruction Fetch: %s", params.I ? "Yes" : "No");
        error("Caused By A Protection-Key Violation: %s", params.PK ? "Yes" : "No");
        error("Caused By A Shadow Stack Access: %s", params.SS ? "Yes" : "No");
        error("Caused By An SGX Violation: %s", params.SGX ? "Yes" : "No");
        if (Frame->ErrorCode & 0x00000008)
            error("One or more page directory entries contain reserved bits which are set to 1.");
        else
            error(PageFaultDescriptions[Frame->ErrorCode & 0b111]);

#ifdef DEBUG
        if (CurCPU)
        {
            Memory::Virtual vma = Memory::Virtual(CurCPU->CurrentProcess->PageTable);
            bool PageAvailable = vma.Check((void *)CheckPageFaultAddress);
            debug("Page available (Check(...)): %s. %s",
                  PageAvailable ? "Yes" : "No",
                  (params.P && !PageAvailable) ? "CR2 == Present; Check() != Present??????" : "CR2 confirms Check() result.");

            if (PageAvailable)
            {
                bool Present = vma.Check((void *)CheckPageFaultAddress);
                bool ReadWrite = vma.Check((void *)CheckPageFaultAddress, Memory::PTFlag::RW);
                bool User = vma.Check((void *)CheckPageFaultAddress, Memory::PTFlag::US);
                bool WriteThrough = vma.Check((void *)CheckPageFaultAddress, Memory::PTFlag::PWT);
                bool CacheDisabled = vma.Check((void *)CheckPageFaultAddress, Memory::PTFlag::PCD);
                bool Accessed = vma.Check((void *)CheckPageFaultAddress, Memory::PTFlag::A);
                bool Dirty = vma.Check((void *)CheckPageFaultAddress, Memory::PTFlag::D);
                bool Global = vma.Check((void *)CheckPageFaultAddress, Memory::PTFlag::G);
                /* ... */

                debug("Page available: %s", Present ? "Yes" : "No");
                debug("Page read/write: %s", ReadWrite ? "Yes" : "No");
                debug("Page user/kernel: %s", User ? "User" : "Kernel");
                debug("Page write-through: %s", WriteThrough ? "Yes" : "No");
                debug("Page cache disabled: %s", CacheDisabled ? "Yes" : "No");
                debug("Page accessed: %s", Accessed ? "Yes" : "No");
                debug("Page dirty: %s", Dirty ? "Yes" : "No");
                debug("Page global: %s", Global ? "Yes" : "No");

                if (Present)
                {
                    uintptr_t CheckPageFaultLinearAddress = (uintptr_t)CheckPageFaultAddress;
                    CheckPageFaultLinearAddress &= 0xFFFFFFFFFFFFF000;
                    debug("%#lx -> %#lx", CheckPageFaultAddress, CheckPageFaultLinearAddress);

                    Memory::Virtual::PageMapIndexer Index = Memory::Virtual::PageMapIndexer((uintptr_t)CheckPageFaultLinearAddress);
                    debug("Index for %#lx is PML:%d PDPTE:%d PDE:%d PTE:%d",
                          CheckPageFaultLinearAddress,
                          Index.PMLIndex,
                          Index.PDPTEIndex,
                          Index.PDEIndex,
                          Index.PTEIndex);
                    Memory::PageMapLevel4 PML4 = CurCPU->CurrentProcess->PageTable->Entries[Index.PMLIndex];

                    Memory::PageDirectoryPointerTableEntryPtr *PDPTE = (Memory::PageDirectoryPointerTableEntryPtr *)((uintptr_t)PML4.GetAddress() << 12);
                    Memory::PageDirectoryEntryPtr *PDE = (Memory::PageDirectoryEntryPtr *)((uintptr_t)PDPTE->Entries[Index.PDPTEIndex].GetAddress() << 12);
                    Memory::PageTableEntryPtr *PTE = (Memory::PageTableEntryPtr *)((uintptr_t)PDE->Entries[Index.PDEIndex].GetAddress() << 12);

                    debug("# %03d-%03d-%03d-%03d: P:%s RW:%s US:%s PWT:%s PCB:%s A:%s NX:%s Address:%#lx",
                          Index.PMLIndex, 0, 0, 0,
                          PML4.Present ? "1" : "0",
                          PML4.ReadWrite ? "1" : "0",
                          PML4.UserSupervisor ? "1" : "0",
                          PML4.WriteThrough ? "1" : "0",
                          PML4.CacheDisable ? "1" : "0",
                          PML4.Accessed ? "1" : "0",
                          PML4.ExecuteDisable ? "1" : "0",
                          PML4.GetAddress() << 12);

                    debug("# %03d-%03d-%03d-%03d: P:%s RW:%s US:%s PWT:%s PCB:%s A:%s NX:%s Address:%#lx",
                          Index.PMLIndex, Index.PDPTEIndex, 0, 0,
                          PDPTE->Entries[Index.PDPTEIndex].Present ? "1" : "0",
                          PDPTE->Entries[Index.PDPTEIndex].ReadWrite ? "1" : "0",
                          PDPTE->Entries[Index.PDPTEIndex].UserSupervisor ? "1" : "0",
                          PDPTE->Entries[Index.PDPTEIndex].WriteThrough ? "1" : "0",
                          PDPTE->Entries[Index.PDPTEIndex].CacheDisable ? "1" : "0",
                          PDPTE->Entries[Index.PDPTEIndex].Accessed ? "1" : "0",
                          PDPTE->Entries[Index.PDPTEIndex].ExecuteDisable ? "1" : "0",
                          PDPTE->Entries[Index.PDPTEIndex].GetAddress() << 12);

                    debug("# %03d-%03d-%03d-%03d: P:%s RW:%s US:%s PWT:%s PCB:%s A:%s NX:%s Address:%#lx",
                          Index.PMLIndex, Index.PDPTEIndex, Index.PDEIndex, 0,
                          PDE->Entries[Index.PDEIndex].Present ? "1" : "0",
                          PDE->Entries[Index.PDEIndex].ReadWrite ? "1" : "0",
                          PDE->Entries[Index.PDEIndex].UserSupervisor ? "1" : "0",
                          PDE->Entries[Index.PDEIndex].WriteThrough ? "1" : "0",
                          PDE->Entries[Index.PDEIndex].CacheDisable ? "1" : "0",
                          PDE->Entries[Index.PDEIndex].Accessed ? "1" : "0",
                          PDE->Entries[Index.PDEIndex].ExecuteDisable ? "1" : "0",
                          PDE->Entries[Index.PDEIndex].GetAddress() << 12);

                    debug("# %03d-%03d-%03d-%03d: P:%s RW:%s US:%s PWT:%s PCB:%s A:%s D:%s PAT:%s G:%s PK:%d NX:%s Address:%#lx",
                          Index.PMLIndex, Index.PDPTEIndex, Index.PDEIndex, Index.PTEIndex,
                          PTE->Entries[Index.PTEIndex].Present ? "1" : "0",
                          PTE->Entries[Index.PTEIndex].ReadWrite ? "1" : "0",
                          PTE->Entries[Index.PTEIndex].UserSupervisor ? "1" : "0",
                          PTE->Entries[Index.PTEIndex].WriteThrough ? "1" : "0",
                          PTE->Entries[Index.PTEIndex].CacheDisable ? "1" : "0",
                          PTE->Entries[Index.PTEIndex].Accessed ? "1" : "0",
                          PTE->Entries[Index.PTEIndex].Dirty ? "1" : "0",
                          PTE->Entries[Index.PTEIndex].PageAttributeTable ? "1" : "0",
                          PTE->Entries[Index.PTEIndex].Global ? "1" : "0",
                          PTE->Entries[Index.PTEIndex].ProtectionKey,
                          PTE->Entries[Index.PTEIndex].ExecuteDisable ? "1" : "0",
                          PTE->Entries[Index.PTEIndex].GetAddress() << 12);
                }
            }
        }
#endif

        if (CurCPU)
            if (CurCPU->CurrentThread->Stack->Expand(CrashHandler::PageFaultAddress))
            {
                debug("Stack expanded");
                TaskManager->GetCurrentThread()->Status = Tasking::TaskStatus::Ready;
                return;
            }
        break;
    }
    case CPU::x86::x87FloatingPoint:
    {
        break;
    }
    case CPU::x86::AlignmentCheck:
    {
        break;
    }
    case CPU::x86::MachineCheck:
    {
        break;
    }
    case CPU::x86::SIMDFloatingPoint:
    {
        break;
    }
    case CPU::x86::Virtualization:
    {
        break;
    }
    case CPU::x86::Security:
    {
        break;
    }
    default:
    {
        break;
    }
    }

    TaskManager->GetCurrentThread()->Status = Tasking::TaskStatus::Terminated;
    __sync;
    error("End of report.");
    CPU::Interrupts(CPU::Enable);
    debug("Interrupts enabled back.");
    return;
}
