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

#include <exec.hpp>

#include <memory.hpp>
#include <lock.hpp>
#include <msexec.h>
#include <cwalk.h>
#include <elf.h>
#include <abi.h>

#include "../kernel.h"
#include "../Fex.hpp"

using namespace Tasking;

namespace Execute
{
	SpawnData Spawn(char *Path, const char **argv, const char **envp)
	{
		SpawnData ret = {.Status = ExStatus::Unknown,
						 .Process = nullptr,
						 .Thread = nullptr};

		VirtualFileSystem::File ExFile = vfs->Open(Path);

		if (!ExFile.IsOK())
		{
			if (ExFile.Status == VirtualFileSystem::FileStatus::NotFound)
			{
				ret.Status = ExStatus::InvalidFilePath;
				goto Exit;
			}
			else
			{
				ret.Status = ExStatus::InvalidFile;
				goto Exit;
			}

			if (ExFile.GetFlags() != VirtualFileSystem::NodeFlags::FILE)
			{
				ret.Status = ExStatus::InvalidFilePath;
				goto Exit;
			}
		}

		switch (GetBinaryType(Path))
		{
		case BinaryType::BinTypeFex:
		{
			Fex FexHdr;
			vfs->Read(ExFile, (uint8_t *)&FexHdr, sizeof(Fex));
			if (FexHdr.Type == FexFormatType::FexFormatType_Executable)
			{
				stub;
				assert(false);
			}

			ret.Status = ExStatus::InvalidFileHeader;
			break;
		}
		case BinaryType::BinTypeELF:
		{
			ELFBaseLoad bl = ELFLoad(Path, argv, envp);
			if (!bl.Success)
			{
				ret.Status = ExStatus::LoadingProcedureFailed;
				break;
			}
			ret = bl.sd;
			break;
		}
		default:
		{
			ret.Status = ExStatus::Unsupported;
			break;
		}
		}

	Exit:
		vfs->Close(ExFile);
		return ret;
	}
}
