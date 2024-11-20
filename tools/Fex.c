// FEX is a file format. Right now in development.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../Kernel/Fex.hpp"

void DumpData(const char *Description, unsigned char *Address, unsigned long Length)
{
    printf("-------------------------------------------------------------------------\n");
    unsigned char Buffer[17];
    unsigned long Iterate;

    if (Description != NULL)
        printf("%s:\n", Description);

    for (Iterate = 0; Iterate < Length; Iterate++)
    {
        if ((Iterate % 16) == 0)
        {
            if (Iterate != 0)
                printf("  %s\n", Buffer);
            printf("  %04x ", Iterate);
        }

        printf(" %02x", Address[Iterate]);

        if ((Address[Iterate] < 0x20) || (Address[Iterate] > 0x7e))
            Buffer[Iterate % 16] = '.';
        else
            Buffer[Iterate % 16] = Address[Iterate];

        Buffer[(Iterate % 16) + 1] = '\0';
    }

    while ((Iterate % 16) != 0)
    {
        printf("   ");
        Iterate++;
    }

    printf("  %s\n", Buffer);
    printf("-------------------------------------------------------------------------\n");
}

int main()
{
    FILE *FilePointer;
    char FileName[20];
    unsigned char *Buffer;
    char BufferChar;
    struct stat st;

    printf("--- FEX file viewer ---\n");
FileSelection:
    printf("File Name: ");
    fgets(FileName, 20, stdin);
    FileName[strcspn(FileName, "\r\n")] = 0;
    FilePointer = fopen(FileName, "r");

    if (NULL == FilePointer)
    {
        printf("Error opening file %s. Try again...\n", FileName);
        FileName[0] = 0;
        goto FileSelection;
    }

    stat(FileName, &st);
    Buffer = (unsigned char *)malloc(st.st_size);
    fread(Buffer, 1, st.st_size, FilePointer);
    DumpData(FileName, Buffer, st.st_size);

    struct Fex *FexFile = Buffer;

    if (FexFile->Magic[0] != 'F' || FexFile->Magic[1] != 'E' || FexFile->Magic[2] != 'X' || FexFile->Magic[3] != '\0')
    {
        printf("Invalid FEX header. Checking if it's in an ELF file...\n");
        if (st.st_size > 0x1000)
        {
            struct Fex *FexFile = (struct Fex *)(Buffer + 0x1000);
            if (FexFile->Magic[0] != 'F' || FexFile->Magic[1] != 'E' || FexFile->Magic[2] != 'X' || FexFile->Magic[3] != '\0')
            {
                printf("Invalid FEX header. Exiting...\n");
                fclose(FilePointer);
                free(Buffer);
                return 1;
            }
        }
        else
        {
            printf("Invalid FEX header. Exiting...\n");
            fclose(FilePointer);
            free(Buffer);
            return 1;
        }
    }

    struct FexExtended *FexExtendedFile = (struct FexExtended *)(Buffer + EXTENDED_SECTION_ADDRESS);

    printf("┌FEX File:\n");
    printf("├Magic: %c%c%c%c\n", FexFile->Magic[0], FexFile->Magic[1], FexFile->Magic[2], FexFile->Magic[3]);
    printf("├Type: %d\n", FexFile->Type);
    printf("├Operating System: %d\n", FexFile->OS);
    printf("├Entry Point: %#lx\n", FexFile->EntryPoint);
    printf("┊\n");
    printf("├FEX Extended Header:\n");
    printf("│ ├Executable:\n");
    printf("│ │ └<not implemented>\n");
    // TODO: Add more stuff to executable category.
    printf("│ ├Driver:\n");
    printf("│ │ ├Name: %s\n", FexExtendedFile->Driver.Name);
    printf("│ │ ├Type: %d\n", FexExtendedFile->Driver.Type);
    printf("│ │ ├Callback: %#lx\n", FexExtendedFile->Driver.Callback);
    printf("│ │ ├Bind:\n");
    printf("│ │ │ ├Type: %d\n", FexExtendedFile->Driver.Bind.Type);
    printf("│ │ │ ├Interrupt:\n");
    printf("│ │ │ │ └Vectors: ");
    for (int i = 0; i < 16; i++)
        printf("%#x ", FexExtendedFile->Driver.Bind.Interrupt.Vector[i]);
    printf("\n");
    printf("│ │ │ ├Process:\n");
    printf("│ │ │ │ └Process IDs: ");
    for (int i = 0; i < 16; i++)
        printf("%d ", FexExtendedFile->Driver.Bind.Process.ProcessId[i]);
    printf("\n");
    printf("│ │ │ ├PCI:\n");
    printf("│ │ │ │ ├Vendor IDs: ");
    for (int i = 0; i < 16; i++)
        printf("%#x ", FexExtendedFile->Driver.Bind.PCI.VendorID[i]);
    printf("\n");
    printf("│ │ │ │ ├Device IDs: ");
    for (int i = 0; i < 16; i++)
        printf("%#x ", FexExtendedFile->Driver.Bind.PCI.DeviceID[i]);
    printf("\n");
    printf("│ │ │ │ ├Class: %#x\n", FexExtendedFile->Driver.Bind.PCI.Class);
    printf("│ │ │ │ ├SubClass: %#x\n", FexExtendedFile->Driver.Bind.PCI.SubClass);
    printf("│ │ │ │ └ProgIF: %#x\n", FexExtendedFile->Driver.Bind.PCI.ProgIF);
    printf("│ │ │ ├Input:\n");
    printf("│ │ │ │ ├Attach to mouse: %s\n", FexExtendedFile->Driver.Bind.Input.AttachToMouse ? "true" : "false");
    printf("│ │ │ │ └Attach to keyboard: %s\n", FexExtendedFile->Driver.Bind.Input.AttachToKeyboard ? "true" : "false");
    printf("┊ ┊ ┊ ┊\n");

    fclose(FilePointer);
    free(Buffer);
    return 0;
}
