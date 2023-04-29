#include <multiboot2.h>
#include <memory.hpp>
#include <binfo.h>
#include <debug.h>
#include <uart.hpp>

#include "loadelf.hpp"

using namespace UniversalAsynchronousReceiverTransmitter;

BootInfo mb2binfo;

bool DoNotWriteOnScreen = false;
extern "C" void terminal_initialize(void);
extern "C" void terminal_putchar(char c);

extern "C" int DetectCPUID();
extern "C" int Detect64Bit();
extern "C" void EnablePaging();

char *strncpy(char *destination, const char *source, unsigned long num)
{
    if (destination == NULL)
        return NULL;
    char *ptr = destination;
    while (*source && num--)
    {
        *destination = *source;
        destination++;
        source++;
    }
    *destination = '\0';
    return ptr;
}

long unsigned strlen(const char s[])
{
    long unsigned i = 0;
    if (s)
        while (s[i] != '\0')
            ++i;
    return i;
}

int strcmp(const char *l, const char *r)
{
    for (; *l == *r && *l; l++, r++)
        ;
    return *(unsigned char *)l - *(unsigned char *)r;
}

extern "C" void putchar(char c)
{
    if (mb2binfo.Framebuffer[0].Type == FramebufferType::EGA && !DoNotWriteOnScreen)
        terminal_putchar(c);
    UART(COM1).Write(c);
}

void ProcessMB2(uint32_t Info)
{
    auto InfoAddress = Info;
    for (auto Tag = (struct multiboot_tag *)((uint8_t *)(uint64_t)InfoAddress + 8);
         ;
         Tag = (struct multiboot_tag *)((multiboot_uint8_t *)Tag + ((Tag->size + 7) & ~7)))
    {
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
            mb2binfo.Modules[module_count].Address = (void *)(uint64_t)module->mod_start;
            mb2binfo.Modules[module_count].Size = (uint64_t)module->size;
            strncpy(mb2binfo.Modules[module_count].Path, "(null)", 6);
            strncpy(mb2binfo.Modules[module_count].CommandLine, module->cmdline,
                    strlen(module->cmdline));
            debug("Module: %s %s %p %lld", mb2binfo.Modules[module_count].Path,
                  mb2binfo.Modules[module_count].CommandLine,
                  mb2binfo.Modules[module_count].Address,
                  mb2binfo.Modules[module_count].Size);
            module_count++;
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
                    debug("Available memory: %#llx - %#llx", entry.addr, entry.addr + entry.len);
                    break;
                case MULTIBOOT_MEMORY_RESERVED:
                    mb2binfo.Memory.Entry[i].BaseAddress = (void *)entry.addr;
                    mb2binfo.Memory.Entry[i].Length = entry.len;
                    mb2binfo.Memory.Entry[i].Type = Reserved;
                    debug("Reserved memory: %#llx - %#llx", entry.addr, entry.addr + entry.len);
                    break;
                case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
                    mb2binfo.Memory.Entry[i].BaseAddress = (void *)entry.addr;
                    mb2binfo.Memory.Entry[i].Length = entry.len;
                    mb2binfo.Memory.Entry[i].Type = ACPIReclaimable;
                    debug("ACPI reclaimable memory: %#llx - %#llx", entry.addr, entry.addr + entry.len);
                    break;
                case MULTIBOOT_MEMORY_NVS:
                    mb2binfo.Memory.Entry[i].BaseAddress = (void *)entry.addr;
                    mb2binfo.Memory.Entry[i].Length = entry.len;
                    mb2binfo.Memory.Entry[i].Type = ACPINVS;
                    debug("ACPI NVS memory: %#llx - %#llx", entry.addr, entry.addr + entry.len);
                    break;
                case MULTIBOOT_MEMORY_BADRAM:
                    mb2binfo.Memory.Entry[i].BaseAddress = (void *)entry.addr;
                    mb2binfo.Memory.Entry[i].Length = entry.len;
                    mb2binfo.Memory.Entry[i].Type = BadMemory;
                    debug("Bad memory: %#llx - %#llx", entry.addr, entry.addr + entry.len);
                    break;
                default:
                    mb2binfo.Memory.Entry[i].BaseAddress = (void *)entry.addr;
                    mb2binfo.Memory.Entry[i].Length = entry.len;
                    mb2binfo.Memory.Entry[i].Type = Unknown;
                    debug("Unknown memory: %#llx - %#llx", entry.addr, entry.addr + entry.len);
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
            mb2binfo.Framebuffer[fb_count].BaseAddress = (void *)fb->common.framebuffer_addr;
            mb2binfo.Framebuffer[fb_count].Width = fb->common.framebuffer_width;
            mb2binfo.Framebuffer[fb_count].Height = fb->common.framebuffer_height;
            mb2binfo.Framebuffer[fb_count].Pitch = fb->common.framebuffer_pitch;
            mb2binfo.Framebuffer[fb_count].BitsPerPixel = fb->common.framebuffer_bpp;
            switch (fb->common.framebuffer_type)
            {
            case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED:
            {
                mb2binfo.Framebuffer[fb_count].Type = FramebufferType::Indexed;
                break;
            }
            case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
            {
                mb2binfo.Framebuffer[fb_count].Type = FramebufferType::RGB;
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
                mb2binfo.Framebuffer[fb_count].Type = FramebufferType::EGA;
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

typedef void (*CallPtr)(void);
extern CallPtr __init_array_start[0], __init_array_end[0];
extern CallPtr __fini_array_start[0], __fini_array_end[0];

extern "C" int loader_main(uint32_t magic, uint32_t addr)
{
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
    {
        error("Multiboot magic is invalid (%#x != %#x)", magic, MULTIBOOT2_BOOTLOADER_MAGIC);
        while (1)
            asmv("cli; hlt");
    }

    for (CallPtr *func = __init_array_start; func != __init_array_end; func++)
        (*func)();

    ProcessMB2(addr);
    if (mb2binfo.Framebuffer[0].Type == FramebufferType::EGA)
        terminal_initialize();
    debug("Multiboot2 tags processed");
    InitializeMemoryManagement(&mb2binfo, true);
    EnablePaging();

    bool Load64Bit = false;

    if (DetectCPUID())
    {
        debug("CPUID supported");
        if (Detect64Bit())
        {
            info("64-bit CPU detected");
            if (strcmp(mb2binfo.Kernel.CommandLine, "no64bit") == 0)
                info("64-bit kernel disabled");
            else
                Load64Bit = true;
        }
        else
        {
            info("32-bit CPU detected");
        }
    }
    else
    {
        error("CPUID not supported");
        while (1)
            asmv("cli; hlt");
    }
    if (!LoadElfInMemory(mb2binfo.Modules[0].Address, mb2binfo.Modules[0].Size, Load64Bit))
    {
        error("Kernel not loaded");
        while (1)
            asmv("cli; hlt");
    }
    error("Unwanted reach");
    while (1)
        asmv("cli; hlt");
}
