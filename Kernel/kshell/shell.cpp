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

#include <interface/driver.h>
#include <interface/input.h>
#include <fs/vfs.hpp>
#include <driver.hpp>
#include <lock.hpp>
#include <exec.hpp>
#include <debug.h>
#include <thread>
#include <cctype>

#include "../kernel.h"
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
	{"theme", cmd_theme},
	{"builtin", __cmd_builtin},
};

void KShellThread()
{
	assert(!ShellLock.Locked());
	ShellLock.Lock(__FUNCTION__);

	KPrint("Starting kernel shell...");
	thisThread->SetPriority(Tasking::TaskPriority::Normal);
	thisProcess->CWD = fs->GetRoot(0);

	std::string strBuf = "";
	std::vector<std::string *> history;
	size_t hIdx = 0;
	bool ctrlDown = false;
	bool upperCase = false;
	bool tabDblPress = false;

	const char *keyDevPath = "/dev/input/keyboard";
	Node root = fs->GetRoot(0);
	Node kfd = fs->Lookup(root, keyDevPath);
	if (kfd == nullptr)
	{
		KPrint("Failed to open keyboard device!");
		return;
	}

	/* This makes debugging easier. */
	auto strBufBck = [&]()
	{
		for (size_t i = 0; i < strBuf.size(); i++)
		{
			putchar('\b');
			putchar(' ');
			putchar('\b');
		}
	};

	printf("Using \x1b[1;34m%s\x1b[0m for keyboard input.\n", keyDevPath);
	while (true)
	{
		size_t bsCount = 0;
		uint32_t homeX = 0, homeY = 0;
		uint32_t unseekX = 0, unseekY = 0;
		size_t seekCount = 0;
		debug("clearing strBuf(\"%s\")", strBuf.c_str());
		strBuf.clear();

		Node cwd = thisProcess->CWD;
		if (!cwd)
			cwd = fs->GetRoot(0);
		debug("cwd: %s", cwd->Path.c_str());

		printf("\x1b[1;34m%s@%s:%s$ \x1b[0m",
			   "kernel", "fennix",
			   cwd->Path.c_str());

		KeyboardReport scBuf{};
		ssize_t nBytes;
		while (true)
		{
			nBytes = fs->Read(kfd, &scBuf, sizeof(KeyboardReport), 0);
			if (nBytes == 0)
			{
				debug("Empty read from keyboard device!");
				continue;
			}
			if (nBytes < (ssize_t)sizeof(KeyboardReport))
			{
				KPrint("Failed to read from keyboard device: %s",
					   strerror((int)nBytes));
				return;
			}

			const KeyScanCodes &sc = scBuf.Key;
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
					for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
						printf("%s ", commands[i].Name);

					putchar('\n');
					goto SecLoopEnd;
				}

				strBufBck();

				for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
				{
					if (strncmp(strBuf.c_str(), commands[i].Name, strBuf.size()) != 0)
						continue;

					strBuf = commands[i].Name;
					for (size_t i = 0; i < strlen(strBuf.c_str()); i++)
						putchar(strBuf[i]);
					seekCount = bsCount = strBuf.size();
					break;
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
					putchar('\b');
					putchar(' ');
					putchar('\b');
					strBuf.pop_back();
					seekCount = --bsCount;
					continue;
				}

				strBufBck();
				size_t strSeek = seekCount ? seekCount - 1 : 0;
				seekCount = strSeek;
				debug("strSeek: %d: %s", strSeek, strBuf.c_str());
				strBuf.erase(strSeek);
				printf("%s", strBuf.c_str());
				debug("after strBuf: %s", strBuf.c_str());

				bsCount--;
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

				strBufBck();
				debug("seekCount: %d: %s", seekCount, strBuf.c_str());
				strBuf.erase(seekCount);
				printf("%s", strBuf.c_str());
				debug("after strBuf: %s", strBuf.c_str());

				bsCount--;
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
					unseekX = unseekY = 0;
				}

				strBufBck();

				strBuf = history[hIdx]->c_str();

				for (size_t i = 0; i < strlen(strBuf.c_str()); i++)
					putchar(strBuf[i]);
				seekCount = bsCount = strBuf.size();
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
						unseekX = unseekY = 0;

					hIdx++;
					strBufBck();
					seekCount = bsCount = strBuf.size();
					continue;
				}

				if (unseekX != 0 || unseekY != 0)
					unseekX = unseekY = 0;

				strBufBck();

				hIdx++;
				strBuf = history[hIdx]->c_str();

				for (size_t i = 0; i < strlen(strBuf.c_str()); i++)
					putchar(strBuf[i]);

				seekCount = bsCount = strBuf.size();
				continue;
			}
			case KEY_LEFT_ARROW:
			{
				if (!(sc & KEY_PRESSED))
					continue;

				if (seekCount == 0)
					continue;

				debug("orig seekCount: %d", seekCount);

				seekCount--;

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
					continue;
				}

				continue;
			}
			case KEY_RIGHT_ARROW:
			{
				if (!(sc & KEY_PRESSED))
					continue;

				if (seekCount == bsCount)
					continue;
				seekCount++;

				debug("orig seekCount: %d", seekCount);

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
					continue;
				}

				continue;
			}
			case KEY_HOME:
			{
				if (!(sc & KEY_PRESSED))
					continue;

				if (homeX == 0 || homeY == 0)
					continue;

				seekCount = 0;

				debug("seekCount set to 0");
				continue;
			}
			case KEY_END:
			{
				if (!(sc & KEY_PRESSED))
					continue;

				if (unseekX == 0 || unseekY == 0)
					continue;

				seekCount = bsCount;
				debug("seekCount set to bsCount (%d)", bsCount);
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
			debug("sc: %#lx, uc: %d -> %c", sc, upperCase, c);

			if (ctrlDown)
			{
				switch (std::toupper((char)c))
				{
				case 'C':
				{
					putchar('^');
					putchar('C');
					putchar('\n');
					fixme("No SIGINT handler yet.");
					goto SecLoopEnd;
				}
				case 'D':
				{
					putchar('^');
					putchar('D');
					putchar('\n');
					fixme("No SIGKILL handler yet.");
					goto SecLoopEnd;
				}
				default:
					continue;
				}
			}

			if (c == '\n')
			{
				putchar(c);
				if (strBuf.length() > 0)
				{
					std::string *hBuff = new std::string(strBuf.c_str());
					debug("cloned strBuf(\"%s\") to hBuff(\"%s\")", strBuf.c_str(), hBuff->c_str());
					history.push_back(hBuff);
					hIdx = history.size();
					debug("pushed \"%s\" to history; index: %d", hBuff->c_str(), hIdx);
				}
				break;
			}
			else if (seekCount >= bsCount)
			{
				putchar(c);
				debug("BEFORE strBuf(\"%s\") %ld %ld", strBuf.c_str(), strBuf.size(), strBuf.capacity());
				strBuf += c;
				debug("AFTER strBuf(\"%s\") %ld %ld", strBuf.c_str(), strBuf.size(), strBuf.capacity());
				seekCount = ++bsCount;
			}
			else
			{
				strBufBck();

				debug("seekCount: %d; \"%s\"", seekCount, strBuf.c_str());
				strBuf.insert(seekCount, (size_t)1, c);
				printf("%s", strBuf.c_str());
				debug("after strBuf: %s (seek and bs is +1 [seek: %d; bs: %d])",
					  strBuf.c_str(), seekCount + 1, bsCount + 1);

				seekCount++;
				bsCount++;
			}
		}
	SecLoopEnd:

		debug("strBuf.length(): %d", strBuf.length());
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

				std::string arg_only = "";
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
		debug("cmd_only: %s", cmd_only.c_str());

		std::string path = "/bin/";

		if (fs->PathIsRelative(cmd_only.c_str()))
		{
			path += cmd_only;
			if (!fs->Lookup(root, path.c_str()))
				path = "/usr/bin/" + cmd_only;
		}
		else
			path = cmd_only;

		debug("path: %s", path.c_str());
		if (fs->Lookup(root, path.c_str()))
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
			if (Config.LinuxSubsystem)
				compat = Tasking::Linux;

			int ret = Execute::Spawn(path.c_str(), argv, envp, nullptr, false, compat, false);
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
					printf("KShell: Failed to get process by ID\n");
					continue;
				}
				pcb->SetWorkingDirectory(cwd);
				tcb = TaskManager->GetThreadByID(ret, pcb);
				if (tcb == nullptr)
				{
					printf("KShell: Failed to get thread by ID\n");
					continue;
				}
				TaskManager->WaitForThread(tcb);
				continue;
			}
		}

		printf("%s: command not found\n",
			   cmd_only.c_str());
	}
	inf_loop;
}
