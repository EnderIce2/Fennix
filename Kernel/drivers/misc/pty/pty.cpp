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

#include <driver.hpp>
#include <kcon.hpp>
#include <task.hpp>
#include <smp.hpp>

extern Driver::Manager *DriverManager;
namespace Driver::TeleTypeDevices
{
	dev_t DriverID;
	TTY::PTMXDevice *ptmx = nullptr;

	struct
	{
		dev_t kcon;
		dev_t tty;
		dev_t ptmx;
	} ids;

	int Open(struct Inode *Node, int Flags, mode_t Mode)
	{
		dev_t min = Node->GetMinor();
		if (min == ids.kcon)
			return KernelConsole::CurrentTerminal.load()->Open(Flags, Mode);
		else if (min == ids.tty)
		{
			TTY::TeletypeDriver *tty = (TTY::TeletypeDriver *)thisProcess->tty;
			if (tty == nullptr)
				return -ENOTTY;
			return tty->Open(Flags, Mode);
		}
		else if (min == ids.ptmx)
			return ptmx->Open();

		return -ENODEV;
	}

	int Close(struct Inode *Node)
	{
		dev_t min = Node->GetMinor();
		if (min == ids.kcon)
			return KernelConsole::CurrentTerminal.load()->Close();
		else if (min == ids.tty)
		{
			TTY::TeletypeDriver *tty = (TTY::TeletypeDriver *)thisProcess->tty;
			if (tty == nullptr)
				return -ENOTTY;
			return tty->Close();
		}
		else if (min == ids.ptmx)
			return ptmx->Close();

		return -ENODEV;
	}

	int Ioctl(struct Inode *Node, unsigned long Request, void *Argp)
	{
		dev_t min = Node->GetMinor();
		if (min == ids.kcon)
			return KernelConsole::CurrentTerminal.load()->Ioctl(Request, Argp);
		else if (min == ids.tty)
		{
			TTY::TeletypeDriver *tty = (TTY::TeletypeDriver *)thisProcess->tty;
			if (tty == nullptr)
				return -ENOTTY;
			return tty->Ioctl(Request, Argp);
		}
		else if (min == ids.ptmx)
			return -ENOSYS;

		return -ENODEV;
	}

	ssize_t Read(struct Inode *Node, void *Buffer, size_t Size, off_t Offset)
	{
		dev_t min = Node->GetMinor();
		if (min == ids.kcon)
			return KernelConsole::CurrentTerminal.load()->Read(Buffer, Size, Offset);
		else if (min == ids.tty)
		{
			TTY::TeletypeDriver *tty = (TTY::TeletypeDriver *)thisProcess->tty;
			if (tty == nullptr)
				return -ENOTTY;
			return tty->Read(Buffer, Size, Offset);
		}
		else if (min == ids.ptmx)
			return -ENOSYS;

		return -ENODEV;
	}

	ssize_t Write(struct Inode *Node, const void *Buffer, size_t Size, off_t Offset)
	{
		dev_t min = Node->GetMinor();
		if (min == ids.kcon)
			return KernelConsole::CurrentTerminal.load()->Write(Buffer, Size, Offset);
		else if (min == ids.tty)
		{
			TTY::TeletypeDriver *tty = (TTY::TeletypeDriver *)thisProcess->tty;
			if (tty == nullptr)
				return -ENOTTY;
			return tty->Write(Buffer, Size, Offset);
		}
		else if (min == ids.ptmx)
			return -ENOSYS;

		return -ENODEV;
	}

	off_t Seek(struct Inode *Node, off_t Offset) { return -ENOSYS; }
	int Stat(struct Inode *Node, struct kstat *Stat) { return -ENOSYS; }

	const struct InodeOperations ops = {
		.Lookup = nullptr,
		.Create = nullptr,
		.Remove = nullptr,
		.Rename = nullptr,
		.Read = Read,
		.Write = Write,
		.Truncate = nullptr,
		.Open = Open,
		.Close = Close,
		.Ioctl = Ioctl,
		.ReadDir = nullptr,
		.MkDir = nullptr,
		.RmDir = nullptr,
		.SymLink = nullptr,
		.ReadLink = nullptr,
		.Seek = Seek,
		.Stat = Stat,
	};

	int Entry()
	{
		ptmx = new TTY::PTMXDevice;
		mode_t mode = 0;

		/* c rw- r-- --- */
		mode = S_IRUSR | S_IWUSR |
			   S_IRGRP |

			   S_IFCHR;
		ids.kcon = DriverManager->CreateDeviceFile(DriverID, "kcon", mode, &ops);

		/* c rw- rw- rw- */
		mode = S_IRUSR | S_IWUSR |
			   S_IRGRP | S_IWGRP |
			   S_IRUSR | S_IWUSR |
			   S_IFCHR;
		ids.tty = DriverManager->CreateDeviceFile(DriverID, "tty", mode, &ops);
		ids.ptmx = DriverManager->CreateDeviceFile(DriverID, "ptmx", mode, &ops);

		return 0;
	}

	int Final()
	{
		DriverManager->UnregisterDevice(DriverID, ids.kcon);
		DriverManager->UnregisterDevice(DriverID, ids.tty);
		DriverManager->UnregisterDevice(DriverID, ids.ptmx);
		delete ptmx;
		return 0;
	}

	int Panic() { return 0; }
	int Probe() { return 0; }

	REGISTER_BUILTIN_DRIVER(pty,
							"Pseudo Terminal Devices Driver",
							"enderice2",
							1, 0, 0,
							Entry,
							Final,
							Panic,
							Probe);
}
