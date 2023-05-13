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

#ifndef __FENNIX_KERNEL_NETWORK_CONTROLLER_H__
#define __FENNIX_KERNEL_NETWORK_CONTROLLER_H__

#include <net/net.hpp>
#include <memory.hpp>
#include <task.hpp>
#include <types.h>
#include <debug.h>
#include <vector>

namespace NetworkInterfaceManager
{
    struct DeviceInterface
    {
        /** @brief Device interface name */
        char Name[128];

        /** @brief Device interface index */
        uint64_t ID;

        /** @brief Device interface MAC address (Big-endian) */
        MediaAccessControl MAC;

        /** @brief Device interface IP address (Big-endian) */
        InternetProtocol IP;

        /** @brief Reserved */
        unsigned long DriverID;
    };

    class Events
    {
    protected:
        Events(DeviceInterface *Interface);
        ~Events();

    public:
        virtual void OnInterfaceAdded(DeviceInterface *Interface)
        {
            UNUSED(Interface);
            netdbg("Event for %s not handled.", Interface->Name);
        }

        virtual void OnInterfaceRemoved(DeviceInterface *Interface)
        {
            UNUSED(Interface);
            netdbg("Event for %s not handled.", Interface->Name);
        }

        virtual void OnInterfaceChanged(DeviceInterface *Interface)
        {
            UNUSED(Interface);
            netdbg("Event for %s not handled.", Interface->Name);
        }

        virtual void OnInterfaceReceived(DeviceInterface *Interface, uint8_t *Data, size_t Length)
        {
            UNUSED(Interface);
            UNUSED(Data);
            UNUSED(Length);
            netdbg("Event for %s not handled.", Interface->Name);
        }

        virtual void OnInterfaceSent(DeviceInterface *Interface, uint8_t *Data, size_t Length)
        {
            UNUSED(Interface);
            UNUSED(Data);
            UNUSED(Length);
            netdbg("Event for %s not handled.", Interface->Name);
        }
    };

    class NetworkInterface
    {
    private:
        Memory::MemMgr *mem;
        int CardIDs = 0;
        std::vector<DeviceInterface *> Interfaces;

        Tasking::TCB *NetSvcThread;
        void StopNetworkStack();
        void FetchNetworkCards(unsigned long DriverUID);

    public:
        NetworkInterface();
        ~NetworkInterface();

        void StartService();

        void Send(DeviceInterface *Interface, uint8_t *Data, size_t Length);
        void Receive(DeviceInterface *Interface, uint8_t *Data, size_t Length);

        void DrvSend(unsigned int DriverID, unsigned char *Data, unsigned short Size);
        void DrvReceive(unsigned int DriverID, unsigned char *Data, unsigned short Size);
        void StartNetworkStack();
    };
}

#endif // !__FENNIX_KERNEL_NETWORK_CONTROLLER_H__
