#include <types.h>

EXTERNC __attribute__((no_stack_protector, section(".multiboot.text"))) void Multiboot2Initializator()
{
    for (int i = 0; i < 2000; i++)
    {
        ((unsigned int *)0xb8000)[i * 2] = ' ';
        ((unsigned int *)0xb8000)[i * 2 + 1] = 0;
    }
    ((uint8_t *)0xb8000)[2 * (80) * (25) - 2] = 'W';
    ((uint8_t *)0xb8000)[2 * (80) * (25) - 1] = 6;
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
        ((uint8_t *)0xb8000)[2 * (80) * (25) - 4] = 'S';
        ((uint8_t *)0xb8000)[2 * (80) * (25) - 3] = 4;
        while (1)
            asmv("hlt");
    }

    ((uint8_t *)0xb8000)[2 * (80) * (25) - 10] = 'Y';
    ((uint8_t *)0xb8000)[2 * (80) * (25) - 9] = 2;
    return;
}
