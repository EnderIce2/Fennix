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

#include "kernel.h"

#include <filesystem/mounts.hpp>
#include <filesystem/ustar.hpp>
#include <memory.hpp>

vfs::Virtual *fs = nullptr;
vfs::Node *DevFS = nullptr;
vfs::Node *MntFS = nullptr;
vfs::Node *ProcFS = nullptr;
vfs::Node *VarLogFS = nullptr;
vfs::PTMXDevice *ptmx = nullptr;

EXTERNC NIF void KernelVFS()
{
	fs = new vfs::Virtual;
	vfs::Node *root = fs->GetRootNode();
	if (root->Children.size() == 0)
		fs->nRoot = new vfs::vfsRoot("/", fs);

	for (size_t i = 0; i < MAX_MODULES; i++)
	{
		if (!bInfo.Modules[i].Address)
			continue;

		if (strcmp(bInfo.Modules[i].CommandLine, "initrd") == 0)
		{
			KPrint("initrd found at %p", bInfo.Modules[i].Address);
			static char initrd = 0;
			if (!initrd++)
			{
				uintptr_t initrdAddress = (uintptr_t)bInfo.Modules[i].Address;

				Memory::Virtual vmm;
				if (!vmm.Check((void *)initrdAddress))
				{
					warn("Initrd is not mapped!");
					vmm.Map((void *)initrdAddress, (void *)initrdAddress,
							bInfo.Modules[i].Size, Memory::RW);
				}

				vfs::USTAR *ustar = new vfs::USTAR;
				ustar->ReadArchive(initrdAddress, fs);
			}
		}
	}

	KPrint("Mounting filesystems");

	if (!fs->PathExists("/dev"))
		DevFS = new vfs::Node(fs->nRoot, "dev", vfs::DIRECTORY);
	else
	{
		vfs::RefNode *dev = fs->Open("/dev");
		if (dev->node->Type != vfs::NodeType::DIRECTORY)
		{
			KPrint("\eE85230/dev is not a directory!");
			CPU::Stop();
		}
		DevFS = dev->node;
		delete dev;
	}

	if (!fs->PathExists("/mnt"))
		MntFS = new vfs::Node(fs->nRoot, "mnt", vfs::DIRECTORY);
	else
	{
		vfs::RefNode *mnt = fs->Open("/mnt");
		if (mnt->node->Type != vfs::NodeType::DIRECTORY)
		{
			KPrint("\eE85230/mnt is not a directory!");
			CPU::Stop();
		}
		MntFS = mnt->node;
		delete mnt;
	}

	if (!fs->PathExists("/proc"))
		ProcFS = new vfs::Node(fs->nRoot, "proc", vfs::DIRECTORY);
	else
	{
		vfs::RefNode *proc = fs->Open("/proc", nullptr);
		if (proc->node->Type != vfs::NodeType::DIRECTORY)
		{
			KPrint("\eE85230/proc is not a directory!");
			CPU::Stop();
		}
		ProcFS = proc->node;
		delete proc;
	}

	if (!fs->PathExists("/var"))
	{
		vfs::Node *var = new vfs::Node(fs->nRoot, "var", vfs::DIRECTORY);
		VarLogFS = new vfs::Node(var, "log", vfs::DIRECTORY);
	}
	else
	{
		vfs::RefNode *var = fs->Open("/var", nullptr);
		if (var->node->Type != vfs::NodeType::DIRECTORY)
		{
			KPrint("\eE85230/var is not a directory!");
			CPU::Stop();
		}
		VarLogFS = var->node;
		delete var;

		if (!fs->PathExists("/var/log"))
			VarLogFS = new vfs::Node(VarLogFS, "log", vfs::DIRECTORY);
		else
		{
			vfs::RefNode *var_log = fs->Open("/var/log", nullptr);
			if (var_log->node->Type != vfs::NodeType::DIRECTORY)
			{
				KPrint("\eE85230/var/log is not a directory!");
				CPU::Stop();
			}
			VarLogFS = var_log->node;
			delete var_log;
		}
	}

	new vfs::NullDevice();
	new vfs::RandomDevice();
	new vfs::ZeroDevice();
	new vfs::KConDevice();
	ptmx = new vfs::PTMXDevice();
}
