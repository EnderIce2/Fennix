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

#include "../Modules/PersonalSystem2/keyboard.hpp"
#include "../kernel.h"
#include "../Fex.hpp"
#include "../DAPI.hpp"
#include "cmds.hpp"

using namespace PS2Keyboard;

NewLock(ShellLock);

const char sc_ascii_low[] = {'?', '?', '1', '2', '3', '4', '5', '6',
							 '7', '8', '9', '0', '-', '=', '?', '?', 'q', 'w', 'e', 'r', 't', 'y',
							 'u', 'i', 'o', 'p', '[', ']', '?', '?', 'a', 's', 'd', 'f', 'g',
							 'h', 'j', 'k', 'l', ';', '\'', '`', '?', '\\', 'z', 'x', 'c', 'v',
							 'b', 'n', 'm', ',', '.', '/', '?', '?', '?', ' '};

const char sc_ascii_high[] = {'?', '?', '!', '@', '#', '$', '%', '^',
							  '&', '*', '(', ')', '_', '+', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y',
							  'U', 'I', 'O', 'P', '{', '}', '?', '?', 'A', 'S', 'D', 'F', 'G',
							  'H', 'J', 'K', 'L', ';', '\"', '~', '?', '|', 'Z', 'X', 'C', 'V',
							  'B', 'N', 'M', '<', '>', '?', '?', '?', '?', ' '};

static int LowerCase = true;

char GetLetterFromScanCode(uint8_t ScanCode)
{
	if (ScanCode & 0x80)
	{
		switch (ScanCode)
		{
		case KEY_U_LSHIFT:
			LowerCase = true;
			return 0;
		case KEY_U_RSHIFT:
			LowerCase = true;
			return 0;
		default:
			return 0;
		}
	}
	else
	{
		switch (ScanCode)
		{
		case KEY_D_RETURN:
			return '\n';
		case KEY_D_LSHIFT:
			LowerCase = false;
			return 0;
		case KEY_D_RSHIFT:
			LowerCase = false;
			return 0;
		case KEY_D_BACKSPACE:
			return ScanCode;
		default:
		{
			if (ScanCode > 0x39)
				break;
			if (LowerCase)
				return sc_ascii_low[ScanCode];
			else
				return sc_ascii_high[ScanCode];
		}
		}
	}
	return 0;
}

int GetChar()
{
	return 0;
}

struct Command
{
	const char *Name;
	void (*Function)(const char *);
};

static Command commands[] = {
	{"lsof", cmd_lsof},
	{"ls", cmd_ls},
	{"cd", cmd_cd},
	{"cat", cmd_cat},
	{"echo", cmd_echo},
	{"mkdir", nullptr},
	{"touch", nullptr},
	{"rm", nullptr},
	{"rmdir", nullptr},
	{"mv", nullptr},
	{"cp", nullptr},
	{"clear", nullptr},
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
	{"lspci", cmd_lspci}};

