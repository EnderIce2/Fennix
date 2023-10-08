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

#include <net/ipv4.hpp>
#include <net/dhcp.hpp>
#include <net/eth.hpp>
#include <net/arp.hpp>
#include <net/ntp.hpp>
#include <net/dns.hpp>
#include <net/udp.hpp>
#include <debug.h>

#include "../kernel.h"
#include "../mapi.hpp"
#include "../Fex.hpp"

/* FIXME: The functions MUST have little endian parameters and return values. */

namespace NetworkInterfaceManager
{
	std::vector<Events *> RegisteredEvents;

	NetworkInterface::NetworkInterface()
	{
		this->vma = new Memory::VirtualMemoryArea(nullptr);
		if (ModuleManager->GetModules().size() > 0)
		{
			foreach (auto Module in ModuleManager->GetModules())
				if (((FexExtended *)Module.ExtendedHeaderAddress)->Module.Type == FexModuleType::FexModuleType_Network)
					this->FetchNetworkCards(Module.modUniqueID);
		}
		else
			KPrint("\eE85230No network drivers found! Cannot fetch network cards!");

		DbgNetwork();
	}

	NetworkInterface::~NetworkInterface()
	{
		debug("Destructor called");

		// Stop the network stack
		this->StopNetworkStack();

		// Unregister all events
		RegisteredEvents.clear();

		forItr(itr, Interfaces) delete *itr;
		Interfaces.clear();

		// Delete all interfaces and their callbacks and free the memory
		delete this->vma, this->vma = nullptr;
	}

	void NetworkInterface::FetchNetworkCards(unsigned long modUniqueID)
	{
		KernelCallback cb{};
		cb.Reason = QueryReason;
		ModuleManager->IOCB(modUniqueID, &cb);

		DeviceInterface *Iface = (DeviceInterface *)vma->RequestPages(TO_PAGES(sizeof(DeviceInterface) + 1));
		strcpy(Iface->Name, cb.NetworkCallback.Fetch.Name);
		Iface->ID = this->CardIDs++;
		Iface->MAC.FromHex(cb.NetworkCallback.Fetch.MAC);
		Iface->DriverID = modUniqueID;
		Interfaces.push_back(Iface);

		foreach (auto var in RegisteredEvents)
			var->OnInterfaceAdded(Iface);

		debug("Network Card: %s; MAC: %#lx", Iface->Name, Iface->MAC.ToHex());
	}

#ifdef DEBUG_NETWORK
#define DbgWriteScreen(x, ...) KPrint(x, ##__VA_ARGS__)
#else
#define DbgWriteScreen(x, ...) debug(x, ##__VA_ARGS__)
#endif

