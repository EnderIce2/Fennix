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

#include <kshell.hpp>

#include <filesystem.hpp>
#include <driver.hpp>
#include <lock.hpp>
#include <debug.h>

#include "../kernel.h"
#include "../driver.h"
#include "cmds.hpp"

NewLock(ShellLock);
struct Command
{
	const char *Name;
	void (*Function)(const char *);
};

static Command commands[] = {
	{"lsof", cmd_lsof},
	{"ls", cmd_ls},
	{"tree", cmd_tree},
	{"cd", cmd_cd},
	{"cat", cmd_cat},
	{"echo", cmd_echo},
	{"mkdir", nullptr},
	{"touch", nullptr},
	{"rm", nullptr},
	{"rmdir", nullptr},
	{"mv", nullptr},
	{"cp", nullptr},
	{"clear", cmd_clear},
	{"help", nullptr},
	{"exit", cmd_exit},
	{"reboot", cmd_reboot},
	{"shutdown", cmd_shutdown},
	{"ps", cmd_ps},
	{"kill", cmd_kill},
	{"killall", cmd_killall},
	{"top", cmd_top},
	{"mem", cmd_mem},
	{"mount", nullptr},
	{"umount", nullptr},
	{"uname", cmd_uname},
	{"whoami", cmd_whoami},
	{"passwd", nullptr},
	{"su", nullptr},
	{"login", nullptr},
	{"logout", nullptr},
	{"uptime", cmd_uptime},
	{"chown", nullptr},
	{"chgrp", nullptr},
	{"chmod", nullptr},
	{"chroot", nullptr},
	{"lspci", cmd_lspci},
	{"lsacpi", cmd_lsacpi},
	{"lsmod", cmd_lsmod},
	{"modinfo", cmd_modinfo},
	{"insmod", nullptr},
	{"rmmod", nullptr},
	{"modprobe", nullptr},
	{"depmod", nullptr},
	{"panic", cmd_panic},
};

