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
#include <thread>

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
	{"dump", cmd_dump},
};

std::atomic_uint32_t CurX = 0x10, CurY = 0x10;
std::atomic_bool CurBlinking = false;
std::atomic_bool CurHalt = true;
std::atomic_uint64_t BlinkerSleep = 0;

NewLock(BlinkerLock);

void PrintBlinker(uint32_t fx, uint32_t fy)
{
	for (uint32_t i = 0; i < fx; i++)
	{
		for (uint32_t j = 0; j < fy; j++)
		{
			uint32_t px = CurX.load() + i;
			uint32_t py = CurY.load() + j;
			uint32_t color = Display->GetPixel(px, py);
			Display->SetPixel(px, py, ~color);
		}
	}
}

void UpdateBlinker()
{
	if (CurBlinking.load())
	{
		SmartLock(BlinkerLock);
		uint32_t fx = 0, fy = 0;
		if (unlikely(fx == 0 || fy == 0))
		{
			fx = Display->GetCurrentFont()->GetInfo().Width;
			fy = Display->GetCurrentFont()->GetInfo().Height;
		}

		PrintBlinker(fx, fy);
		CurBlinking.store(false);
		Display->UpdateBuffer();
	}
}

void CursorBlink()
{
	uint32_t fx, fy;
	fx = Display->GetCurrentFont()->GetInfo().Width;
	fy = Display->GetCurrentFont()->GetInfo().Height;
	while (true)
	{
		if (CurHalt.load() ||
			BlinkerSleep.load() > TimeManager->GetCounter())
		{
			TaskManager->Sleep(250);
			continue;
		}

		{
			SmartLock(BlinkerLock);
			PrintBlinker(fx, fy);
			CurBlinking.store(!CurBlinking.load());
			Display->UpdateBuffer();
		}
		TaskManager->Sleep(500);

		{
			SmartLock(BlinkerLock);
			PrintBlinker(fx, fy);
			CurBlinking.store(!CurBlinking.load());
			Display->UpdateBuffer();
		}
		TaskManager->Sleep(500);
	}
}

