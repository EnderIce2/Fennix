#include <net/nc.hpp>
#include <net/eth.hpp>
#include <net/arp.hpp>
#include <net/ipv4.hpp>
#include <net/udp.hpp>
#include <net/dhcp.hpp>
#include <debug.h>

#include "../kernel.h"

#include "../DAPI.hpp"
#include "../Fex.hpp"

namespace NetworkInterfaceManager
{
    Vector<Events *> RegisteredEvents;

    NetworkInterface::NetworkInterface()
    {
        mem = new Memory::MemMgr;
        if (DriverManager->GetDrivers().size() > 0)
        {
            foreach (auto Driver in DriverManager->GetDrivers())
                if (((FexExtended *)((uintptr_t)Driver->Address + EXTENDED_SECTION_ADDRESS))->Driver.Type == FexDriverType::FexDriverType_Network)
                    this->FetchNetworkCards(Driver->DriverUID);
        }
        else
            KPrint("\eE85230No network drivers found! Cannot fetch network cards!");

        DbgNetwork();
    }

    NetworkInterface::~NetworkInterface()
    {
        // Stop the network stack
        this->StopNetworkStack();

        // Unregister all events
        for (size_t i = 0; i < RegisteredEvents.size(); i++)
                RegisteredEvents.remove(i);

        // Delete all interfaces and their callbacks and free the memory
        delete mem;
    }

    void NetworkInterface::FetchNetworkCards(unsigned long DriverUID)
    {
        KernelCallback *cb = (KernelCallback *)mem->RequestPages(TO_PAGES(sizeof(KernelCallback)));
        memset(cb, 0, sizeof(KernelCallback));
        cb->Reason = FetchReason;
        DriverManager->IOCB(DriverUID, (void *)cb);

        DeviceInterface *Iface = (DeviceInterface *)mem->RequestPages(TO_PAGES(sizeof(DeviceInterface)));
        strcpy(Iface->Name, cb->NetworkCallback.Fetch.Name);
        Iface->ID = this->CardIDs++;
        Iface->MAC.FromHex(cb->NetworkCallback.Fetch.MAC);
        Iface->DriverCallBackAddress = cb;
        Iface->DriverID = DriverUID;
        Interfaces.push_back(Iface);

        foreach (auto var in RegisteredEvents)
            var->OnInterfaceAdded(Iface);

        debug("Network Card: %s; MAC: %#lx", Iface->Name, Iface->MAC.ToHex());
    }

    void NetworkInterface::StartNetworkStack()
    {
        TaskManager->GetCurrentThread()->SetPriority(100);
        DeviceInterface *DefaultDevice = nullptr;
        foreach (auto var in Interfaces)
            if (var && var->DriverCallBackAddress)
            {
                DefaultDevice = var;
                break;
            }

        if (!DefaultDevice)
            error("No network device found!");
        else
        {
            NetworkEthernet::Ethernet *eth = new NetworkEthernet::Ethernet(DefaultDevice); // Use the first device found as the ethernet device
            NetworkARP::ARP *arp = new NetworkARP::ARP(eth);
            NetworkIPv4::IPv4 *ipv4 = new NetworkIPv4::IPv4(arp, eth);
            NetworkUDP::UDP *udp = new NetworkUDP::UDP(ipv4, DefaultDevice);
            NetworkUDP::Socket *DHCP_Socket = udp->Connect({.Address = {0xFF, 0xFF, 0xFF, 0xFF}}, 67);
            NetworkDHCP::DHCP *dhcp = new NetworkDHCP::DHCP(DHCP_Socket, DefaultDevice);
            udp->Bind(DHCP_Socket, dhcp);
            dhcp->Request();
            DefaultDevice->IP = InternetProtocol4().FromHex(b32(dhcp->IP.ToHex()));
            ipv4->SubNetworkMaskIP = dhcp->SubNetworkMask;
            ipv4->GatewayIP = dhcp->Gateway;
            arp->Broadcast(dhcp->Gateway);
            trace("IP: %d.%d.%d.%d", dhcp->IP.Address[3], dhcp->IP.Address[2], dhcp->IP.Address[1], dhcp->IP.Address[0]);
            trace("SubNetwork Mask: %d.%d.%d.%d", dhcp->SubNetworkMask.Address[3], dhcp->SubNetworkMask.Address[2], dhcp->SubNetworkMask.Address[1], dhcp->SubNetworkMask.Address[0]);
            trace("Gateway: %d.%d.%d.%d", dhcp->Gateway.Address[3], dhcp->Gateway.Address[2], dhcp->Gateway.Address[1], dhcp->Gateway.Address[0]);
            trace("DNS: %d.%d.%d.%d", dhcp->DomainNameSystem.Address[3], dhcp->DomainNameSystem.Address[2], dhcp->DomainNameSystem.Address[1], dhcp->DomainNameSystem.Address[0]);

            /* TODO: Store everything in an vector and initialize all network cards */
        }

        TaskManager->GetCurrentThread()->SetPriority(1);
        CPU::Pause(true);
    }

