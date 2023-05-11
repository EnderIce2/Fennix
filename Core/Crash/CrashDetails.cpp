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

SafeFunction void DivideByZeroExceptionHandler(CHArchTrapFrame *Frame)
{
    fixme("Divide by zero exception\n");
    UNUSED(Frame);
}

SafeFunction void DebugExceptionHandler(CHArchTrapFrame *Frame)
{
    CrashHandler::EHPrint("Kernel triggered debug exception.\n");
    UNUSED(Frame);
}

SafeFunction void NonMaskableInterruptExceptionHandler(CHArchTrapFrame *Frame)
{
    fixme("NMI exception");
    UNUSED(Frame);
}

SafeFunction void BreakpointExceptionHandler(CHArchTrapFrame *Frame)
{
    fixme("Breakpoint exception");
    UNUSED(Frame);
}

SafeFunction void OverflowExceptionHandler(CHArchTrapFrame *Frame)
{
    fixme("Overflow exception");
    UNUSED(Frame);
}

SafeFunction void BoundRangeExceptionHandler(CHArchTrapFrame *Frame)
{
    fixme("Bound range exception");
    UNUSED(Frame);
}

SafeFunction void InvalidOpcodeExceptionHandler(CHArchTrapFrame *Frame)
{
    CrashHandler::EHPrint("Kernel tried to execute an invalid opcode.\n");
    UNUSED(Frame);
}

SafeFunction void DeviceNotAvailableExceptionHandler(CHArchTrapFrame *Frame)
{
    fixme("Device not available exception");
    UNUSED(Frame);
}

SafeFunction void DoubleFaultExceptionHandler(CHArchTrapFrame *Frame)
{
    fixme("Double fault exception");
    UNUSED(Frame);
}

SafeFunction void CoprocessorSegmentOverrunExceptionHandler(CHArchTrapFrame *Frame)
{
    fixme("Coprocessor segment overrun exception");
    UNUSED(Frame);
}

SafeFunction void InvalidTSSExceptionHandler(CHArchTrapFrame *Frame)
{
    fixme("Invalid TSS exception");
    UNUSED(Frame);
}

SafeFunction void SegmentNotPresentExceptionHandler(CHArchTrapFrame *Frame)
{
    fixme("Segment not present exception");
    UNUSED(Frame);
}

SafeFunction void StackFaultExceptionHandler(CHArchTrapFrame *Frame)
{
    CPU::x64::SelectorErrorCode SelCode = {.raw = Frame->ErrorCode};
#if defined(a64)
    CrashHandler::EHPrint("Stack segment fault at address %#lx\n", Frame->rip);
#elif defined(a32)
    CrashHandler::EHPrint("Stack segment fault at address %#lx\n", Frame->eip);
#elif defined(aa64)
#endif
    CrashHandler::EHPrint("External: %d\n", SelCode.External);
    CrashHandler::EHPrint("Table: %d\n", SelCode.Table);
    CrashHandler::EHPrint("Index: %#x\n", SelCode.Idx);
    CrashHandler::EHPrint("Error code: %#lx\n", Frame->ErrorCode);
}

SafeFunction void GeneralProtectionExceptionHandler(CHArchTrapFrame *Frame)
{
    CPU::x64::SelectorErrorCode SelCode = {.raw = Frame->ErrorCode};
    // switch (SelCode.Table)
    // {
    // case CPU::x64::0b00:
    //     memcpy(desc_tmp, "GDT", 3);
    //     break;
    // case CPU::x64::0b01:
    //     memcpy(desc_tmp, "IDT", 3);
    //     break;
    // case CPU::x64::0b10:
    //     memcpy(desc_tmp, "LDT", 3);
    //     break;
    // case CPU::x64::0b11:
    //     memcpy(desc_tmp, "IDT", 3);
    //     break;
    // default:
    //     memcpy(desc_tmp, "Unknown", 7);
    //     break;
    // }
    CrashHandler::EHPrint("Kernel performed an illegal operation.\n");
    CrashHandler::EHPrint("External: %d\n", SelCode.External);
    CrashHandler::EHPrint("Table: %d\n", SelCode.Table);
    CrashHandler::EHPrint("Index: %#x\n", SelCode.Idx);
}

