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

#include <boot/binfo.h>
#include <types.h>
#include <debug.h>
#include <convert.h>

#include "../../../../tools/limine/limine.h"
#include "../../../kernel.h"

void InitLimine();

static volatile struct limine_entry_point_request EntryPointRequest = {
    .id = LIMINE_ENTRY_POINT_REQUEST,
    .revision = 0,
    .response = NULL,
    .entry = InitLimine};
static volatile struct limine_bootloader_info_request BootloaderInfoRequest = {
    .id = LIMINE_BOOTLOADER_INFO_REQUEST,
    .revision = 0};
static volatile struct limine_terminal_request TerminalRequest = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0};
static volatile struct limine_framebuffer_request FramebufferRequest = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0};
static volatile struct limine_memmap_request MemmapRequest = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0};
static volatile struct limine_kernel_address_request KernelAddressRequest = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0};
static volatile struct limine_rsdp_request RsdpRequest = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0};
static volatile struct limine_kernel_file_request KernelFileRequest = {
    .id = LIMINE_KERNEL_FILE_REQUEST,
    .revision = 0};
static volatile struct limine_module_request ModuleRequest = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0};
static volatile struct limine_smbios_request SmbiosRequest = {
    .id = LIMINE_SMBIOS_REQUEST,
    .revision = 0};

void *TempStackPtr = NULL;
__naked __used __no_stack_protector void InitLimine()
{
    asmv("mov %%rsp, %0"
         : "=r"(TempStackPtr));

    asmv("mov %0, %%rsp"
         :
         : "r"((uintptr_t)TempStackPtr - 0xFFFF800000000000));

    asmv("jmp InitLimineAfterStack");
}

