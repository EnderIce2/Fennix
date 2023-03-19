#include <types.h>

#include <boot/protocols/multiboot2.h>
#include <memory.hpp>
#include <io.h>

#include "../../../kernel.h"

BootInfo mb2binfo;

enum VideoType
{
    VIDEO_TYPE_NONE = 0x00,
    VIDEO_TYPE_COLOUR = 0x20,
    VIDEO_TYPE_MONOCHROME = 0x30,
};

uint16_t GetBiosAreaHardware()
{
    const uint16_t *BIOSDataAreaDetectedHardware = (const uint16_t *)0x410;
    return *BIOSDataAreaDetectedHardware;
}

enum VideoType GetVideoType() { return (enum VideoType)(GetBiosAreaHardware() & 0x30); }

void GetSMBIOS()
{
    unsigned char *SMBIOSAddress = (unsigned char *)0xF0000;
    while ((unsigned int)(unsigned long)SMBIOSAddress < 0x100000)
    {
        if (SMBIOSAddress[0] == '_' &&
            SMBIOSAddress[1] == 'S' &&
            SMBIOSAddress[2] == 'M' &&
            SMBIOSAddress[3] == '_')
        {
            unsigned char Checksum = 0;
            int Length = SMBIOSAddress[5];
            for (int i = 0; i < Length; i++)
                Checksum += SMBIOSAddress[i];

            if (Checksum == 0)
                break;
        }
        SMBIOSAddress += 16;
    }

    if ((unsigned int)(unsigned long)SMBIOSAddress == 0x100000)
    {
        // No SMBIOS found
    }
}

