#include <types.h>

#include <boot/protocols/multiboot2.h>
#include <io.h>

#include "../../kernel.h"

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

struct multiboot_info
{
    multiboot_uint32_t Size;
    multiboot_uint32_t Reserved;
    struct multiboot_tag *Tag;
};

EXTERNC void x32Multiboot2Entry(multiboot_info *Info, unsigned int Magic)
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
        trace("Hello, World!");
        CPU::Stop();
    }

    uint64_t div = 1193180 / 1000;
    outb(0x43, 0xB6);
    outb(0x42, (uint8_t)div);
    outb(0x42, (uint8_t)(div >> 8));
    uint8_t tmp = inb(0x61);
    if (tmp != (tmp | 3))
        outb(0x61, tmp | 3);

    BootInfo binfo;
    uint32_t Itr = 0;

    for (uint32_t i = 8; i < Info->Size; i += Itr)
    {
        multiboot_tag *Tag = (multiboot_tag *)((uint8_t *)Info + i);
        if (Tag->type == MULTIBOOT_TAG_TYPE_END)
            break;

        switch (Tag->type)
        {
        case MULTIBOOT_TAG_TYPE_CMDLINE:
        {
            strcpy(binfo.Kernel.CommandLine, ((multiboot_tag_string *)Tag)->string);
            break;
        }
        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
        {
            strcpy(binfo.Bootloader.Name, ((multiboot_tag_string *)Tag)->string);
            break;
        }
        case MULTIBOOT_TAG_TYPE_MODULE:
        {
            multiboot_tag_module *module = (multiboot_tag_module *)Tag;
            static int module_count = 0;
            binfo.Modules[module_count++].Address = (void *)module->mod_start;
            binfo.Modules[module_count++].Size = module->size;
            strcpy(binfo.Modules[module_count++].Path, "(null)");
            strcpy(binfo.Modules[module_count++].CommandLine, module->cmdline);
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

            binfo.Memory.Entries = EntryCount;
            for (uint32_t i = 0; i < EntryCount; i++)
            {
                if (EntryCount > MAX_MEMORY_ENTRIES)
                {
                    warn("Too many memory entries, skipping the rest...");
                    break;
                }

                multiboot_mmap_entry entry = mmap->entries[i];
                binfo.Memory.Size += entry.len;
                switch (entry.type)
                {
                case MULTIBOOT_MEMORY_AVAILABLE:
                    binfo.Memory.Entry[i].BaseAddress = (void *)entry.addr;
                    binfo.Memory.Entry[i].Length = entry.len;
                    binfo.Memory.Entry[i].Type = Usable;
                    break;
                case MULTIBOOT_MEMORY_RESERVED:
                    binfo.Memory.Entry[i].BaseAddress = (void *)entry.addr;
                    binfo.Memory.Entry[i].Length = entry.len;
                    binfo.Memory.Entry[i].Type = Reserved;
                    break;
                case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
                    binfo.Memory.Entry[i].BaseAddress = (void *)entry.addr;
                    binfo.Memory.Entry[i].Length = entry.len;
                    binfo.Memory.Entry[i].Type = ACPIReclaimable;
                    break;
                case MULTIBOOT_MEMORY_NVS:
                    binfo.Memory.Entry[i].BaseAddress = (void *)entry.addr;
                    binfo.Memory.Entry[i].Length = entry.len;
                    binfo.Memory.Entry[i].Type = ACPINVS;
                    break;
                case MULTIBOOT_MEMORY_BADRAM:
                    binfo.Memory.Entry[i].BaseAddress = (void *)entry.addr;
                    binfo.Memory.Entry[i].Length = entry.len;
                    binfo.Memory.Entry[i].Type = BadMemory;
                    break;
                default:
                    binfo.Memory.Entry[i].BaseAddress = (void *)entry.addr;
                    binfo.Memory.Entry[i].Length = entry.len;
                    binfo.Memory.Entry[i].Type = Unknown;
                    break;
                }
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

            binfo.Framebuffer[fb_count].BaseAddress = (void *)fb->common.framebuffer_addr;
            binfo.Framebuffer[fb_count].Width = fb->common.framebuffer_width;
            binfo.Framebuffer[fb_count].Height = fb->common.framebuffer_height;
            binfo.Framebuffer[fb_count].Pitch = fb->common.framebuffer_pitch;
            binfo.Framebuffer[fb_count].BitsPerPixel = fb->common.framebuffer_bpp;
            binfo.Framebuffer[fb_count].MemoryModel = fb->common.framebuffer_type;

            switch (fb->common.framebuffer_type)
            {
            case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED:
            {
                fixme("indexed");
                break;
            }
            case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
            {
                binfo.Framebuffer[fb_count].RedMaskSize = fb->framebuffer_red_mask_size;
                binfo.Framebuffer[fb_count].RedMaskShift = fb->framebuffer_red_field_position;
                binfo.Framebuffer[fb_count].GreenMaskSize = fb->framebuffer_green_mask_size;
                binfo.Framebuffer[fb_count].GreenMaskShift = fb->framebuffer_green_field_position;
                binfo.Framebuffer[fb_count].BlueMaskSize = fb->framebuffer_blue_mask_size;
                binfo.Framebuffer[fb_count].BlueMaskShift = fb->framebuffer_blue_field_position;
                break;
            }
            case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
            {
                fixme("ega_text");
                break;
            }
            }
            debug("Framebuffer %d: %dx%d %d bpp", i, fb->common.framebuffer_width, fb->common.framebuffer_height, fb->common.framebuffer_bpp);
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
            binfo.RSDP = (BootInfo::RSDPInfo *)((multiboot_tag_old_acpi *)Tag)->rsdp;
            break;
        }
        case MULTIBOOT_TAG_TYPE_ACPI_NEW:
        {
            binfo.RSDP = (BootInfo::RSDPInfo *)((multiboot_tag_new_acpi *)Tag)->rsdp;
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
            binfo.Kernel.PhysicalBase = (void *)load_base_addr->load_base_addr;
            binfo.Kernel.VirtualBase = (void *)(load_base_addr->load_base_addr + 0xC0000000);
            break;
        }
        }
        Itr = Tag->size;
        if ((Itr % 8) != 0)
            Itr += (8 - Itr % 8);
    }

    tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);

    int *vm = (int *)0xb8000;
    // "Not supported yet"
    vm[0] = 0x054E;
    vm[1] = 0x056F;
    vm[2] = 0x0574;
    vm[3] = 0x0520;
    vm[4] = 0x0573;
    vm[5] = 0x0575;
    vm[6] = 0x0570;
    vm[7] = 0x0570;
    vm[8] = 0x0572;
    vm[9] = 0x056F;
    vm[10] = 0x0574;
    vm[11] = 0x0520;
    vm[12] = 0x0579;
    vm[13] = 0x0565;
    vm[14] = 0x0574;
    
    CPU::Stop();
    // Entry(&binfo);
}