	void NetworkInterface::StartNetworkStack()
	{
		thisThread->SetPriority(Tasking::TaskPriority::Critical);
		DeviceInterface *DefaultDevice = nullptr;
		foreach (auto inf in Interfaces)
			if (inf)
			{
				DefaultDevice = inf;
				break;
			}

		if (!DefaultDevice)
			error("No network device found!");
		else
		{
			DbgWriteScreen("Using %s (%s) as the default network device", DefaultDevice->Name, DefaultDevice->MAC.ToString());
			NetworkEthernet::Ethernet *eth = new NetworkEthernet::Ethernet(DefaultDevice); // Use the first device found as the ethernet device
			NetworkARP::ARP *arp = new NetworkARP::ARP(eth);
			NetworkIPv4::IPv4 *ipv4 = new NetworkIPv4::IPv4(arp, eth);
			NetworkUDP::UDP *udp = new NetworkUDP::UDP(ipv4, DefaultDevice);
			NetworkUDP::Socket *DHCP_Socket = udp->Connect(InternetProtocol() /* Default value is 255.255.255.255 */, 67);
			NetworkDHCP::DHCP *dhcp = new NetworkDHCP::DHCP(DHCP_Socket, DefaultDevice);
			debug("eth: %p; arp: %p; ipv4: %p; udp: %p; dhcp: %p",
				  eth, arp, ipv4, udp, dhcp);
			udp->Bind(DHCP_Socket, dhcp);
			dhcp->Request();

			DefaultDevice->IP.v4.FromHex(dhcp->IP.v4.ToHex());
			ipv4->SubNetworkMaskIP = dhcp->SubNetworkMask;
			ipv4->GatewayIP = dhcp->Gateway;
			arp->Broadcast(dhcp->Gateway);
			TaskManager->Sleep(200);
			DbgWriteScreen("IP: %s", dhcp->IP.v4.ToStringLittleEndian());
			DbgWriteScreen("SubNetwork Mask: %s", dhcp->SubNetworkMask.v4.ToStringLittleEndian());
			DbgWriteScreen("Gateway: %s", dhcp->Gateway.v4.ToStringLittleEndian());
			DbgWriteScreen("DNS: %s", dhcp->DomainNameSystem.v4.ToStringLittleEndian());
			TaskManager->Sleep(200);

			/* TODO: This is a quick workaround we need DNS resolver asap.

			https://tf.nist.gov/tf-cgi/servers.cgi
			https://www.ntppool.org

			- 0.ro.pool.ntp.org ( {86, 127, 71, 168} )
			- time-a-g.nist.gov ( {129, 6, 15, 28} )
			*/
			// InternetProtocol ip = {.v4 = {.Address = {129, 6, 15, 28}},
			//                        .v6 = {.Address = {}}};
			// NetworkUDP::Socket *NTP_Socket = udp->Connect(ip, 123);
			// NetworkNTP::NTP *ntp = new NetworkNTP::NTP(NTP_Socket);
			// udp->Bind(NTP_Socket, ntp);
			// int UnixTimestamp = ntp->ReadTime();
			// Time::Clock time = Time::ConvertFromUnix(UnixTimestamp);
			// DbgWriteScreen("NTP: %d - %d.%d.%d %d:%d:%d", time.Counter,
			//                time.Day, time.Month, time.Year,
			//                time.Hour, time.Minute, time.Second);
			TaskManager->Sleep(200);

			/* TODO: Store everything in an vector and initialize all network cards */
		}

		thisThread->SetPriority(Tasking::TaskPriority::Idle);
		CPU::Halt(true);
	}

	void NetworkInterface::StopNetworkStack()
	{
		fixme("Stop network stack");
	}

	void CallStartNetworkStackWrapper() { NIManager->StartNetworkStack(); }

	void NetworkInterface::StartService()
	{
		this->NetSvcThread = TaskManager->CreateThread(thisProcess, Tasking::IP(CallStartNetworkStackWrapper));
		this->NetSvcThread->Rename("Network Service");
	}

	void NetworkInterface::DrvSend(unsigned int DriverID, unsigned char *Data, unsigned short Size)
	{
		foreach (auto inf in this->Interfaces)
			if (inf->DriverID == DriverID)
				NIManager->Send(inf, Data, Size);
	}

	void NetworkInterface::DrvReceive(unsigned int DriverID, unsigned char *Data, unsigned short Size)
	{
		foreach (auto inf in this->Interfaces)
			if (inf->DriverID == DriverID)
				NIManager->Receive(inf, Data, Size);
	}

	void NetworkInterface::Send(DeviceInterface *Interface, uint8_t *Data, size_t Length)
	{
		void *DataToBeSent = vma->RequestPages(TO_PAGES(Length + 1));
		memcpy(DataToBeSent, Data, Length);

		KernelCallback cb{};
		cb.Reason = SendReason;
		cb.NetworkCallback.Send.Data = (uint8_t *)DataToBeSent;
		cb.NetworkCallback.Send.Length = Length;
		ModuleManager->IOCB(Interface->DriverID, &cb);

		vma->FreePages(DataToBeSent, TO_PAGES(Length + 1));
		foreach (auto ev in RegisteredEvents)
			ev->OnInterfaceSent(Interface, Data, Length);
	}

	void NetworkInterface::Receive(DeviceInterface *Interface, uint8_t *Data, size_t Length)
	{
		foreach (auto re in RegisteredEvents)
			re->OnInterfaceReceived(Interface, Data, Length);
	}

	Events::Events(DeviceInterface *Interface)
	{
		UNUSED(Interface);
		RegisteredEvents.push_back(this);
	}

	Events::~Events()
	{
		forItr(itr, RegisteredEvents)
		{
			if (*itr == this)
			{
				RegisteredEvents.erase(itr);
				break;
			}
		}
	}
}