    void NetworkInterface::StopNetworkStack()
    {
        fixme("Stop network stack");
    }

    ReadFSFunction(NetRead)
    {
        fixme("Not implemented.");
        return Size;
    }

    WriteFSFunction(NetWrite)
    {
        fixme("Not implemented.");
        return Size;
    }

    void CallStartNetworkStackWrapper() { NIManager->StartNetworkStack(); }

    void NetworkInterface::StartService()
    {
        this->NetSvcProcess = TaskManager->CreateProcess(TaskManager->GetCurrentProcess(), "Network Service", Tasking::TaskTrustLevel::System);
        Vector<AuxiliaryVector> auxv;
        auxv.push_back({.archaux = {.a_type = AT_NULL, .a_un = {.a_val = 0}}});
        this->NetSvcThread = TaskManager->CreateThread(this->NetSvcProcess, (Tasking::IP)CallStartNetworkStackWrapper, nullptr, nullptr, auxv);
    }

    void NetworkInterface::DrvSend(unsigned int DriverID, unsigned char *Data, unsigned short Size)
    {
        foreach (auto var in this->Interfaces)
            if (var->DriverID == DriverID)
                NIManager->Send(var, Data, Size);
    }

    void NetworkInterface::DrvReceive(unsigned int DriverID, unsigned char *Data, unsigned short Size)
    {
        foreach (auto var in this->Interfaces)
            if (var->DriverID == DriverID)
                NIManager->Receive(var, Data, Size);
    }

    void NetworkInterface::Send(DeviceInterface *Interface, uint8_t *Data, uint64_t Length)
    {
        void *DataToBeSent = mem->RequestPages(TO_PAGES(Length));
        memcpy(DataToBeSent, Data, Length);

        KernelCallback *cb = (KernelCallback *)Interface->DriverCallBackAddress;

        memset(cb, 0, sizeof(KernelCallback));
        cb->Reason = SendReason;
        cb->NetworkCallback.Send.Data = (uint8_t *)DataToBeSent;
        cb->NetworkCallback.Send.Length = Length;
        DriverManager->IOCB(Interface->DriverID, (void *)cb);

        mem->FreePages(DataToBeSent, TO_PAGES(Length));
        foreach (auto var in RegisteredEvents)
            var->OnInterfaceSent(Interface, Data, Length);
    }

    void NetworkInterface::Receive(DeviceInterface *Interface, uint8_t *Data, uint64_t Length)
    {
        foreach (auto var in RegisteredEvents)
            var->OnInterfaceReceived(Interface, Data, Length);
    }

    Events::Events(DeviceInterface *Interface) { RegisteredEvents.push_back(this); }

    Events::~Events()
    {
        for (size_t i = 0; i < RegisteredEvents.size(); i++)
            if (RegisteredEvents[i] == this)
            {
                RegisteredEvents.remove(i);
                return;
            }
    }
}
