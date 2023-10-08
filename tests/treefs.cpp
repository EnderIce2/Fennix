#ifdef DEBUG

#include "t.h"

#include "../kernel.h"

void TreeFS(vfs::Node *node, int Depth)
{
	return;
	foreach (auto Chld in node->Children)
	{
		printf("%*c %s\eFFFFFF\n", Depth, ' ', Chld->Name);

		if (!Config.BootAnimation)
			Display->SetBuffer(0);
		TaskManager->Sleep(100);
		TreeFS(Chld, Depth + 1);
	}
}

#endif // DEBUG
