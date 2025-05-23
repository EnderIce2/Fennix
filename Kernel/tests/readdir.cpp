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

#include <fs/vfs.hpp>

#include "../kernel.h"

#ifdef DEBUG

void TestReadDirectory(Node &Target)
{
	const size_t bufLen = 4096;
	std::unique_ptr<uint8_t[]> buf(new uint8_t[bufLen]);
	off_t off = 0;
	off_t max = LONG_MAX;

	KPrint("Listing directory: \x1b[1;36m%s\x1b[0m", Target->Name.c_str());

	while (true)
	{
		ssize_t bytes = fs->ReadDirectory(Target, (kdirent *)buf.get(), bufLen, off, max);
		if (bytes < 0)
		{
			KPrint("ReadDirectory returned error: %d", bytes);
			break;
		}
		if (bytes == 0)
			break;

		ssize_t Pos = 0;
		while (Pos < bytes)
		{
			kdirent *Entry = (kdirent *)(buf.get() + Pos);
			if (Entry->d_reclen == 0)
			{
				KPrint("Entry with zero length detected at offset %ld!", off + Pos);
				break;
			}

			KPrint("name=\"%s\" inode=%lu type=%u reclen=%u", Entry->d_name, Entry->d_ino, Entry->d_type, Entry->d_reclen);

			Pos += Entry->d_reclen;
		}
		off += bytes;
	}
}

void readdir_sanity_tests()
{
	Node root = fs->GetRoot(0);
	Node t0 = fs->Lookup(root, "/");
	Node t1 = fs->Lookup(root, "/dev");
	Node t2 = fs->Lookup(root, "/home");
	Node t3 = fs->Lookup(root, "/var");
	Node t4 = fs->Lookup(root, "/tmp");

	KPrint("TEST /");
	TestReadDirectory(t0);
	TaskManager->Sleep(Time::FromMilliseconds(2000));

	KPrint("TEST /dev");
	TestReadDirectory(t1);
	TaskManager->Sleep(Time::FromMilliseconds(2000));

	KPrint("TEST /home");
	TestReadDirectory(t2);
	TaskManager->Sleep(Time::FromMilliseconds(2000));

	KPrint("TEST /var");
	TestReadDirectory(t3);
	TaskManager->Sleep(Time::FromMilliseconds(2000));

	KPrint("TEST /tmp");
	TestReadDirectory(t4);
	TaskManager->Sleep(Time::FromMilliseconds(2000));

	CPU::Stop();
}

#endif // DEBUG
