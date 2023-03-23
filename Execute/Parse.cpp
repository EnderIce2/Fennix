#include <exec.hpp>

#include <msexec.h>

#include "../kernel.h"
#include "../Fex.hpp"

namespace Execute
{
    BinaryType GetBinaryType(void *Image)
    {
        Fex *FexHdr = (Fex *)Image;

        /* Elf64_Ehdr and Elf32_Ehdr are very similar (Elf64_Half and
           Elf32_Half are the same size type) so we can use directly Elf64_Ehdr. */
        Elf64_Ehdr *ELFHeader = (Elf64_Ehdr *)Image;

        IMAGE_DOS_HEADER *MZHeader = (IMAGE_DOS_HEADER *)Image;

        /* Check Fex magic */
        if (FexHdr->Magic[0] == 'F' && FexHdr->Magic[1] == 'E' && FexHdr->Magic[2] == 'X' && FexHdr->Magic[3] == '\0')
        {
            /* If the fex type is driver, we shouldn't return as Fex. */
            if (FexHdr->Type == FexFormatType::FexFormatType_Executable)
            {
                debug("Image - Fex");
                return BinaryType::BinTypeFex;
            }
            else if (FexHdr->Type == FexFormatType::FexFormatType_Driver)
                debug("Fex Driver is not supposed to be executed.");
        }
        /* Check ELF magic. */
        else if (ELFHeader->e_ident[EI_MAG0] == ELFMAG0 &&
                 ELFHeader->e_ident[EI_MAG1] == ELFMAG1 &&
                 ELFHeader->e_ident[EI_MAG2] == ELFMAG2 &&
                 ELFHeader->e_ident[EI_MAG3] == ELFMAG3)
        {
            debug("Image - ELF");
            return BinaryType::BinTypeELF;
        }
        /* Every Windows executable starts with MZ header. */
        else if (MZHeader->e_magic == IMAGE_DOS_SIGNATURE)
        {
            IMAGE_NT_HEADERS *PEHeader = (IMAGE_NT_HEADERS *)(((char *)Image) + MZHeader->e_lfanew);
            IMAGE_OS2_HEADER *NEHeader = (IMAGE_OS2_HEADER *)(((char *)Image) + MZHeader->e_lfanew);

            /* TODO: LE, EDOS */
            if (PEHeader->Signature == IMAGE_NT_SIGNATURE)
            {
                debug("Image - PE");
                return BinaryType::BinTypePE;
            }
            else if (NEHeader->ne_magic == IMAGE_OS2_SIGNATURE)
            {
                debug("Image - NE");
                return BinaryType::BinTypeNE;
            }
            else
            {
                debug("Image - MZ");
                return BinaryType::BinTypeMZ;
            }
        }

        /* ... */
        return BinaryType::BinTypeUnknown;
    }

    BinaryType GetBinaryType(char *Path)
    {
        BinaryType Type = BinaryType::BinTypeInvalid;
        std::shared_ptr<VirtualFileSystem::File> ExFile = vfs->Open(Path);

        if (ExFile->Status == VirtualFileSystem::FileStatus::OK)
        {
            debug("File opened: %s", Path);
            Type = GetBinaryType((void *)ExFile->node->Address);
        }

        vfs->Close(ExFile);
        return Type;
    }
}
