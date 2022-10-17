#include <types.h>

#include <boot/protocols/multiboot2.h>

struct multiboot_info
{
    multiboot_uint32_t Size;
    multiboot_uint32_t Reserved;
    struct multiboot_tag *Tag;
};

EXTERNC __attribute__((no_stack_protector, section(".mb2bootcode.text"))) void Multiboot2Initializator(multiboot_info *Info, unsigned int Magic)
{
    if (Info == NULL || Magic == NULL)
    {
        if (Magic == NULL)
        {
            ((unsigned char *)0xb8000)[2 * (80) * (25) - 4] = 'E';
            ((unsigned char *)0xb8000)[2 * (80) * (25) - 3] = 4;
        }
        if (Info == NULL)
        {
            ((unsigned char *)0xb8000)[2 * (80) * (25) - 2] = 'R';
            ((unsigned char *)0xb8000)[2 * (80) * (25) - 1] = 4;
        }
        while (1)
            asmv("hlt");
    }
    else if (Magic != MULTIBOOT2_BOOTLOADER_MAGIC)
    {
        ((unsigned char *)0xb8000)[2 * (80) * (25) - 2] = 'M';
        ((unsigned char *)0xb8000)[2 * (80) * (25) - 1] = 4;
        while (1)
            asmv("hlt");
    }

    /* TODO */

    ((unsigned char *)0xb8000)[2 * (80) * (25) - 2] = 'Y';
    ((unsigned char *)0xb8000)[2 * (80) * (25) - 1] = 2;
    while (1)
        asmv("hlt");
}
