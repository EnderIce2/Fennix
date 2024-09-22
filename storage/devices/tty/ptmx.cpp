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

#include <filesystem/mounts.hpp>
#include <filesystem/ioctl.hpp>
#include <smp.hpp>
#include <errno.h>

#include "../../../kernel.h"

namespace vfs
{
	int PTMXDevice::Open(Inode *Node, int Flags, mode_t Mode, struct Inode *Result)
	{
		// SmartLock(PTMXLock);

		// int ptyID = -1;
		// for (int i = 0; i < (int)ptysId.Size; i++)
		// {
		// 	if (ptysId.Buffer[i])
		// 		continue;

		// 	ptyID = i;
		// 	ptysId.Buffer[i] = true;
		// 	break;
		// }

		// if (ptyID == -1)
		// 	return -ENFILE;

		// PTYDevice *pty = new PTYDevice(pts, ptyID);
		// ptysList.insert(std::make_pair(ptyID, pty));
		// // return pty->OpenMaster(Flags, Mode);
		assert(!"Function not implemented");
	}

	int PTMXDevice::Close(struct Inode *Node)
	{
		SmartLock(PTMXLock);

		PTYDevice *pty = ptysList.at(Node->Index);
		ptysList.erase(Node->Index);
		assert(!"Function not implemented");
	}

	PTMXDevice::PTMXDevice()
	{
		fixme("PTMXDevice");
		// /* c rw- rw- rw- */
		// mode_t mode = S_IRUSR | S_IWUSR |
		// 			  S_IRGRP | S_IWGRP |
		// 			  S_IROTH | S_IWOTH |
		// 			  S_IFCHR;

		// ptmx = fs->Create(ptmx, "pts", mode);
		// assert(!"Function not implemented");
		// // ptmx->SetDevice(5, 2);

		// /* d rwx r-x r-x */
		// mode_t ptsMode = S_IRWXU |
		// 				 S_IRGRP | S_IXGRP |
		// 				 S_IROTH | S_IXOTH |
		// 				 S_IFDIR;
		// pts = fs->Create(ptmx, "pts", ptsMode);
		// assert(pts != nullptr);

		// ptysId.Buffer = new uint8_t[0x1000];
		// ptysId.Size = 0x1000;
	}

	PTMXDevice::~PTMXDevice()
	{
		SmartLock(PTMXLock);
		delete[] ptysId.Buffer;
	}
}