void ProcessMB2(unsigned long Info)
{
    uint8_t *VideoBuffer = (uint8_t *)0xB8F00 + 0xC0000000;
    int pos = 0;
    auto InfoAddress = Info;
    for (auto Tag = (struct multiboot_tag *)((uint8_t *)InfoAddress + 8);
         ;
         Tag = (struct multiboot_tag *)((multiboot_uint8_t *)Tag + ((Tag->size + 7) & ~7)))
    {
        VideoBuffer[pos++] = '.';
        VideoBuffer[pos++] = 0x2;

        if (Tag->type == MULTIBOOT_TAG_TYPE_END)
        {
            debug("End of multiboot2 tags");
            break;
        }

        switch (Tag->type)
        {
        case MULTIBOOT_TAG_TYPE_CMDLINE:
        {
            strncpy(mb2binfo.Kernel.CommandLine,
                    ((multiboot_tag_string *)Tag)->string,
                    strlen(((multiboot_tag_string *)Tag)->string));
            debug("Kernel command line: %s", mb2binfo.Kernel.CommandLine);
            break;
        }
        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
        {
            strncpy(mb2binfo.Bootloader.Name,
                    ((multiboot_tag_string *)Tag)->string,
                    strlen(((multiboot_tag_string *)Tag)->string));
            debug("Bootloader name: %s", mb2binfo.Bootloader.Name);
            break;
        }
        case MULTIBOOT_TAG_TYPE_MODULE:
        {
            multiboot_tag_module *module = (multiboot_tag_module *)Tag;
            static int module_count = 0;
            mb2binfo.Modules[module_count++].Address = (void *)module->mod_start;
            mb2binfo.Modules[module_count++].Size = module->size;
            strncpy(mb2binfo.Modules[module_count++].Path, "(null)", 6);
            strncpy(mb2binfo.Modules[module_count++].CommandLine, module->cmdline,
                    strlen(module->cmdline));
            debug("Module: %s", mb2binfo.Modules[module_count++].Path);
            break;
        }
        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
        {
            multiboot_tag_basic_meminfo *meminfo = (multiboot_tag_basic_meminfo *)Tag;
            fixme("basic_meminfo->[mem_lower: %#x, mem_upper: %#x]",
                  meminfo->mem_lower, meminfo->mem_upper);
            break;
        }
        case MULTIBOOT_TAG_TYPE_BOOTDEV:
        {
            multiboot_tag_bootdev *bootdev = (multiboot_tag_bootdev *)Tag;
            fixme("bootdev->[biosdev: %#x, slice: %#x, part: %#x]",
                  bootdev->biosdev, bootdev->slice, bootdev->part);
            break;
        }
        case MULTIBOOT_TAG_TYPE_MMAP:
        {
            multiboot_tag_mmap *mmap = (multiboot_tag_mmap *)Tag;
            uint32_t EntryCount = mmap->size / sizeof(multiboot_mmap_entry);
            mb2binfo.Memory.Entries = EntryCount;
            for (uint32_t i = 0; i < EntryCount; i++)
            {
                if (EntryCount > MAX_MEMORY_ENTRIES)
                {
                    warn("Too many memory entries, skipping the rest...");
                    break;
                }
                multiboot_mmap_entry entry = mmap->entries[i];
                mb2binfo.Memory.Size += entry.len;
                switch (entry.type)
                {
                case MULTIBOOT_MEMORY_AVAILABLE:
                    mb2binfo.Memory.Entry[i].BaseAddress = (void *)entry.addr;
                    mb2binfo.Memory.Entry[i].Length = entry.len;
                    mb2binfo.Memory.Entry[i].Type = Usable;
                    break;
                case MULTIBOOT_MEMORY_RESERVED:
                    mb2binfo.Memory.Entry[i].BaseAddress = (void *)entry.addr;
                    mb2binfo.Memory.Entry[i].Length = entry.len;
                    mb2binfo.Memory.Entry[i].Type = Reserved;
                    break;
                case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
                    mb2binfo.Memory.Entry[i].BaseAddress = (void *)entry.addr;
                    mb2binfo.Memory.Entry[i].Length = entry.len;
                    mb2binfo.Memory.Entry[i].Type = ACPIReclaimable;
                    break;
                case MULTIBOOT_MEMORY_NVS:
                    mb2binfo.Memory.Entry[i].BaseAddress = (void *)entry.addr;
                    mb2binfo.Memory.Entry[i].Length = entry.len;
                    mb2binfo.Memory.Entry[i].Type = ACPINVS;
                    break;
                case MULTIBOOT_MEMORY_BADRAM:
                    mb2binfo.Memory.Entry[i].BaseAddress = (void *)entry.addr;
                    mb2binfo.Memory.Entry[i].Length = entry.len;
                    mb2binfo.Memory.Entry[i].Type = BadMemory;
                    break;
                default:
                    mb2binfo.Memory.Entry[i].BaseAddress = (void *)entry.addr;
                    mb2binfo.Memory.Entry[i].Length = entry.len;
                    mb2binfo.Memory.Entry[i].Type = Unknown;
                    break;
                }
                debug("Memory entry: [BaseAddress: %#x, Length: %#x, Type: %d]",
                      mb2binfo.Memory.Entry[i].BaseAddress,
                      mb2binfo.Memory.Entry[i].Length,
                      mb2binfo.Memory.Entry[i].Type);
            }
            break;
        }
        case MULTIBOOT_TAG_TYPE_VBE:
        {
            multiboot_tag_vbe *vbe = (multiboot_tag_vbe *)Tag;
            fixme("vbe->[vbe_mode: %#x, vbe_interface_seg: %#x, vbe_interface_off: %#x, vbe_interface_len: %#x]",
                  vbe->vbe_mode, vbe->vbe_interface_seg, vbe->vbe_interface_off, vbe->vbe_interface_len);
            break;
        }
        case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
        {
            multiboot_tag_framebuffer *fb = (multiboot_tag_framebuffer *)Tag;
            static int fb_count = 0;
            mb2binfo.Framebuffer[fb_count].BaseAddress = (void *)fb->common.framebuffer_addr;
            mb2binfo.Framebuffer[fb_count].Width = fb->common.framebuffer_width;
            mb2binfo.Framebuffer[fb_count].Height = fb->common.framebuffer_height;
            mb2binfo.Framebuffer[fb_count].Pitch = fb->common.framebuffer_pitch;
            mb2binfo.Framebuffer[fb_count].BitsPerPixel = fb->common.framebuffer_bpp;
            mb2binfo.Framebuffer[fb_count].MemoryModel = fb->common.framebuffer_type;
            switch (fb->common.framebuffer_type)
            {
            case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED:
            {
                fixme("indexed");
                break;
            }
            case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
            {
                mb2binfo.Framebuffer[fb_count].RedMaskSize = fb->framebuffer_red_mask_size;
                mb2binfo.Framebuffer[fb_count].RedMaskShift = fb->framebuffer_red_field_position;
                mb2binfo.Framebuffer[fb_count].GreenMaskSize = fb->framebuffer_green_mask_size;
                mb2binfo.Framebuffer[fb_count].GreenMaskShift = fb->framebuffer_green_field_position;
                mb2binfo.Framebuffer[fb_count].BlueMaskSize = fb->framebuffer_blue_mask_size;
                mb2binfo.Framebuffer[fb_count].BlueMaskShift = fb->framebuffer_blue_field_position;
                break;
            }
            case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
            {
                fixme("ega_text");
                break;
            }
            }
            debug("Framebuffer %d: %dx%d %d bpp", Tag, fb->common.framebuffer_width, fb->common.framebuffer_height, fb->common.framebuffer_bpp);
            debug("More info:\nAddress: %p\nPitch: %lld\nMemoryModel: %d\nRedMaskSize: %d\nRedMaskShift: %d\nGreenMaskSize: %d\nGreenMaskShift: %d\nBlueMaskSize: %d\nBlueMaskShift: %d",
                  fb->common.framebuffer_addr, fb->common.framebuffer_pitch, fb->common.framebuffer_type,
                  fb->framebuffer_red_mask_size, fb->framebuffer_red_field_position, fb->framebuffer_green_mask_size,
                  fb->framebuffer_green_field_position, fb->framebuffer_blue_mask_size, fb->framebuffer_blue_field_position);
            fb_count++;
            break;
        }
        case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:
        {
            multiboot_tag_elf_sections *elf = (multiboot_tag_elf_sections *)Tag;
            fixme("elf_sections->[num=%d, size=%d, entsize=%d, shndx=%d]",
                  elf->num, elf->size, elf->entsize, elf->shndx);
            break;
        }
        case MULTIBOOT_TAG_TYPE_APM:
        {
            multiboot_tag_apm *apm = (multiboot_tag_apm *)Tag;
            fixme("apm->[version: %d, cseg: %d, offset: %d, cseg_16: %d, dseg: %d, flags: %d, cseg_len: %d, cseg_16_len: %d, dseg_len: %d]",
                  apm->version, apm->cseg, apm->offset, apm->cseg_16, apm->dseg, apm->flags, apm->cseg_len, apm->cseg_16_len, apm->dseg_len);
            break;
        }
        case MULTIBOOT_TAG_TYPE_EFI32:
        {
            multiboot_tag_efi32 *efi32 = (multiboot_tag_efi32 *)Tag;
            fixme("efi32->[pointer: %p, size: %d]", efi32->pointer, efi32->size);
            break;
        }
        case MULTIBOOT_TAG_TYPE_EFI64:
        {
            multiboot_tag_efi64 *efi64 = (multiboot_tag_efi64 *)Tag;
            fixme("efi64->[pointer: %p, size: %d]", efi64->pointer, efi64->size);
            break;
        }
        case MULTIBOOT_TAG_TYPE_SMBIOS:
        {
            multiboot_tag_smbios *smbios = (multiboot_tag_smbios *)Tag;
            fixme("smbios->[major: %d, minor: %d]", smbios->major, smbios->minor);
            break;
        }
        case MULTIBOOT_TAG_TYPE_ACPI_OLD:
        {
            mb2binfo.RSDP = (BootInfo::RSDPInfo *)((multiboot_tag_old_acpi *)Tag)->rsdp;
            debug("OLD ACPI RSDP: %p", mb2binfo.RSDP);
            break;
        }
        case MULTIBOOT_TAG_TYPE_ACPI_NEW:
        {
            mb2binfo.RSDP = (BootInfo::RSDPInfo *)((multiboot_tag_new_acpi *)Tag)->rsdp;
            debug("NEW ACPI RSDP: %p", mb2binfo.RSDP);
            break;
        }
        case MULTIBOOT_TAG_TYPE_NETWORK:
        {
            multiboot_tag_network *net = (multiboot_tag_network *)Tag;
            fixme("network->[dhcpack: %p]", net->dhcpack);
            break;
        }
        case MULTIBOOT_TAG_TYPE_EFI_MMAP:
        {
            multiboot_tag_efi_mmap *efi_mmap = (multiboot_tag_efi_mmap *)Tag;
            fixme("efi_mmap->[descr_size: %d, descr_vers: %d, efi_mmap: %p]",
                  efi_mmap->descr_size, efi_mmap->descr_vers, efi_mmap->efi_mmap);
            break;
        }
        case MULTIBOOT_TAG_TYPE_EFI_BS:
        {
            fixme("efi_bs->[%p] (unknown structure)", Tag);
            break;
        }
        case MULTIBOOT_TAG_TYPE_EFI32_IH:
        {
            multiboot_tag_efi32_ih *efi32_ih = (multiboot_tag_efi32_ih *)Tag;
            fixme("efi32_ih->[pointer: %p]", efi32_ih->pointer);
            break;
        }
        case MULTIBOOT_TAG_TYPE_EFI64_IH:
        {
            multiboot_tag_efi64_ih *efi64_ih = (multiboot_tag_efi64_ih *)Tag;
            fixme("efi64_ih->[pointer: %p]", efi64_ih->pointer);
            break;
        }
        case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR:
        {
            multiboot_tag_load_base_addr *load_base_addr = (multiboot_tag_load_base_addr *)Tag;
            mb2binfo.Kernel.PhysicalBase = (void *)load_base_addr->load_base_addr;
            mb2binfo.Kernel.VirtualBase = (void *)(load_base_addr->load_base_addr + 0xC0000000);
            debug("Kernel base: %p (physical) %p (virtual)", mb2binfo.Kernel.PhysicalBase, mb2binfo.Kernel.VirtualBase);
            break;
        }
        default:
        {
            error("Unknown multiboot2 tag type: %d", Tag->type);
            break;
        }
        }
    }
}

EXTERNC void Multiboot2Entry(unsigned long Info, unsigned int Magic)
{
    if (Info == NULL || Magic == NULL)
    {
        if (Magic == NULL)
            error("Multiboot magic is NULL");
        if (Info == NULL)
            error("Multiboot info is NULL");
        CPU::Stop();
    }
    else if (Magic != MULTIBOOT2_BOOTLOADER_MAGIC)
    {
        error("Multiboot magic is invalid (%#x != %#x)", Magic, MULTIBOOT2_BOOTLOADER_MAGIC);
        CPU::Stop();
    }

    uint64_t div = 1193180 / 1000;
    outb(0x43, 0xB6);
    outb(0x42, (uint8_t)div);
    outb(0x42, (uint8_t)(div >> 8));
    uint8_t tmp = inb(0x61);
    if (tmp != (tmp | 3))
        outb(0x61, tmp | 3);

    ProcessMB2(Info);

    tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);

    CPU::Stop();
    Entry(&mb2binfo);
}
