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
