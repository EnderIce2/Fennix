#ifndef __FENNIX_KERNEL_NETWORK_CONTROLLER_H__
#define __FENNIX_KERNEL_NETWORK_CONTROLLER_H__

#include <net/net.hpp>
#include <vector.hpp>
#include <memory.hpp>
#include <task.hpp>
#include <types.h>
#include <debug.h>

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
        void *DriverCallBackAddress;

        /** @brief Reserved */
        unsigned int DriverID;
    };

    class Events
    {
    protected:
        Events(DeviceInterface *Interface);
        ~Events();

    public:
        virtual void OnInterfaceAdded(DeviceInterface *Interface) { netdbg("Event for %s not handled.", Interface->Name); }
        virtual void OnInterfaceRemoved(DeviceInterface *Interface) { netdbg("Event for %s not handled.", Interface->Name); }
        virtual void OnInterfaceChanged(DeviceInterface *Interface) { netdbg("Event for %s not handled.", Interface->Name); }
        virtual void OnInterfaceReceived(DeviceInterface *Interface, uint8_t *Data, uint64_t Length) { netdbg("Event for %s not handled.", Interface->Name); }
        virtual void OnInterfaceSent(DeviceInterface *Interface, uint8_t *Data, uint64_t Length) { netdbg("Event for %s not handled.", Interface->Name); }
    };

    class NetworkInterface
    {
    private:
        Memory::MemMgr *mem;
        int CardIDs = 0;
        Vector<DeviceInterface *> Interfaces;

        Tasking::TCB *NetSvcThread;
        void StopNetworkStack();
        void FetchNetworkCards(unsigned long DriverUID);

    public:
        NetworkInterface();
        ~NetworkInterface();

        void StartService();

        void Send(DeviceInterface *Interface, uint8_t *Data, uint64_t Length);
        void Receive(DeviceInterface *Interface, uint8_t *Data, uint64_t Length);

        void DrvSend(unsigned int DriverID, unsigned char *Data, unsigned short Size);
        void DrvReceive(unsigned int DriverID, unsigned char *Data, unsigned short Size);
        void StartNetworkStack();
    };
}

#endif // !__FENNIX_KERNEL_NETWORK_CONTROLLER_H__
