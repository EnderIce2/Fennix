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
#include <cwalk.h>

namespace vfs
{
	int Node::open(int Flags, mode_t Mode)
	{
		if (likely(open_ptr))
			return open_ptr(Flags, Mode);

		debug("Operation not handled for %s(%#lx)",
			  this->FullPath, this);
		return -ENODEV;
	}

	int Node::close()
	{
		if (likely(close_ptr))
			return close_ptr();

		debug("Operation not handled for %s(%#lx)",
			  this->FullPath, this);
		return -ENODEV;
	}

	size_t Node::read(uint8_t *Buffer, size_t Size, off_t Offset)
	{
		if (likely(read_ptr))
			return read_ptr(Buffer, Size, Offset);

		debug("Operation not handled for %s(%#lx)",
			  this->FullPath, this);
		return -ENODEV;
	}

	size_t Node::write(uint8_t *Buffer, size_t Size, off_t Offset)
	{
		if (likely(write_ptr))
			return write_ptr(Buffer, Size, Offset);

		debug("Operation not handled for %s(%#lx)",
			  this->FullPath, this);
		return -ENODEV;
	}

	int Node::ioctl(unsigned long Request, void *Argp)
	{
		if (likely(ioctl_ptr))
			return ioctl_ptr(Request, Argp);

		debug("Operation not handled for %s(%#lx)",
			  this->FullPath, this);
		return -ENODEV;
	}

	RefNode *Node::CreateReference()
	{
		SmartLock(NodeLock);
		RefNode *ref = new RefNode(this);
		References.push_back(ref);
		debug("Created reference %#lx for node %#lx(%s)",
			  ref, this, this->Name);
		return ref;
	}

	void Node::RemoveReference(RefNode *Reference)
	{
		SmartLock(NodeLock);
		debug("Removing reference %#lx for node %#lx", Reference, this);
		References.erase(std::find(References.begin(),
								   References.end(),
								   Reference));
	}

	Node::Node(Node *Parent, const char *Name, NodeType Type,
			   bool NoParent, Virtual *_fs, int *Err)
	{
		assert(Name != nullptr);
		assert(strlen(Name) != 0);

		if (Parent && _fs == nullptr)
			_fs = Parent->vFS;

		if (Err != nullptr)
			*Err = 0;

		this->Type = Type;

		auto GetChild = [](const char *Name, Node *Parent)
		{
			if (!Parent)
				return (Node *)nullptr;

			foreach (auto Child in Parent->Children)
			{
				if (strcmp(Child->Name, Name) == 0)
					return Child;
			}

			return (Node *)nullptr;
		};

		auto CreateWithParent = [this](const char *Name, Node *Parent)
		{
			assert(Parent->vFS != nullptr);
			assert(Parent->Type == DIRECTORY ||
				   Parent->Type == MOUNTPOINT);

			this->vFS = Parent->vFS;
			this->Parent = Parent;

			this->Name = new char[strlen(Name) + 1];
			strcpy((char *)this->Name, Name);

			this->FullPath = new char[strlen(Parent->FullPath) +
									  strlen(this->Name) + 2];
			strcpy((char *)this->FullPath, Parent->FullPath);
			if (strcmp(this->FullPath, "/") != 0)
				strcat((char *)this->FullPath, "/");
			strcat((char *)this->FullPath, this->Name);

			this->Parent->Children.push_back(this);
		};

		if (NoParent)
		{
			Parent = nullptr;
			const char *Path = Name;

			Node *RootNode = _fs->FileSystemRoot->Children[0];
			Node *CurrentParent = _fs->GetParent(Path, Parent);

			if (_fs->PathExists(Path, CurrentParent))
			{
				debug("Path \"%s\" already exists.", Path);
				delete[] Path;
				if (Err != nullptr)
					*Err = -EEXIST;
				delete this;
				return;
			}

			const char *CleanPath = _fs->NormalizePath(Path, CurrentParent);

			cwk_segment segment;
			if (!cwk_path_get_first_segment(CleanPath, &segment))
			{
				debug("Path doesn't have any segments.");
				delete[] CleanPath;
				if (Err != nullptr)
					*Err = -EINVAL;
				delete this;
				return;
			}

			cwk_segment last_segment;
			cwk_path_get_last_segment(CleanPath, &last_segment);

			do
			{
				char *SegmentName = new char[segment.end - segment.begin + 1];
				memcpy(SegmentName, segment.begin, segment.end - segment.begin);

				if (Parent)
				{
					if (GetChild(SegmentName, RootNode) != nullptr)
					{
						RootNode = GetChild(SegmentName, RootNode);
						delete[] SegmentName;
						continue;
					}
				}

				if (GetChild(SegmentName, CurrentParent) == nullptr)
				{
					if (segment.begin == last_segment.begin)
					{
						CreateWithParent(SegmentName, CurrentParent);
						delete[] SegmentName;
						break; /* This is the last segment anyway... */
					}

					CurrentParent = new Node(CurrentParent,
											 SegmentName,
											 Type);
				}
				else
				{
					CurrentParent = GetChild(SegmentName, CurrentParent);
				}

				delete[] SegmentName;
			} while (cwk_path_get_next_segment(&segment));
		}
		else if (Parent)
			CreateWithParent(Name, Parent);
		else
		{
			this->Name = new char[strlen(Name) + 1];
			strcpy((char *)this->Name, Name);
			this->FullPath = Name;

			trace("Node %s(%#lx) has no parent",
				  this->Name, this);
		}

		// debug("Created node %s(%#lx)", this->FullPath, this);
	}

	Node::~Node()
	{
		debug("Destroyed node %s(%#lx)", this->FullPath, this);
		// assert(this->Children.size() == 0);

		foreach (auto Child in this->Children)
			delete Child;

		if (this->Parent)
		{
			debug("Removing node %s(%#lx) from parent %s(%#lx)",
				  this->FullPath, this,
				  this->Parent->FullPath, this->Parent);

			this->Parent->Children.erase(std::find(this->Parent->Children.begin(),
												   this->Parent->Children.end(),
												   this));
		}

		delete[] this->Name;
		if (this->Parent)
			delete[] this->FullPath;
		if (this->Symlink)
			delete[] this->Symlink;
	}
}
