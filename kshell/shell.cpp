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
#include <exec.hpp>
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

bool ignoreBuiltin = false;

void __cmd_builtin(const char *)
{
	ignoreBuiltin = !ignoreBuiltin;
	if (ignoreBuiltin)
		printf("Builtin commands are now ignored.\n");
	else
		printf("Builtin commands are now accepted.\n");
}

static Command commands[] = {
	{"lsof", cmd_lsof},
	{"ls", cmd_ls},
	{"tree", cmd_tree},
	{"cd", cmd_cd},
	{"cat", cmd_cat},
	{"echo", cmd_echo},
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
	{"uname", cmd_uname},
	{"whoami", cmd_whoami},
	{"uptime", cmd_uptime},
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
	{"builtin", __cmd_builtin},
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

void UpdateBlinker(bool force = false)
{
	SmartLock(BlinkerLock);
	if (CurBlinking.load() || force)
	{
		uint32_t fx = 0, fy = 0;
		if (unlikely(fx == 0 || fy == 0))
		{
			fx = Display->GetCurrentFont()->GetInfo().Width;
			fy = Display->GetCurrentFont()->GetInfo().Height;
		}

		PrintBlinker(fx, fy);
		CurBlinking.store(force);
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

	vfs::RefNode *kfd = fs->Open("/dev/key");
	if (kfd == nullptr)
	{
		KPrint("Failed to open keyboard device!");
		return;
	}

	/* This makes debugging easier. */
	auto strBufBck = [&]()
	{
		for (size_t i = 0; i < strBuf.size(); i++)
			Display->Print('\b');
	};

	std::thread thd(CursorBlink);

	printf("Using \eCA21F6/dev/key\eCCCCCC for keyboard input.\n");
	while (true)
	{
		size_t bsCount = 0;
		uint32_t homeX = 0, homeY = 0;
		uint32_t unseekX = 0, unseekY = 0;
		size_t seekCount = 0;
		strBuf.clear();

		vfs::Node *cwd = thisProcess->CurrentWorkingDirectory;
		if (!cwd)
			cwd = fs->GetNodeFromPath("/");

		printf("\e34C6EB%s@%s:%s$ \eCCCCCC",
			   "kernel",
			   "fennix",
			   cwd->FullPath);
		Display->UpdateBuffer();

		Display->GetBufferCursor(&homeX, &homeY);

		uint8_t scBuf[2];
		scBuf[1] = 0x00; /* Request scan code */
		ssize_t nBytes;
		while (true)
		{
			uint32_t __cx, __cy;
			Display->GetBufferCursor(&__cx, &__cy);
			CurX.store(__cx);
			CurY.store(__cy);
			CurHalt.store(false);

			nBytes = kfd->read(scBuf, 2);
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

			BlinkerSleep.store(TimeManager->CalculateTarget(250, Time::Units::Milliseconds));
			CurHalt.store(true);
			UpdateBlinker();

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
					if (unseekX != 0 || unseekY != 0)
					{
						Display->SetBufferCursor(unseekX, unseekY);
						unseekX = unseekY = 0;
					}

					for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
						printf("%s ", commands[i].Name);

					Display->Print('\n');
					Display->UpdateBuffer();
					goto SecLoopEnd;
				}

				if (unseekX != 0 || unseekY != 0)
				{
					Display->SetBufferCursor(unseekX, unseekY);
					unseekX = unseekY = 0;
				}

				strBufBck();
				Display->UpdateBuffer();

				for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
				{
					if (strncmp(strBuf.c_str(), commands[i].Name, strBuf.size()) == 0)
					{
						strBuf = commands[i].Name;
						for (size_t i = 0; i < strlen(strBuf.c_str()); i++)
							Display->Print(strBuf[i]);
						seekCount = bsCount = strBuf.size();
						Display->UpdateBuffer();
						break;
					}
				}
				continue;
			}
			case KEY_BACKSPACE:
			{
				if (!(sc & KEY_PRESSED))
					continue;

				if (bsCount == 0)
					continue;

				if (seekCount == bsCount)
				{
					debug("seekCount == bsCount (%d == %d)",
						  seekCount, bsCount);
					Display->Print('\b');
					strBuf.pop_back();
					seekCount = --bsCount;
					Display->UpdateBuffer();
					continue;
				}

				uint32_t tmpX, tmpY;
				Display->GetBufferCursor(&tmpX, &tmpY);

				Display->SetBufferCursor(unseekX, unseekY);
				strBufBck();
				size_t strSeek = seekCount ? seekCount - 1 : 0;
				seekCount = strSeek;
				debug("strSeek: %d: %s", strSeek, strBuf.c_str());
				strBuf.erase((int)strSeek);
				Display->PrintString(strBuf.c_str());
				debug("after strBuf: %s", strBuf.c_str());

				uint32_t fx = Display->GetCurrentFont()->GetInfo().Width;
				Display->SetBufferCursor(tmpX - fx, tmpY);
				unseekX -= fx;

				bsCount--;
				Display->UpdateBuffer();
				continue;
			}
			case KEY_DELETE:
			{
				if (!(sc & KEY_PRESSED))
					continue;

				if (bsCount == 0)
					continue;

				if (seekCount == bsCount)
				{
					debug("seekCount == bsCount (%d == %d)",
						  seekCount, bsCount);
					continue;
				}

				uint32_t tmpX, tmpY;
				Display->GetBufferCursor(&tmpX, &tmpY);

				Display->SetBufferCursor(unseekX, unseekY);
				strBufBck();
				debug("seekCount: %d: %s", seekCount, strBuf.c_str());
				strBuf.erase((int)seekCount);
				Display->PrintString(strBuf.c_str());
				debug("after strBuf: %s", strBuf.c_str());

				Display->SetBufferCursor(tmpX, tmpY);
				unseekX -= Display->GetCurrentFont()->GetInfo().Width;
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

				if (unseekX != 0 || unseekY != 0)
				{
					Display->SetBufferCursor(unseekX, unseekY);
					unseekX = unseekY = 0;
				}

				strBufBck();
				Display->UpdateBuffer();

				strBuf = history[hIdx]->c_str();

				for (size_t i = 0; i < strlen(strBuf.c_str()); i++)
					Display->Print(strBuf[i]);
				seekCount = bsCount = strBuf.size();
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
					if (unseekX != 0 || unseekY != 0)
					{
						Display->SetBufferCursor(unseekX, unseekY);
						unseekX = unseekY = 0;
					}

					hIdx++;
					strBufBck();
					seekCount = bsCount = strBuf.size();
					Display->UpdateBuffer();
					continue;
				}

				if (unseekX != 0 || unseekY != 0)
				{
					Display->SetBufferCursor(unseekX, unseekY);
					unseekX = unseekY = 0;
				}

				strBufBck();
				Display->UpdateBuffer();

				hIdx++;
				strBuf = history[hIdx]->c_str();

				for (size_t i = 0; i < strlen(strBuf.c_str()); i++)
					Display->Print(strBuf[i]);

				seekCount = bsCount = strBuf.size();
				Display->UpdateBuffer();
				continue;
			}
			case KEY_LEFT_ARROW:
			{
				if (!(sc & KEY_PRESSED))
					continue;

				UpdateBlinker();
				if (seekCount == 0)
					continue;

				debug("orig seekCount: %d", seekCount);

				seekCount--;

				if (unseekX == 0 && unseekY == 0)
					Display->GetBufferCursor(&unseekX, &unseekY);

				if (ctrlDown)
				{
					uint32_t offset = 0;
					/* We use unsigned so this will underflow to SIZE_MAX
						and it is safe because we add 1 to it. */
					while (seekCount != SIZE_MAX && strBuf[seekCount] == ' ')
					{
						seekCount--;
						offset++;
					}
					while (seekCount != SIZE_MAX && strBuf[seekCount] != ' ')
					{
						seekCount--;
						offset++;
					}
					seekCount++;
					debug("offset: %d; seekCount: %d", offset, seekCount);

					uint32_t fx = Display->GetCurrentFont()->GetInfo().Width;
					uint32_t cx, cy;
					Display->GetBufferCursor(&cx, &cy);
					Display->SetBufferCursor(cx - (fx * offset), cy);
					CurX.store(cx - (fx * offset));
					CurY.store(cy);
					UpdateBlinker(true);
					Display->UpdateBuffer();
					continue;
				}

				uint32_t cx, cy;
				Display->GetBufferCursor(&cx, &cy);
				uint32_t fx = Display->GetCurrentFont()->GetInfo().Width;
				Display->SetBufferCursor(cx - fx, cy);

				CurX.store(cx - fx);
				CurY.store(cy);
				UpdateBlinker(true);
				Display->UpdateBuffer();
				continue;
			}
			case KEY_RIGHT_ARROW:
			{
				if (!(sc & KEY_PRESSED))
					continue;

				UpdateBlinker();
				if (seekCount == bsCount)
					continue;
				seekCount++;

				debug("orig seekCount: %d", seekCount);

				if (unseekX == 0 && unseekY == 0)
					Display->GetBufferCursor(&unseekX, &unseekY);

				if (ctrlDown)
				{
					uint32_t offset = 0;
					while (seekCount <= bsCount && strBuf[seekCount] != ' ')
					{
						seekCount++;
						offset++;
					}
					while (seekCount <= bsCount && strBuf[seekCount] == ' ')
					{
						seekCount++;
						offset++;
					}
					seekCount--;

					debug("offset: %d; seekCount: %d", offset, seekCount);

					uint32_t fx = Display->GetCurrentFont()->GetInfo().Width;
					uint32_t cx, cy;
					Display->GetBufferCursor(&cx, &cy);
					Display->SetBufferCursor(cx + (fx * offset), cy);

					CurX.store(cx + (fx * offset));
					CurY.store(cy);
					UpdateBlinker(true);
					Display->UpdateBuffer();
					continue;
				}

				uint32_t cx, cy;
				Display->GetBufferCursor(&cx, &cy);
				uint32_t fx = Display->GetCurrentFont()->GetInfo().Width;
				Display->SetBufferCursor(cx + fx, cy);

				CurX.store(cx + fx);
				CurY.store(cy);
				Display->UpdateBuffer();
				UpdateBlinker(true);
				continue;
			}
			case KEY_HOME:
			{
				if (!(sc & KEY_PRESSED))
					continue;

				if (homeX == 0 || homeY == 0)
					continue;

				UpdateBlinker();
				if (unseekX == 0 || unseekY == 0)
					Display->GetBufferCursor(&unseekX, &unseekY);

				Display->SetBufferCursor(homeX, homeY);

				seekCount = 0;

				debug("seekCount set to 0");

				CurX.store(homeX);
				CurY.store(homeY);
				Display->UpdateBuffer();
				UpdateBlinker(true);
				continue;
			}
			case KEY_END:
			{
				if (!(sc & KEY_PRESSED))
					continue;

				if (unseekX == 0 || unseekY == 0)
					continue;

				UpdateBlinker();
				Display->SetBufferCursor(unseekX, unseekY);
				seekCount = bsCount;

				debug("seekCount set to bsCount (%d)", bsCount);

				CurX.store(unseekX);
				CurY.store(unseekY);
				Display->UpdateBuffer();
				UpdateBlinker(true);
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

			if (c == '\n')
			{
				Display->Print(c);
				if (strBuf.length() > 0)
				{
					std::string *hBuff = new std::string(strBuf.c_str());
					history.push_back(hBuff);
					hIdx = history.size();
				}
				break;
			}
			else if (seekCount >= bsCount)
			{
				Display->Print(c);
				strBuf += c;
				seekCount = ++bsCount;
			}
			else
			{
				uint32_t tmpX, tmpY;
				Display->GetBufferCursor(&tmpX, &tmpY);

				if (unseekX != 0 && unseekY != 0)
					Display->SetBufferCursor(unseekX, unseekY);
				strBufBck();

				// size_t strSeek = seekCount ? seekCount - 1 : 0;
				debug("seekCount: %d; \"%s\"", seekCount, strBuf.c_str());
				strBuf.insert(seekCount, 1, c);
				Display->PrintString(strBuf.c_str());
				debug("after strBuf: %s (seek and bs is +1 [seek: %d; bs: %d])",
					  strBuf.c_str(), seekCount + 1, bsCount + 1);

				uint32_t fx = Display->GetCurrentFont()->GetInfo().Width;
				Display->SetBufferCursor(tmpX + fx, tmpY);
				unseekX += fx;
				seekCount++;
				bsCount++;
			}

			Display->UpdateBuffer();
		}
	SecLoopEnd:

		if (strBuf.length() == 0)
			continue;

		bool Found = false;
		for (size_t i = 0; i < sizeof(commands) / sizeof(Command); i++)
		{
			if (unlikely(strncmp(strBuf.c_str(), "builtin", strBuf.length()) == 0))
			{
				__cmd_builtin(nullptr);
				Found = true;
				break;
			}

			if (ignoreBuiltin)
				break;

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

		if (Found)
			continue;

		std::string cmd_only;
		for (size_t i = 0; i < strBuf.length(); i++)
		{
			if (strBuf[i] == ' ')
				break;
			cmd_only += strBuf[i];
		}

		std::string path = "/bin/";

		if (!fs->PathExists("/bin"))
			path = "/usr/bin/";

		path += cmd_only;
		debug("path: %s", path.c_str());
		if (fs->PathExists(path.c_str()))
		{
			const char *envp[5] = {
				"PATH=/bin:/usr/bin",
				"TERM=tty",
				"HOME=/root",
				"USER=root",
				nullptr};

			const char **argv;
			if (strBuf.length() > cmd_only.length())
			{
				std::string arg_only;
				for (size_t i = cmd_only.length() + 1; i < strBuf.length(); i++)
					arg_only += strBuf[i];

				argv = new const char *[3];
				argv[0] = path.c_str();
				argv[1] = new char[arg_only.length() + 1];
				strcpy((char *)argv[1], arg_only.c_str());
				argv[2] = nullptr;

				debug("argv[0]: %s; argv[1]: %s", argv[0], argv[1]);
			}
			else
			{
				argv = new const char *[2];
				argv[0] = path.c_str();
				argv[1] = nullptr;
			}

			Tasking::TaskCompatibility compat = Tasking::Native;
			if (Config.UseLinuxSyscalls)
				compat = Tasking::Linux;

			int ret = Execute::Spawn((char *)path.c_str(), argv, envp,
									 nullptr, false, compat, false);
			if (argv[1])
				delete argv[1];
			delete argv;
			if (ret >= 0)
			{
				Tasking::TCB *tcb;
				Tasking::PCB *pcb;
				pcb = TaskManager->GetProcessByID(ret);
				if (pcb == nullptr)
				{
					printf("Failed to get process by ID\n");
					continue;
				}
				pcb->SetWorkingDirectory(cwd);
				tcb = TaskManager->GetThreadByID(ret, pcb);
				if (tcb == nullptr)
				{
					printf("Failed to get thread by ID\n");
					continue;
				}
				TaskManager->WaitForThread(tcb);
				continue;
			}
		}

		printf("%s: command not found\n",
			   cmd_only.c_str());
	}
}

void KShellThread()
{
	StartKernelShell();
	inf_loop;
}