SafeFunction NIF void InitLimineAfterStack()
{
    struct BootInfo binfo;
    struct limine_bootloader_info_response *BootloaderInfoResponse = BootloaderInfoRequest.response;
    info("Bootloader: %s %s", BootloaderInfoResponse->name, BootloaderInfoResponse->version);

    struct limine_terminal_response *TerminalResponse = TerminalRequest.response;

    if (TerminalResponse == NULL || TerminalResponse->terminal_count < 1)
    {
        warn("No terminal available.");
        while (1)
            asmv("hlt");
    }
    TerminalResponse->write(TerminalResponse->terminals[0], "\033[37mPlease wait... ", 20);

    struct limine_framebuffer_response *FrameBufferResponse = FramebufferRequest.response;
    struct limine_memmap_response *MemmapResponse = MemmapRequest.response;
    struct limine_kernel_address_response *KernelAddressResponse = KernelAddressRequest.response;
    struct limine_rsdp_response *RsdpResponse = RsdpRequest.response;
    struct limine_kernel_file_response *KernelFileResponse = KernelFileRequest.response;
    struct limine_module_response *ModuleResponse = ModuleRequest.response;
    struct limine_smbios_response *SmbiosResponse = SmbiosRequest.response;

    if (FrameBufferResponse == NULL || FrameBufferResponse->framebuffer_count < 1)
    {
        error("No framebuffer available [%p;%ld]", FrameBufferResponse,
              (FrameBufferResponse == NULL) ? 0 : FrameBufferResponse->framebuffer_count);
        TerminalResponse->write(TerminalResponse->terminals[0], "No framebuffer available", 24);
        while (1)
            asmv("hlt");
    }

    if (MemmapResponse == NULL || MemmapResponse->entry_count < 1)
    {
        error("No memory map available [%p;%ld]", MemmapResponse,
              (MemmapResponse == NULL) ? 0 : MemmapResponse->entry_count);
        TerminalResponse->write(TerminalResponse->terminals[0], "No memory map available", 23);
        while (1)
            asmv("hlt");
    }

    if (KernelAddressResponse == NULL)
    {
        error("No kernel address available [%p]", KernelAddressResponse);
        TerminalResponse->write(TerminalResponse->terminals[0], "No kernel address available", 27);
        while (1)
            asmv("hlt");
    }

    if (RsdpResponse == NULL || RsdpResponse->address == 0)
    {
        error("No RSDP address available [%p;%p]", RsdpResponse,
              (RsdpResponse == NULL) ? 0 : RsdpResponse->address);
        TerminalResponse->write(TerminalResponse->terminals[0], "No RSDP address available", 25);
        while (1)
            asmv("hlt");
    }

    if (KernelFileResponse == NULL || KernelFileResponse->kernel_file == NULL)
    {
        error("No kernel file available [%p;%p]", KernelFileResponse,
              (KernelFileResponse == NULL) ? 0 : KernelFileResponse->kernel_file);
        TerminalResponse->write(TerminalResponse->terminals[0], "No kernel file available", 24);
        while (1)
            asmv("hlt");
    }

    if (ModuleResponse == NULL || ModuleResponse->module_count < 1)
    {
        error("No module information available [%p;%ld]", ModuleResponse,
              (ModuleResponse == NULL) ? 0 : ModuleResponse->module_count);
        TerminalResponse->write(TerminalResponse->terminals[0], "No module information available", 31);
        while (1)
            asmv("hlt");
    }

    for (uint64_t i = 0; i < FrameBufferResponse->framebuffer_count; i++)
    {
        struct limine_framebuffer *framebuffer = FrameBufferResponse->framebuffers[i];
        binfo.Framebuffer[i].Type = RGB;
        binfo.Framebuffer[i].BaseAddress = (void *)((uint64_t)framebuffer->address - 0xFFFF800000000000);
        binfo.Framebuffer[i].Width = (uint32_t)framebuffer->width;
        binfo.Framebuffer[i].Height = (uint32_t)framebuffer->height;
        binfo.Framebuffer[i].Pitch = (uint32_t)framebuffer->pitch;
        binfo.Framebuffer[i].BitsPerPixel = framebuffer->bpp;
        binfo.Framebuffer[i].MemoryModel = framebuffer->memory_model;
        binfo.Framebuffer[i].RedMaskSize = framebuffer->red_mask_size;
        binfo.Framebuffer[i].RedMaskShift = framebuffer->red_mask_shift;
        binfo.Framebuffer[i].GreenMaskSize = framebuffer->green_mask_size;
        binfo.Framebuffer[i].GreenMaskShift = framebuffer->green_mask_shift;
        binfo.Framebuffer[i].BlueMaskSize = framebuffer->blue_mask_size;
        binfo.Framebuffer[i].BlueMaskShift = framebuffer->blue_mask_shift;
        binfo.Framebuffer[i].ExtendedDisplayIdentificationData = framebuffer->edid;
        binfo.Framebuffer[i].EDIDSize = framebuffer->edid_size;
        debug("Framebuffer %d: %dx%d %d bpp", i, framebuffer->width, framebuffer->height, framebuffer->bpp);
        debug("More info:\nAddress: %p\nPitch: %ld\nMemoryModel: %d\nRedMaskSize: %d\nRedMaskShift: %d\nGreenMaskSize: %d\nGreenMaskShift: %d\nBlueMaskSize: %d\nBlueMaskShift: %d\nEDID: %p\nEDIDSize: %d",
              (uint64_t)framebuffer->address - 0xFFFF800000000000, framebuffer->pitch, framebuffer->memory_model, framebuffer->red_mask_size, framebuffer->red_mask_shift, framebuffer->green_mask_size, framebuffer->green_mask_shift, framebuffer->blue_mask_size, framebuffer->blue_mask_shift, framebuffer->edid, framebuffer->edid_size);
    }

    binfo.Memory.Entries = MemmapResponse->entry_count;
    for (uint64_t i = 0; i < MemmapResponse->entry_count; i++)
    {
        if (MemmapResponse->entry_count > MAX_MEMORY_ENTRIES)
        {
            warn("Too many memory entries, skipping the rest...");
            break;
        }

        struct limine_memmap_entry *entry = MemmapResponse->entries[i];
        binfo.Memory.Size += entry->length;
        switch (entry->type)
        {
        case LIMINE_MEMMAP_USABLE:
            binfo.Memory.Entry[i].BaseAddress = (void *)entry->base;
            binfo.Memory.Entry[i].Length = entry->length;
            binfo.Memory.Entry[i].Type = Usable;
            break;
        case LIMINE_MEMMAP_RESERVED:
            binfo.Memory.Entry[i].BaseAddress = (void *)entry->base;
            binfo.Memory.Entry[i].Length = entry->length;
            binfo.Memory.Entry[i].Type = Reserved;
            break;
        case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
            binfo.Memory.Entry[i].BaseAddress = (void *)entry->base;
            binfo.Memory.Entry[i].Length = entry->length;
            binfo.Memory.Entry[i].Type = ACPIReclaimable;
            break;
        case LIMINE_MEMMAP_ACPI_NVS:
            binfo.Memory.Entry[i].BaseAddress = (void *)entry->base;
            binfo.Memory.Entry[i].Length = entry->length;
            binfo.Memory.Entry[i].Type = ACPINVS;
            break;
        case LIMINE_MEMMAP_BAD_MEMORY:
            binfo.Memory.Entry[i].BaseAddress = (void *)entry->base;
            binfo.Memory.Entry[i].Length = entry->length;
            binfo.Memory.Entry[i].Type = BadMemory;
            break;
        case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
            binfo.Memory.Entry[i].BaseAddress = (void *)entry->base;
            binfo.Memory.Entry[i].Length = entry->length;
            binfo.Memory.Entry[i].Type = BootloaderReclaimable;
            break;
        case LIMINE_MEMMAP_KERNEL_AND_MODULES:
            binfo.Memory.Entry[i].BaseAddress = (void *)entry->base;
            binfo.Memory.Entry[i].Length = entry->length;
            binfo.Memory.Entry[i].Type = KernelAndModules;
            break;
        case LIMINE_MEMMAP_FRAMEBUFFER:
            binfo.Memory.Entry[i].BaseAddress = (void *)entry->base;
            binfo.Memory.Entry[i].Length = entry->length;
            binfo.Memory.Entry[i].Type = Framebuffer;
            break;
        default:
            binfo.Memory.Entry[i].BaseAddress = (void *)entry->base;
            binfo.Memory.Entry[i].Length = entry->length;
            binfo.Memory.Entry[i].Type = Unknown;
            break;
        }
    }

    for (uint64_t i = 0; i < ModuleResponse->module_count; i++)
    {
        if (i > MAX_MODULES)
        {
            warn("Too many modules, skipping the rest...");
            break;
        }

        binfo.Modules[i].Address = (void *)((uint64_t)ModuleResponse->modules[i]->address - 0xFFFF800000000000);
        strncpy(binfo.Modules[i].Path,
                ModuleResponse->modules[i]->path,
                strlen(ModuleResponse->modules[i]->path) + 1);
        strncpy(binfo.Modules[i].CommandLine,
                ModuleResponse->modules[i]->cmdline,
                strlen(ModuleResponse->modules[i]->cmdline) + 1);
        binfo.Modules[i].Size = ModuleResponse->modules[i]->size;
        debug("Module %d:\nAddress: %p\nPath: %s\nCommand Line: %s\nSize: %ld", i,
              (uint64_t)ModuleResponse->modules[i]->address - 0xFFFF800000000000, ModuleResponse->modules[i]->path,
              ModuleResponse->modules[i]->cmdline, ModuleResponse->modules[i]->size);
    }

    binfo.RSDP = (struct RSDPInfo *)((uint64_t)RsdpResponse->address - 0xFFFF800000000000);
    trace("RSDP: %p(%p) [Signature: %.8s] [OEM: %.6s]",
          RsdpResponse->address, binfo.RSDP, binfo.RSDP->Signature, binfo.RSDP->OEMID);

    debug("SMBIOS: %p %p", SmbiosResponse->entry_32, SmbiosResponse->entry_64);
    if (SmbiosResponse->entry_32 != NULL)
        binfo.SMBIOSPtr = (void *)((uint64_t)SmbiosResponse->entry_32 - 0xFFFF800000000000);
    else if (SmbiosResponse->entry_64 != NULL)
        binfo.SMBIOSPtr = (void *)((uint64_t)SmbiosResponse->entry_64 - 0xFFFF800000000000);
    else
        binfo.SMBIOSPtr = NULL;

    binfo.Kernel.PhysicalBase = (void *)KernelAddressResponse->physical_base;
    binfo.Kernel.VirtualBase = (void *)KernelAddressResponse->virtual_base;
    binfo.Kernel.FileBase = (void *)((uint64_t)KernelFileResponse->kernel_file->address - 0xFFFF800000000000);
    strncpy(binfo.Kernel.CommandLine,
            KernelFileResponse->kernel_file->cmdline,
            strlen(KernelFileResponse->kernel_file->cmdline) + 1);
    binfo.Kernel.Size = KernelFileResponse->kernel_file->size;
    trace("Kernel physical address: %p", KernelAddressResponse->physical_base);
    trace("Kernel virtual address: %p", KernelAddressResponse->virtual_base);

    strncpy(binfo.Bootloader.Name,
            BootloaderInfoResponse->name,
            strlen(BootloaderInfoResponse->name) + 1);
    strncpy(binfo.Bootloader.Version,
            BootloaderInfoResponse->version,
            strlen(BootloaderInfoResponse->version) + 1);

    // Call kernel entry point
    Entry(&binfo);
}
