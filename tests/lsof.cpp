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

#ifdef DEBUG

#include "t.h"

#include "../kernel.h"

using vfs::Node;
using vfs::NodeType;

static int ShowOpenFiles = 0;

void lsof()
{
	thisThread->Rename("Debug File List");
	thisThread->SetPriority(Tasking::Idle);

	while (ShowOpenFiles == 0)
		CPU::Pause();

	thisThread->SetPriority(Tasking::High);

	fs->Create("/dummy_lsof_file", NodeType::FILE);
	fopen("/dummy_lsof_file", "r");

	while (true)
	{
		while (ShowOpenFiles == 0)
			CPU::Pause();

		Video::ScreenBuffer *sb = Display->GetBuffer(0);
		for (short i = 0; i < 500; i++)
		{
			for (short j = 0; j < 500; j++)
			{
				uint32_t *Pixel = (uint32_t *)((uintptr_t)sb->Buffer + (j * sb->Width + i) * (bInfo.Framebuffer[0].BitsPerPixel / 8));
				*Pixel = 0x222222;
			}
		}

		uint32_t tmpX, tmpY;
		Display->GetBufferCursor(0, &tmpX, &tmpY);
		Display->SetBufferCursor(0, 0, 0);
		printf("\eF02C21Open Files (%ld):\e00AAAA\n",
			   TaskManager->GetProcessList().size());
		foreach (auto Proc in TaskManager->GetProcessList())
		{
			if (!Proc)
				continue;

			printf("%s:\n", Proc->Name);

			std::vector<vfs::FileDescriptorTable::Fildes> fds_array =
				Proc->FileDescriptors->GetFileDescriptors();
			foreach (auto fd in fds_array)
				printf("  %d: %s\n", fd.Descriptor,
					   fd.Handle->node->FullPath);
		}
		Display->SetBufferCursor(0, tmpX, tmpY);
		if (!Config.BootAnimation)
			Display->SetBuffer(0);
	}
}

#endif // DEBUG
