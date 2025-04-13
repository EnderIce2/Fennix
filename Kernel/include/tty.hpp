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

#ifndef __FENNIX_KERNEL_TTY_H__
#define __FENNIX_KERNEL_TTY_H__

#include <termios.h>
#include <mutex>

namespace TTY
{
	class TerminalBuffer
	{
	private:
		std::vector<char> Buffer;
		size_t ReadIndex;
		size_t WriteIndex;
		std::mutex Mutex;

	public:
		TerminalBuffer(size_t Size) : Buffer(Size), ReadIndex(0), WriteIndex(0) {}

		ssize_t Read(char *OutputBuffer, size_t Size)
		{
			std::lock_guard<std::mutex> lock(Mutex);
			size_t bytesRead = 0;

			while (bytesRead < Size && ReadIndex != WriteIndex)
			{
				OutputBuffer[bytesRead++] = Buffer[ReadIndex];
				ReadIndex = (ReadIndex + 1) % Buffer.size();
			}

			return bytesRead;
		}

		ssize_t Write(const char *InputBuffer, size_t Size)
		{
			std::lock_guard<std::mutex> lock(Mutex);
			size_t bytesWritten = 0;

			for (size_t i = 0; i < Size; ++i)
			{
				Buffer[WriteIndex] = InputBuffer[i];
				WriteIndex = (WriteIndex + 1) % Buffer.size();
				bytesWritten++;
			}

			return bytesWritten;
		}

		void DrainOutput()
		{
			std::lock_guard<std::mutex> lock(Mutex);
			ReadIndex = WriteIndex;
		}

		size_t AvailableToRead() const
		{
			return (WriteIndex - ReadIndex + Buffer.size()) % Buffer.size();
		}

		size_t AvailableToWrite() const
		{
			return Buffer.size() - AvailableToRead() - 1;
		}
	};

	class TeletypeDriver
	{
	protected:
		termios TerminalConfig{};
		winsize TerminalSize{};
		TerminalBuffer TermBuf;
		pid_t ProcessGroup;

	public:
		virtual int Open(int Flags, mode_t Mode);
		virtual int Close();
		virtual ssize_t Read(void *Buffer, size_t Size, off_t Offset);
		virtual ssize_t Write(const void *Buffer, size_t Size, off_t Offset);
		virtual int Ioctl(unsigned long Request, void *Argp);

		TeletypeDriver();
		virtual ~TeletypeDriver() = default;
	};

	class PTYDevice
	{
	private:
		class PTYMaster
		{
		private:
			TerminalBuffer TermBuf;

		public:
			PTYMaster() : TermBuf(1024) {}
			~PTYMaster() = default;

			ssize_t Read(void *Buffer, size_t Size)
			{
				return TermBuf.Read((char *)Buffer, Size);
			}

			ssize_t Write(const void *Buffer, size_t Size)
			{
				return TermBuf.Write((const char *)Buffer, Size);
			}
		};

		class PTYSlave
		{
		private:
			TerminalBuffer TermBuf;

		public:
			PTYSlave() : TermBuf(1024) {}
			~PTYSlave() = default;

			ssize_t Read(void *Buffer, size_t Size)
			{
				return TermBuf.Read((char *)Buffer, Size);
			}

			ssize_t Write(const void *Buffer, size_t Size)
			{
				return TermBuf.Write((const char *)Buffer, Size);
			}
		};

		PTYMaster Master;
		PTYSlave Slave;

	public:
		PTYDevice() : Master(), Slave() {}
		~PTYDevice() = default;

		int Open()
		{
			stub;
			return -ENOSYS;
		}

		int Close()
		{
			stub;
			return -ENOSYS;
		}

		ssize_t Read(void *Buffer, size_t Size)
		{
			return Slave.Read(Buffer, Size);
		}

		ssize_t Write(const void *Buffer, size_t Size)
		{
			return Master.Write(Buffer, Size);
		}
	};

	class PTMXDevice
	{
	private:
		std::vector<PTYDevice *> PTYs;
		std::mutex PTYMutex;

	public:
		PTMXDevice() = default;

		~PTMXDevice()
		{
			for (auto pty : PTYs)
				delete pty;
		}

		int Open();
		int Close();
		PTYDevice *CreatePTY();
		int RemovePTY(PTYDevice *pty);
	};
}

#endif // !__FENNIX_KERNEL_TTY_H__