SafeFunction void PageFaultExceptionHandler(CHArchTrapFrame *Frame)
{
    CPU::x64::PageFaultErrorCode params = {.raw = (uint32_t)Frame->ErrorCode};
#if defined(a64)
    CrashHandler::EHPrint("\eAFAFAFAn exception occurred at %#lx by %#lx\n", CrashHandler::PageFaultAddress, Frame->rip);
#elif defined(a32)
    CrashHandler::EHPrint("\eAFAFAFAn exception occurred at %#lx by %#lx\n", CrashHandler::PageFaultAddress, Frame->eip);
#elif defined(aa64)
#endif
    CrashHandler::EHPrint("Page: %s\n", params.P ? "Present" : "Not Present");
    CrashHandler::EHPrint("Write Operation: %s\n", params.W ? "Read-Only" : "Read-Write");
    CrashHandler::EHPrint("Processor Mode: %s\n", params.U ? "User-Mode" : "Kernel-Mode");
    CrashHandler::EHPrint("CPU Reserved Bits: %s\n", params.R ? "Reserved" : "Unreserved");
    CrashHandler::EHPrint("Caused By An Instruction Fetch: %s\n", params.I ? "Yes" : "No");
    CrashHandler::EHPrint("Caused By A Protection-Key Violation: %s\n", params.PK ? "Yes" : "No");
    CrashHandler::EHPrint("Caused By A Shadow Stack Access: %s\n", params.SS ? "Yes" : "No");
    CrashHandler::EHPrint("Caused By An SGX Violation: %s\n", params.SGX ? "Yes" : "No");
    if (Frame->ErrorCode & 0x00000008)
        CrashHandler::EHPrint("One or more page directory entries contain reserved bits which are set to 1.\n");
    else
        CrashHandler::EHPrint(PageFaultDescriptions[Frame->ErrorCode & 0b111]);

#ifdef DEBUG
    uintptr_t CheckPageFaultAddress = 0;
    CheckPageFaultAddress = CrashHandler::PageFaultAddress;
    if (CheckPageFaultAddress == 0)
#ifdef a64
        CheckPageFaultAddress = Frame->rip;
#elif defined(a32)
        CheckPageFaultAddress = Frame->eip;
#elif defined(aa64)
        CheckPageFaultAddress = 0;
#endif

#if defined(a64)
    Memory::Virtual vma = Memory::Virtual(((Memory::PageTable *)CPU::x64::readcr3().raw));
#elif defined(a32)
    Memory::Virtual vma = Memory::Virtual(((Memory::PageTable *)CPU::x32::readcr3().raw));
#elif defined(aa64)
    Memory::Virtual vma = Memory::Virtual();
#warning "TODO: aa64"
#endif

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
#if defined(a64)
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
#if defined(a64)
            Memory::PageMapLevel4 PML4 = ((Memory::PageTable *)CPU::x64::readcr3().raw)->Entries[Index.PMLIndex];
#elif defined(a32)
            Memory::PageMapLevel4 PML4 = ((Memory::PageTable *)CPU::x32::readcr3().raw)->Entries[Index.PMLIndex];
#elif defined(aa64)
            Memory::PageMapLevel4 PML4 = {.raw = 0};
#warning "TODO: aa64"
#endif

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
#endif
        }
    }
#endif
}

SafeFunction void x87FloatingPointExceptionHandler(CHArchTrapFrame *Frame)
{
    fixme("x87 floating point exception");
    UNUSED(Frame);
}

SafeFunction void AlignmentCheckExceptionHandler(CHArchTrapFrame *Frame)
{
    fixme("Alignment check exception");
    UNUSED(Frame);
}

SafeFunction void MachineCheckExceptionHandler(CHArchTrapFrame *Frame)
{
    fixme("Machine check exception");
    UNUSED(Frame);
}

SafeFunction void SIMDFloatingPointExceptionHandler(CHArchTrapFrame *Frame)
{
    fixme("SIMD floating point exception");
    UNUSED(Frame);
}

SafeFunction void VirtualizationExceptionHandler(CHArchTrapFrame *Frame)
{
    fixme("Virtualization exception");
    UNUSED(Frame);
}

SafeFunction void SecurityExceptionHandler(CHArchTrapFrame *Frame)
{
    fixme("Security exception");
    UNUSED(Frame);
}

SafeFunction void UnknownExceptionHandler(CHArchTrapFrame *Frame)
{
    fixme("Unknown exception");
    UNUSED(Frame);
}
