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

#include <net/nc.hpp>

/*
TODO: This code is a mess. It needs to be cleaned up.
*/

#ifdef DEBUG_NETWORK

#include <uart.hpp>
#include <printf.h>
#include <lock.hpp>

NewLock(netdbg_lock);

namespace NetDbg
{
	class NETWORK_DEBUG : public NetworkInterfaceManager::Events
	{
	public:
		static inline void print_wrapper(char c, void *unused) { UART(COM1).Write(c); }
		int vprintf(const char *format, va_list list) { return vfctprintf(print_wrapper, NULL, format, list); }
		void WriteRaw(const char *format, ...)
		{
			va_list args;
			va_start(args, format);
			vprintf(format, args);
			va_end(args);
		}

		void DumpData(const char *Description, void *Address, unsigned long Length)
		{
			SmartLock(netdbg_lock);
			WriteRaw("-------------------------------------------------------------------------\n");
			unsigned char *AddressChar = (unsigned char *)Address;
			unsigned char Buffer[17];
			unsigned long Iterate;

			if (Description != nullptr)
				WriteRaw("%s:\n", Description);

			for (Iterate = 0; Iterate < Length; Iterate++)
			{
				if ((Iterate % 16) == 0)
				{
					if (Iterate != 0)
						WriteRaw("  %s\n", Buffer);
					WriteRaw("  %04x ", Iterate);
				}

				WriteRaw(" %02x", AddressChar[Iterate]);

				if ((AddressChar[Iterate] < 0x20) || (AddressChar[Iterate] > 0x7e))
					Buffer[Iterate % 16] = '.';
				else
					Buffer[Iterate % 16] = AddressChar[Iterate];

				Buffer[(Iterate % 16) + 1] = '\0';
			}

			while ((Iterate % 16) != 0)
			{
				WriteRaw("   ");
				Iterate++;
			}

			WriteRaw("  %s\n", Buffer);
			WriteRaw("-------------------------------------------------------------------------\n");
		}

		NETWORK_DEBUG() : NetworkInterfaceManager::Events(nullptr) { netdbg("NetworkDebugger initialized."); }
		~NETWORK_DEBUG() { netdbg("NetworkDebugger destroyed."); }
		void OnInterfaceReceived(NetworkInterfaceManager::DeviceInterface *Interface, uint8_t *Data, size_t Length) { DumpData("Received", Data, Length); }
		void OnInterfaceSent(NetworkInterfaceManager::DeviceInterface *Interface, uint8_t *Data, size_t Length) { DumpData("Sent", Data, Length); }
	};
}

NetDbg::NETWORK_DEBUG *N;
void DbgNetwork() { N = new NetDbg::NETWORK_DEBUG; }
void DbgDumpData(const char *Description, void *Address, unsigned long Length) { N->DumpData(Description, Address, Length); }

#endif
