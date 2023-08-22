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

namespace VirtualFileSystem
{
	ReferenceNode *Node::CreateReference()
	{
		SmartLock(NodeLock);
		ReferenceNode *rn = new ReferenceNode(this);
		References.push_back(rn);
		debug("Created reference %p for node %p", rn, this);
		return rn;
	}

	void Node::RemoveReference(ReferenceNode *Reference)
	{
		SmartLock(NodeLock);
		debug("Removing reference %p for node %p", Reference, this);
		References.erase(std::find(References.begin(), References.end(), Reference));
	}

	/**************************************************************/

	size_t ReferenceNode::Read(uint8_t *Buffer, size_t Size)
	{
		if (this->SymlinkTo)
			return this->SymlinkTo->Read(Buffer, Size);

		if (!this->node->Operator)
		{
			errno = EFAULT;
			return -1;
		}

		SmartLock(RefNodeLock);

		if (this->node->Operator->Read)
		{
			off_t RefOffset = off_t(this->Offset.load());
			return this->node->Operator->Read(this->node, Size, Buffer, RefOffset);
		}

		errno = ENOSYS;
		return -1;
	}

	size_t ReferenceNode::Write(uint8_t *Buffer, size_t Size)
	{
		if (this->SymlinkTo)
			return this->SymlinkTo->Write(Buffer, Size);

		if (!this->node->Operator)
		{
			errno = EFAULT;
			return -1;
		}

		SmartLock(RefNodeLock);

		if (this->node->Operator->Write)
		{
			off_t RefOffset = off_t(this->Offset.load());
			return this->node->Operator->Write(this->node, Size, Buffer, RefOffset);
		}

		errno = ENOSYS;
		return -1;
	}

	off_t ReferenceNode::Seek(off_t _Offset, int Whence)
	{
		if (this->SymlinkTo)
			return this->SymlinkTo->Seek(_Offset, Whence);

		if (!this->node->Operator)
		{
			errno = EFAULT;
			return -1;
		}

		SmartLock(RefNodeLock);

		if (this->node->Operator->Seek)
		{
			off_t RefOffset = off_t(this->Offset.load());
			debug("The node has a seek function");
			return this->node->Operator->Seek(this->node, _Offset, Whence, RefOffset);
		}

		debug("Current offset is %d", this->Offset.load());
		switch (Whence)
		{
		case SEEK_SET:
		{
			if (_Offset > this->node->Length)
			{
				errno = EINVAL;
				return -1;
			}

			if (_Offset < 0)
			{
				fixme("Negative offset %d is not implemented", _Offset);
				_Offset = 0;
			}

			if (_Offset > this->node->Length)
			{
				fixme("Offset %d is bigger than file size %d",
					  _Offset, this->node->Length);
				_Offset = this->node->Length;
			}

			this->Offset.store(_Offset);
			break;
		}
		case SEEK_CUR:
		{
			off_t NewOffset = off_t(this->Offset.load()) + _Offset;
			if (NewOffset > this->node->Length ||
				NewOffset < 0)
			{
				errno = EINVAL;
				return -1;
			}

			this->Offset.store(NewOffset);
			break;
		}
		case SEEK_END:
		{
			off_t NewOffset = this->node->Length + _Offset;
			if (NewOffset > this->node->Length ||
				NewOffset < 0)
			{
				errno = EINVAL;
				return -1;
			}

			this->Offset.store(NewOffset);
			break;
		}
		default:
		{
			error("Invalid whence!");
			errno = EINVAL;
			return -1;
		}
		}

		off_t RetOffset = off_t(this->Offset.load());
		debug("( %d %ld %s[%d] ) -> %d",
			  _Offset, this->Offset.load(),
			  SeekStrings[Whence], Whence,
			  RetOffset);
		return RetOffset;
	}

	ReferenceNode::ReferenceNode(Node *node)
	{
		SmartLock(RefNodeLock);
		this->node = node;
		this->FileSize = node->Length;
		this->AbsolutePath += node->FileSystem->GetPathFromNode(node);
		if (this->node->Flags == SYMLINK)
		{
			if (!this->node->SymlinkTarget)
			{
				this->node->SymlinkTarget =
					node->FileSystem->GetNodeFromPath(this->node->Symlink);

				/* not standard but useful in kernel-space */
				this->node->Length = this->node->SymlinkTarget->Length;
			}

			if (!this->node->SymlinkTarget)
			{
				error("Symlink target %s not found!",
					  this->node->Symlink);
				errno = ENOENT;
				return;
			}

			this->SymlinkTo = this->node->SymlinkTarget->CreateReference();
		}

		debug("Created reference node for %s [%#lx]",
			  this->AbsolutePath.c_str(), (uintptr_t)this);
	}

	ReferenceNode::~ReferenceNode()
	{
		SmartLock(RefNodeLock);
		if (this->SymlinkTo)
			this->node->SymlinkTarget->RemoveReference(this);
		this->node->RemoveReference(this);

		debug("Destroyed reference node for %s [%#lx]",
			  this->AbsolutePath.c_str(), (uintptr_t)this);
	}
}
