#include "loadelf.hpp"

#include <memory.hpp>
#include <debug.h>
#include <elf.h>

static uint64_t KernelJumpAddress = 0x0;

bool LoadElfInMemory(void *Address, size_t Length, bool Allow64)
{
    Elf32_Ehdr *Header32 = (Elf32_Ehdr *)Address;
    Elf64_Ehdr *Header64 = (Elf64_Ehdr *)Address;

    bool Is64Bit = false;

    if (Header32->e_ident[EI_MAG0] == ELFMAG0 &&
        Header32->e_ident[EI_MAG1] == ELFMAG1 &&
        Header32->e_ident[EI_MAG2] == ELFMAG2 &&
        Header32->e_ident[EI_MAG3] == ELFMAG3)
    {
        debug("Elf magic is correct");
        if (Header32->e_machine == EM_X86_64)
        {
            if (!Allow64)
            {
                error("64 bit elf not allowed");
                return false;
            }
            Is64Bit = true;
            debug("Elf is 64 bit");
        }
        else if (Header32->e_machine == EM_386)
        {
            if (Allow64)
            {
                error("32 bit elf not allowed");
                return false;
            }
            debug("Elf is 32 bit");
        }
        else
        {
            error("Elf is neither 32 nor 64 bit");
            return false;
        }

        if (Is64Bit)
        {
            fixme("64 bit elf not implemented");
        }
        else
        {
            if (Header32->e_phentsize != sizeof(Elf32_Phdr))
            {
                error("Elf program header size is not correct");
                return false;
            }

            Elf32_Phdr *ProgramHeader = (Elf32_Phdr *)((uintptr_t)Address + Header32->e_phoff);
            for (size_t i = 0; i < Header32->e_phnum; i++)
            {
                if (ProgramHeader[i].p_type != PT_LOAD)
                    continue;

                if (ProgramHeader[i].p_filesz > ProgramHeader[i].p_memsz)
                {
                    error("File size is greater than memory size");
                    return false;
                }

                size_t SegmentSize = ProgramHeader[i].p_memsz;
                void *VirtualAddress = (void *)ProgramHeader[i].p_vaddr;
                void *PhysicalAddress = KernelAllocator32.RequestPages(TO_PAGES(SegmentSize));
                debug("- Segment: %p (physical allocated at: %p) (%#x bytes)", VirtualAddress, PhysicalAddress, SegmentSize);

                debug("  Mapping %d pages at %p (%p-%p)", TO_PAGES(SegmentSize), VirtualAddress, PhysicalAddress, (void *)((uintptr_t)PhysicalAddress + TO_PAGES(SegmentSize) * PAGE_SIZE));
                Memory32::Virtual().Map(VirtualAddress, PhysicalAddress, SegmentSize, P | RW);

                void *memcpy_Dest = (void *)((uintptr_t)PhysicalAddress + (ProgramHeader[i].p_vaddr - 0xC0000000));
                void *memcpy_Src = (void *)((uintptr_t)Address + ProgramHeader[i].p_offset);
                debug("  memcpy(%p, %p, %#x)", memcpy_Dest, memcpy_Src, ProgramHeader[i].p_filesz);
                memcpy(memcpy_Dest, memcpy_Src, ProgramHeader[i].p_filesz);

                if (ProgramHeader[i].p_filesz < ProgramHeader[i].p_memsz)
                {
                    debug("  Zeroing %#x bytes at %p", ProgramHeader[i].p_memsz - ProgramHeader[i].p_filesz, (void *)((uintptr_t)PhysicalAddress + ProgramHeader[i].p_filesz));
                    memset((void *)((uintptr_t)PhysicalAddress + ProgramHeader[i].p_filesz), 0, ProgramHeader[i].p_memsz - ProgramHeader[i].p_filesz);
                }
            }

            void *Stack = KernelAllocator32.RequestPage();
            memset(Stack, 0, PAGE_SIZE - 1);
            debug("Stack allocated at: %p", Stack);

            void *StackTop = (void *)((uintptr_t)Stack + PAGE_SIZE - 0x10);
            debug("Stack top at: %p", StackTop);

            debug("Memory Info: %lldMB / %lldMB (%lldMB reserved)",
                  TO_MB(KernelAllocator32.UsedMemory),
                  TO_MB(KernelAllocator32.TotalMemory),
                  TO_MB(KernelAllocator32.ReservedMemory));

            debug("Kernel Entry Point: %p", (void *)Header32->e_entry);
            KernelJumpAddress = (uint64_t)Header32->e_entry;

            asmv("cli");
            asmv("mov %0, %%esp"
                 :
                 : "r"(StackTop));
            asmv("pushf\n"
                 "orl $0x46, (%esp)\n"
                 "popf");
            asmv("mov $0x80000011, %eax");
            asmv("mov %eax, %cr0");
            asmv("mov $0, %eax");
            asmv("mov %eax, %cr4");
            asmv("mov $0, %eax\n"
                 "mov $0, %ebx\n"
                 "mov $0, %ecx\n"
                 "mov $0, %edx\n"
                 "mov $0, %esi\n"
                 "mov $0, %edi\n"
                 "mov $0, %ebp\n");
            asmv("call %0"
                 :
                 : "r"(KernelJumpAddress));
            // asmv("jmp %0"
            //      :
            //      : "r"(KernelJumpAddress));


            // asmv("call *%0"
            //      :
            //      : "r"(KernelJumpAddress));

            // void (*EntryPoint)() = (void (*)())KernelJumpAddress;
            // EntryPoint();
        }
    }

    return false;
}
