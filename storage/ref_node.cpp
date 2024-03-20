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

#include <filesystem.hpp>

#ifdef DEBUG
const char *SeekStrings[] =
	{"SEEK_SET",
	 "SEEK_CUR",
	 "SEEK_END"};
#endif

namespace vfs
{
	size_t RefNode::read(uint8_t *Buffer, size_t Size)
	{
		if (this->SymlinkTo)
			return this->SymlinkTo->read(Buffer, Size);

		debug("Reading %d bytes from %s[%d]",
			  Size, this->node->FullPath, this->FileOffset.load());
		return this->node->read(Buffer, Size, this->FileOffset.load());
	}

	size_t RefNode::write(uint8_t *Buffer, size_t Size)
	{
		if (this->SymlinkTo)
			return this->SymlinkTo->write(Buffer, Size);

		debug("Writing %d bytes to %s[%d]",
			  Size, this->node->FullPath, this->FileOffset.load());
		return this->node->write(Buffer, Size, this->FileOffset.load());
	}

	off_t RefNode::seek(off_t Offset, int Whence)
	{
		if (this->SymlinkTo)
			return this->SymlinkTo->seek(Offset, Whence);

		// debug("Current offset is %d", this->Offset.load());
		switch (Whence)
		{
		case SEEK_SET:
		{
			if (Offset > this->FileSize)
				return -EINVAL;

			if (Offset < 0)
			{
				fixme("Negative offset %d is not implemented", Offset);
				Offset = 0;
			}

			if (Offset > this->FileSize)
			{
				fixme("Offset %d is bigger than file size %d",
					  Offset, this->FileSize);
				Offset = this->FileSize;
			}

			this->FileOffset.store(Offset);
			break;
		}
		case SEEK_CUR:
		{
			off_t NewOffset = off_t(this->FileOffset.load()) + Offset;
			if (NewOffset > this->FileSize ||
				NewOffset < 0)
			{
				return -EINVAL;
			}

			this->FileOffset.store(NewOffset);
			break;
		}
		case SEEK_END:
		{
			off_t NewOffset = this->FileSize + Offset;
			if (NewOffset > this->FileSize ||
				NewOffset < 0)
			{
				return -EINVAL;
			}

			this->FileOffset.store(NewOffset);
			break;
		}
		default:
		{
			error("Invalid whence!");
			return -EINVAL;
		}
		}

		off_t RetOffset = off_t(this->FileOffset.load());
		// debug("( %d %ld %s[%d] ) -> %d",
		// 	  Offset, this->Offset.load(),
		// 	  SeekStrings[Whence], Whence,
		// 	  RetOffset);
		return RetOffset;
	}

	int RefNode::ioctl(unsigned long Request, void *Argp)
	{
		if (this->SymlinkTo)
			return this->SymlinkTo->ioctl(Request, Argp);

		return this->node->ioctl(Request, Argp);
	}

	RefNode::RefNode(Node *node)
	{
		this->node = node;
		this->FileSize = node->Size;
		if (this->node->Type == SYMLINK)
		{
			if (!this->node->SymlinkTarget)
			{
				this->node->SymlinkTarget =
					node->vFS->GetNodeFromPath(this->node->Symlink);
			}

			if (!this->node->SymlinkTarget)
			{
				error("Symlink target %s not found!",
					  this->node->Symlink);
				return;
			}

			/* not standard but useful in kernel-space */
			this->node->Size = this->node->SymlinkTarget->Size;
			this->SymlinkTo = this->node->SymlinkTarget->CreateReference();
		}

		debug("Created reference node for %s [%#lx]",
			  this->node->FullPath, (uintptr_t)this);
	}

	RefNode::~RefNode()
	{
		if (this->SymlinkTo)
			this->node->SymlinkTarget->RemoveReference(this);

		this->node->RemoveReference(this);

		debug("Destroyed reference node for %s [%#lx]",
			  this->node->FullPath, (uintptr_t)this);
	}
}