void StartKernelShell()
{
	if (ShellLock.Locked())
		return;
	SmartLock(ShellLock);

	debug("Starting kernel shell...");
	KPrint("Starting kernel shell...");
	thisThread->SetPriority(Tasking::TaskPriority::High);

	std::string Buffer;
	std::vector<std::string *> History;
	size_t HistoryIndex = 0;
	bool CtrlDown = false;
	bool UpperCase = false;
	bool TabDoublePress = false;

	int kfd = fopen("/dev/key", "r");
	if (kfd < 0)
	{
		KPrint("Failed to open keyboard device! %s",
			   strerror(kfd));
		return;
	}

	printf("Using \eCA21F6/dev/key\eCCCCCC for keyboard input.\n");
	while (true)
	{
		size_t BackspaceCount = 0;
		Buffer.clear();

		vfs::Node *cwd = thisProcess->CurrentWorkingDirectory;
		if (!cwd)
			cwd = fs->GetNodeFromPath("/");

		printf("\e34C6EB%s@%s:%s$ \eCCCCCC",
			   "kernel",
			   "fennix",
			   cwd->FullPath);
		Display->UpdateBuffer();

		uint8_t scBuf[2];
		scBuf[1] = 0x00; /* Request scan code */
		ssize_t nBytes;
		while (true)
		{
			nBytes = fread(kfd, scBuf, 2);
			if (nBytes == 0)
				continue;
			if (nBytes < 0)
			{
				KPrint("Failed to read from keyboard device: %s",
					   strerror((int)nBytes));
				return;
			}

			if (scBuf[0] == 0x00)
				continue;

			uint8_t sc = scBuf[0];
			switch (sc & ~KEY_PRESSED)
			{
			case KEY_LEFT_CTRL:
			case KEY_RIGHT_CTRL:
			{
				if (sc & KEY_PRESSED)
					CtrlDown = true;
				else
					CtrlDown = false;
				continue;
			}
			case KEY_LEFT_SHIFT:
			case KEY_RIGHT_SHIFT:
			{
				if (sc & KEY_PRESSED)
					UpperCase = true;
				else
					UpperCase = false;
				continue;
			}
			case KEY_BACKSPACE:
			{
				if (!(sc & KEY_PRESSED))
					continue;

				if (BackspaceCount == 0)
					continue;

				Display->Print('\b');
				Buffer.pop_back();
				BackspaceCount--;
				Display->UpdateBuffer();
				continue;
			}
			case KEY_UP_ARROW:
			{
				if (!(sc & KEY_PRESSED))
					continue;

				if (History.size() == 0 ||
					HistoryIndex == 0)
					continue;

				HistoryIndex--;

				for (size_t i = 0; i < Buffer.size(); i++)
					Display->Print('\b');
				Display->UpdateBuffer();

				Buffer = History[HistoryIndex]->c_str();

				for (size_t i = 0; i < strlen(Buffer.c_str()); i++)
					Display->Print(Buffer[i]);
				BackspaceCount = Buffer.size();
				Display->UpdateBuffer();
				continue;
			}
			case KEY_DOWN_ARROW:
			{
				if (!(sc & KEY_PRESSED))
					continue;

				if (History.size() == 0 ||
					HistoryIndex == History.size())
					continue;

				if (HistoryIndex == History.size() - 1)
				{
					HistoryIndex++;
					for (size_t i = 0; i < Buffer.size(); i++)
						Display->Print('\b');
					BackspaceCount = Buffer.size();
					Display->UpdateBuffer();
					continue;
				}

				for (size_t i = 0; i < Buffer.size(); i++)
					Display->Print('\b');
				Display->UpdateBuffer();

				HistoryIndex++;
				Buffer = History[HistoryIndex]->c_str();

				for (size_t i = 0; i < strlen(Buffer.c_str()); i++)
					Display->Print(Buffer[i]);

				BackspaceCount = Buffer.size();
				Display->UpdateBuffer();
				continue;
			}
			case KEY_TAB:
			{
				if (!(sc & KEY_PRESSED))
					continue;

				if (!TabDoublePress)
				{
					TabDoublePress = true;
					continue;
				}
				TabDoublePress = false;
				if (Buffer.size() == 0)
				{
					for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
						printf("%s ", commands[i].Name);

					Display->Print('\n');
					Display->UpdateBuffer();
					goto SecLoopEnd;
				}

				for (size_t i = 0; i < Buffer.size(); i++)
					Display->Print('\b');
				Display->UpdateBuffer();

				for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
				{
					if (strncmp(Buffer.c_str(), commands[i].Name, Buffer.size()) == 0)
					{
						Buffer = commands[i].Name;
						for (size_t i = 0; i < strlen(Buffer.c_str()); i++)
							Display->Print(Buffer[i]);
						BackspaceCount = Buffer.size();
						Display->UpdateBuffer();
						break;
					}
				}
				continue;
			}
			default:
				break;
			}

			if (!(sc & KEY_PRESSED))
				continue;

			if (!Driver::IsValidChar(sc))
				continue;

			char c = Driver::GetScanCode(sc, UpperCase);

			if (CtrlDown)
			{
				switch (std::toupper((char)c))
				{
				case 'C':
				{
					Display->Print('^');
					Display->Print('C');
					Display->Print('\n');
					fixme("No SIGINT handler yet.");
					Display->UpdateBuffer();
					goto SecLoopEnd;
				}
				case 'D':
				{
					Display->Print('^');
					Display->Print('D');
					Display->Print('\n');
					fixme("No SIGKILL handler yet.");
					Display->UpdateBuffer();
					goto SecLoopEnd;
				}
				default:
					continue;
				}
			}

			Display->Print(c);
			if (c == '\n')
			{
				if (Buffer.length() > 0)
				{
					std::string *hBuff = new std::string(Buffer.c_str());
					History.push_back(hBuff);
					HistoryIndex = History.size();
				}
				break;
			}

			Buffer += c;
			BackspaceCount++;
			Display->UpdateBuffer();
		}
	SecLoopEnd:

		if (Buffer.length() == 0)
			continue;

		bool Found = false;
		for (size_t i = 0; i < sizeof(commands) / sizeof(Command); i++)
		{
			std::string cmd_extracted;
			for (size_t i = 0; i < Buffer.length(); i++)
			{
				if (Buffer[i] == ' ')
					break;
				cmd_extracted += Buffer[i];
			}

			// debug("cmd: %s, array[%d]: %s", cmd_extracted.c_str(), i, commands[i].Name);
			if (strncmp(commands[i].Name, cmd_extracted.c_str(), cmd_extracted.size()) == 0)
			{
				if (strlen(commands[i].Name) != cmd_extracted.size())
					continue;

				Found = true;

				std::string arg_only;
				const char *cmd_name = commands[i].Name;
				for (size_t i = strlen(cmd_name) + 1; i < Buffer.length(); i++)
					arg_only += Buffer[i];

				if (commands[i].Function)
					commands[i].Function(arg_only.c_str());
				else
				{
					std::string cmd_only;
					for (size_t i = 0; i < Buffer.length(); i++)
					{
						if (Buffer[i] == ' ')
							break;
						cmd_only += Buffer[i];
					}
					printf("%s: command not implemented\n",
						   cmd_only.c_str());
				}
				break;
			}
		}

		if (!Found)
		{
			std::string cmd_only;
			for (size_t i = 0; i < Buffer.length(); i++)
			{
				if (Buffer[i] == ' ')
					break;
				cmd_only += Buffer[i];
			}
			printf("%s: command not found\n",
				   cmd_only.c_str());
		}
	}
}

void KShellThread()
{
	StartKernelShell();
	inf_loop;
}
