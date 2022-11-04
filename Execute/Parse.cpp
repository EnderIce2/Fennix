#include <exec.hpp>

#include "../kernel.h"
#include "../Fex.hpp"

namespace Execute
{
    BinaryType GetBinaryType(char *Path)
    {
        BinaryType Type = BinaryType::BinTypeInvalid;
        FileSystem::FILE *ExFile = vfs->Open(Path);

        if (ExFile->Status == FileSystem::FileStatus::OK)
        {
            if (ExFile->Node->Flags == FileSystem::NodeFlags::FS_FILE)
            {
                Fex *FexHdr = (Fex *)ExFile->Node->Address;
                if (FexHdr->Magic[0] == 'F' && FexHdr->Magic[1] == 'E' && FexHdr->Magic[2] == 'X' && FexHdr->Magic[3] == '\0')
                {
                    if (FexHdr->Type == FexFormatType::FexFormatType_Executable)
                    {
                        Type = BinaryType::BinTypeFex;
                        goto Exit;
                    }
                }

                /* ... */

                Type = BinaryType::BinTypeUnknown;
            }
        }
    Exit:
        vfs->Close(ExFile);
        return Type;
    }
}