void StartKernelShell()
{
	SmartLock(ShellLock);

	debug("Starting kernel shell...");
	printf("Starting kernel shell...\n");
	thisThread->SetPriority(Tasking::TaskPriority::High);
	Display->SetBuffer(0);

	Driver::DriverFile KeyboardModule;
	if (likely(DriverManager->GetDrivers().size() > 0))
	{
		foreach (auto Driver in DriverManager->GetDrivers())
		{
			if (((FexExtended *)Driver.ExtendedHeaderAddress)->Driver.Type == FexDriverType::FexDriverType_Input &&
				((FexExtended *)Driver.ExtendedHeaderAddress)->Driver.TypeFlags & FexDriverInputTypes::FexDriverInputTypes_Keyboard)
			{
				KeyboardModule = Driver;
				printf("Using driver \eCA21F6%s\eCCCCCC for keyboard input.\n",
					   ((FexExtended *)Driver.ExtendedHeaderAddress)->Driver.Name);
				break;
			}
		}
	}

	Display->SetBuffer(0);
	std::string Buffer;
	std::vector<std::string *> History;
	size_t HistoryIndex = 0;
	bool CtrlDown = false;
	bool TabDoublePress = false;

	while (true)
	{
		size_t BackspaceCount = 0;
		Buffer.clear();

		VirtualFileSystem::Node *cwd = thisProcess->CurrentWorkingDirectory;
		if (!cwd)
			cwd = vfs->GetNodeFromPath("/");

		printf("\e34C6EB%s@%s:%s$ \eCCCCCC",
			   "kernel",
			   "fennix",
			   cwd->FileSystem->GetPathFromNode(cwd).c_str());
		Display->SetBuffer(0);

		while (true)
		{
			KernelCallback callback{};
			callback.Reason = PollWaitReason;
			DriverManager->IOCB(KeyboardModule.DriverUID, &callback);
			char c = GetLetterFromScanCode(callback.InputCallback.Keyboard.Key);

			switch (callback.InputCallback.Keyboard.Key)
			{
			case KEY_D_LCTRL:
			{
				CtrlDown = true;
				continue;
			}
			case KEY_U_LCTRL:
			{
				CtrlDown = false;
				continue;
			}
			case KEY_D_BACKSPACE:
			{
				if (BackspaceCount == 0)
					continue;

				Display->Print('\b', 0);
				Buffer.pop_back();
				BackspaceCount--;
				Display->SetBuffer(0);
				continue;
			}
			case KEY_D_UP:
			{
				if (History.size() == 0 ||
					HistoryIndex == 0)
					continue;

				HistoryIndex--;

				for (size_t i = 0; i < Buffer.size(); i++)
					Display->Print('\b', 0);
				Display->SetBuffer(0);

				Buffer = History[HistoryIndex]->c_str();

				for (size_t i = 0; i < strlen(Buffer.c_str()); i++)
					Display->Print(Buffer[i], 0);
				BackspaceCount = Buffer.size();
				Display->SetBuffer(0);
				continue;
			}
			case KEY_D_DOWN:
			{
				if (History.size() == 0 ||
					HistoryIndex == History.size())
					continue;

				if (HistoryIndex == History.size() - 1)
				{
					HistoryIndex++;
					for (size_t i = 0; i < Buffer.size(); i++)
						Display->Print('\b', 0);
					BackspaceCount = Buffer.size();
					Display->SetBuffer(0);
					continue;
				}

				for (size_t i = 0; i < Buffer.size(); i++)
					Display->Print('\b', 0);
				Display->SetBuffer(0);

				HistoryIndex++;
				Buffer = History[HistoryIndex]->c_str();

				for (size_t i = 0; i < strlen(Buffer.c_str()); i++)
					Display->Print(Buffer[i], 0);

				BackspaceCount = Buffer.size();
				Display->SetBuffer(0);
				continue;
			}
			case KEY_D_TAB:
			{
				if (!TabDoublePress)
				{
					TabDoublePress = true;
					continue;
				}
				TabDoublePress = false;
				if (Buffer.size() == 0)
				{
					for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
					{
						printf("%s ", commands[i].Name);
						Display->SetBuffer(0);
					}
					Display->Print('\n', 0);
					Display->SetBuffer(0);
					goto SecLoopEnd;
				}

				for (size_t i = 0; i < Buffer.size(); i++)
					Display->Print('\b', 0);
				Display->SetBuffer(0);

				for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
				{
					if (strncmp(Buffer.c_str(), commands[i].Name, Buffer.size()) == 0)
					{
						Buffer = commands[i].Name;
						for (size_t i = 0; i < strlen(Buffer.c_str()); i++)
							Display->Print(Buffer[i], 0);
						BackspaceCount = Buffer.size();
						Display->SetBuffer(0);
						break;
					}
				}
				continue;
			}
			default:
				break;
			}

			if (c == 0)
				continue;

			if (CtrlDown)
			{
				switch (std::toupper((char)c))
				{
				case 'C':
				{
					Display->Print('^', 0);
					Display->Print('C', 0);
					Display->Print('\n', 0);
					fixme("No SIGINT handler yet.");
					Display->SetBuffer(0);
					goto SecLoopEnd;
				}
				case 'D':
				{
					Display->Print('^', 0);
					Display->Print('D', 0);
					Display->Print('\n', 0);
					fixme("No SIGKILL handler yet.");
					Display->SetBuffer(0);
					goto SecLoopEnd;
				}
				default:
					continue;
				}
			}

			Display->Print(c, 0);
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
			Display->SetBuffer(0);
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

			debug("cmd: %s, array[%d]: %s", cmd_extracted.c_str(), i, commands[i].Name);
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