void StartKernelShell()
{
	if (ShellLock.Locked())
		return;
	SmartLock(ShellLock);

	debug("Starting kernel shell...");
	KPrint("Starting kernel shell...");
	thisThread->SetPriority(Tasking::TaskPriority::High);

	std::string strBuf;
	std::vector<std::string *> history;
	size_t hIdx = 0;
	bool ctrlDown = false;
	bool upperCase = false;
	bool tabDblPress = false;

	int kfd = fopen("/dev/key", "r");
	if (kfd < 0)
	{
		KPrint("Failed to open keyboard device! %s",
			   strerror(kfd));
		return;
	}

	std::thread thd(CursorBlink);

	printf("Using \eCA21F6/dev/key\eCCCCCC for keyboard input.\n");
	while (true)
	{
		size_t bsCount = 0;
		strBuf.clear();

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
			uint32_t cx, cy;
			Display->GetBufferCursor(&cx, &cy);
			CurX.store(cx);
			CurY.store(cy);
			CurHalt.store(false);

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

			UpdateBlinker();
			BlinkerSleep.store(TimeManager->CalculateTarget(250, Time::Units::Milliseconds));
			CurHalt.store(true);

			uint8_t sc = scBuf[0];
			switch (sc & ~KEY_PRESSED)
			{
			case KEY_LEFT_CTRL:
			case KEY_RIGHT_CTRL:
			{
				if (sc & KEY_PRESSED)
					ctrlDown = true;
				else
					ctrlDown = false;
				continue;
			}
			case KEY_LEFT_SHIFT:
			case KEY_RIGHT_SHIFT:
			{
				if (sc & KEY_PRESSED)
					upperCase = true;
				else
					upperCase = false;
				continue;
			}
			case KEY_BACKSPACE:
			{
				if (!(sc & KEY_PRESSED))
					continue;

				if (bsCount == 0)
					continue;

				Display->Print('\b');
				strBuf.pop_back();
				bsCount--;
				Display->UpdateBuffer();
				continue;
			}
			case KEY_UP_ARROW:
			{
				if (!(sc & KEY_PRESSED))
					continue;

				if (history.size() == 0 ||
					hIdx == 0)
					continue;

				hIdx--;

				for (size_t i = 0; i < strBuf.size(); i++)
					Display->Print('\b');
				Display->UpdateBuffer();

				strBuf = history[hIdx]->c_str();

				for (size_t i = 0; i < strlen(strBuf.c_str()); i++)
					Display->Print(strBuf[i]);
				bsCount = strBuf.size();
				Display->UpdateBuffer();
				continue;
			}
			case KEY_DOWN_ARROW:
			{
				if (!(sc & KEY_PRESSED))
					continue;

				if (history.size() == 0 ||
					hIdx == history.size())
					continue;

				if (hIdx == history.size() - 1)
				{
					hIdx++;
					for (size_t i = 0; i < strBuf.size(); i++)
						Display->Print('\b');
					bsCount = strBuf.size();
					Display->UpdateBuffer();
					continue;
				}

				for (size_t i = 0; i < strBuf.size(); i++)
					Display->Print('\b');
				Display->UpdateBuffer();

				hIdx++;
				strBuf = history[hIdx]->c_str();

				for (size_t i = 0; i < strlen(strBuf.c_str()); i++)
					Display->Print(strBuf[i]);

				bsCount = strBuf.size();
				Display->UpdateBuffer();
				continue;
			}
			case KEY_TAB:
			{
				if (!(sc & KEY_PRESSED))
					continue;

				if (!tabDblPress)
				{
					tabDblPress = true;
					continue;
				}
				tabDblPress = false;
				if (strBuf.size() == 0)
				{
					for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
						printf("%s ", commands[i].Name);

					Display->Print('\n');
					Display->UpdateBuffer();
					goto SecLoopEnd;
				}

				for (size_t i = 0; i < strBuf.size(); i++)
					Display->Print('\b');
				Display->UpdateBuffer();

				for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
				{
					if (strncmp(strBuf.c_str(), commands[i].Name, strBuf.size()) == 0)
					{
						strBuf = commands[i].Name;
						for (size_t i = 0; i < strlen(strBuf.c_str()); i++)
							Display->Print(strBuf[i]);
						bsCount = strBuf.size();
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

			char c = Driver::GetScanCode(sc, upperCase);

			if (ctrlDown)
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
				if (strBuf.length() > 0)
				{
					std::string *hBuff = new std::string(strBuf.c_str());
					history.push_back(hBuff);
					hIdx = history.size();
				}
				break;
			}

			strBuf += c;
			bsCount++;
			Display->UpdateBuffer();
		}
	SecLoopEnd:

		if (strBuf.length() == 0)
			continue;

		bool Found = false;
		for (size_t i = 0; i < sizeof(commands) / sizeof(Command); i++)
		{
			std::string cmd_extracted;
			for (size_t i = 0; i < strBuf.length(); i++)
			{
				if (strBuf[i] == ' ')
					break;
				cmd_extracted += strBuf[i];
			}

			// debug("cmd: %s, array[%d]: %s", cmd_extracted.c_str(), i, commands[i].Name);
			if (strncmp(commands[i].Name, cmd_extracted.c_str(), cmd_extracted.size()) == 0)
			{
				if (strlen(commands[i].Name) != cmd_extracted.size())
					continue;

				Found = true;

				std::string arg_only;
				const char *cmd_name = commands[i].Name;
				for (size_t i = strlen(cmd_name) + 1; i < strBuf.length(); i++)
					arg_only += strBuf[i];

				if (commands[i].Function)
					commands[i].Function(arg_only.c_str());
				else
				{
					std::string cmd_only;
					for (size_t i = 0; i < strBuf.length(); i++)
					{
						if (strBuf[i] == ' ')
							break;
						cmd_only += strBuf[i];
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
			for (size_t i = 0; i < strBuf.length(); i++)
			{
				if (strBuf[i] == ' ')
					break;
				cmd_only += strBuf[i];
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
