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

#include <syscalls.hpp>

#include <memory.hpp>
#include <lock.hpp>
#include <exec.hpp>
#include <limits.h>
#include <errno.h>
#include <debug.h>

#include "../../syscalls.h"
#include "../../kernel.h"

using Tasking::PCB;
using vfs::RefNode;
using namespace Memory;

/* https://pubs.opengroup.org/onlinepubs/9699919799/functions/exec.html */
int sys_execve(SysFrm *Frame, const char *path,
			   char *const argv[], char *const envp[])
{
	PCB *pcb = thisProcess;
	Virtual vmm = Virtual(pcb->PageTable);

	if (path == nullptr ||
		!vmm.Check((void *)path, US) ||
		!vmm.Check((void *)argv, US) ||
		!vmm.Check((void *)envp, US))
		return -ENOENT;

	const char *safe_path;
	char **safe_argv;
	char **safe_envp;
	safe_path = (const char *)pcb->vma->RequestPages(1);
	safe_argv = (char **)pcb->vma->RequestPages(TO_PAGES(MAX_ARG));
	safe_envp = (char **)pcb->vma->RequestPages(TO_PAGES(MAX_ARG));
	{
		SwapPT swap(pcb->PageTable);
		size_t len = strlen(path);
		memset((void *)safe_path, 0, PAGE_SIZE);
		memcpy((void *)safe_path, path, len);

		const char *arg;
		char *n_arg;
		for (int i = 0; argv[i] != nullptr; i++)
		{
			arg = argv[i];
			size_t len = strlen(arg);

			n_arg = (char *)pcb->vma->RequestPages(TO_PAGES(len));
			memcpy((void *)n_arg, arg, len);
			n_arg[len] = '\0';

			safe_argv[i] = n_arg;

			if (likely(i < MAX_ARG - 1))
				safe_argv[i + 1] = nullptr;
		}

		for (int i = 0; envp[i] != nullptr; i++)
		{
			arg = envp[i];
			size_t len = strlen(arg);

			n_arg = (char *)pcb->vma->RequestPages(TO_PAGES(len));
			memcpy((void *)n_arg, arg, len);
			n_arg[len] = '\0';

			safe_envp[i] = n_arg;

			if (likely(i < MAX_ARG - 1))
				safe_envp[i + 1] = nullptr;
		}
	}

	function("%s %#lx %#lx", safe_path, safe_argv, safe_envp);

#ifdef DEBUG
	for (int i = 0; safe_argv[i] != nullptr; i++)
		debug("safe_argv[%d]: %s", i, safe_argv[i]);

	for (int i = 0; safe_envp[i] != nullptr; i++)
		debug("safe_envp[%d]: %s", i, safe_envp[i]);
#endif

	RefNode *File = fs->Open(safe_path,
							 pcb->CurrentWorkingDirectory);

	if (!File)
	{
		error("File not found");
		return -ENOENT;
	}

	char shebang_magic[2];
	File->read((uint8_t *)shebang_magic, 2);

	if (shebang_magic[0] == '#' && shebang_magic[1] == '!')
	{
		char *orig_path = (char *)pcb->vma->RequestPages(TO_PAGES(strlen(path) + 1));
		memcpy(orig_path, path, strlen(path) + 1);

		char *shebang = (char *)safe_path;
		size_t shebang_len = 0;
		constexpr int shebang_len_max = 255;
		File->seek(2, SEEK_SET);
		off_t shebang_off = 2;
		while (true)
		{
			char c;
			if (File->node->read((uint8_t *)&c, 1, shebang_off) == 0)
				break;
			if (c == '\n' || shebang_len == shebang_len_max)
				break;
			shebang[shebang_len++] = c;
			shebang_off++;
		}
		shebang[shebang_len] = '\0';
		debug("Shebang: %s", shebang);

		char **c_safe_argv = (char **)pcb->vma->RequestPages(TO_PAGES(MAX_ARG));
		int i = 0;
		for (; safe_argv[i] != nullptr; i++)
		{
			size_t arg_len = strlen(safe_argv[i]);
			char *c_arg = (char *)pcb->vma->RequestPages(TO_PAGES(arg_len));
			memcpy((void *)c_arg, safe_argv[i], arg_len);
			c_arg[arg_len] = '\0';

			c_safe_argv[i] = c_arg;
			debug("c_safe_argv[%d]: %s", i, c_safe_argv[i]);
		}
		c_safe_argv[i] = nullptr;

		char *token = strtok(shebang, " ");
		i = 0;
		while (token != nullptr)
		{
			size_t len = strlen(token);
			char *t_arg = (char *)pcb->vma->RequestPages(TO_PAGES(len));
			memcpy((void *)t_arg, token, len);
			t_arg[len] = '\0';

			safe_argv[i++] = t_arg;
			token = strtok(nullptr, " ");
		}

		safe_argv[i++] = orig_path;
		for (int j = 1; c_safe_argv[j] != nullptr; j++)
		{
			safe_argv[i++] = c_safe_argv[j];
			debug("clone: safe_argv[%d]: %s",
				  i, safe_argv[i - 1]);
		}
		safe_argv[i] = nullptr;

		delete File;
		return sys_execve(Frame, safe_argv[0],
						  (char *const *)safe_argv,
						  (char *const *)safe_envp);
	}

	int ret = Execute::Spawn((char *)safe_path,
							 (const char **)safe_argv,
							 (const char **)safe_envp,
							 pcb->Parent, true,
							 pcb->Info.Compatibility);

	if (ret < 0)
	{
		error("Failed to spawn");
		delete File;
		return ret;
	}

	delete File;
	Tasking::Task *ctx = pcb->GetContext();
	ctx->Sleep(1000);
	pcb->SetState(Tasking::Zombie);
	pcb->SetExitCode(0); /* FIXME: get process exit code */
	while (true)
		ctx->Yield();
	__builtin_unreachable();
}
