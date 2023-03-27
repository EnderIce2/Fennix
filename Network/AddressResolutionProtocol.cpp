#include <net/arp.hpp>
#include <debug.h>
#include <smart_ptr.hpp>

#include "../kernel.h"

/* conversion from ‘uint48_t’ {aka ‘long unsigned int’} to ‘long unsigned int:48’ may change value */
#pragma GCC diagnostic ignored "-Wconversion"

namespace NetworkARP
{
    DiscoveredAddress *ARP::ManageDiscoveredAddresses(DAType Type, InternetProtocol IP, MediaAccessControl MAC)
    {
        switch (Type)
        {
        case DA_ADD:
        {
            DiscoveredAddress *tmp = new DiscoveredAddress;
            tmp->IP = IP;
            tmp->MAC = MAC;
            DiscoveredAddresses.push_back(tmp);
            netdbg("Added %s to discovered addresses", IP.v4.ToStringLittleEndian());
            return tmp;
        }
        case DA_DEL:
        {
            for (size_t i = 0; i < DiscoveredAddresses.size(); i++)
            {
                if (DiscoveredAddresses[i]->IP.v4 == IP.v4)
                {
                    DiscoveredAddress *tmp = DiscoveredAddresses[i];
                    netdbg("Removed %s from discovered addresses", IP.v4.ToStringLittleEndian());
                    delete tmp, tmp = nullptr;
                    DiscoveredAddresses.remove(i);
                }
            }
            return nullptr;
        }
        case DA_SEARCH:
        {
            for (size_t i = 0; i < DiscoveredAddresses.size(); i++)
            {
                if (DiscoveredAddresses[i]->IP.v4 == IP.v4)
                {
                    netdbg("Found %s in discovered addresses", IP.v4.ToStringLittleEndian());
                    return DiscoveredAddresses[i];
                }
            }
            return nullptr;
        }
        case DA_UPDATE:
        {
            for (size_t i = 0; i < DiscoveredAddresses.size(); i++)
            {
                if (DiscoveredAddresses[i]->IP.v4 == IP.v4)
                {
                    DiscoveredAddresses[i]->MAC = MAC;
                    netdbg("Updated %s in discovered addresses", IP.v4.ToStringLittleEndian());
                    return DiscoveredAddresses[i];
                }
            }
            return nullptr;
        }
        default:
        {
            return nullptr;
        }
        }
        return nullptr;
    }

    ARP::ARP(NetworkEthernet::Ethernet *Ethernet) : NetworkEthernet::EthernetEvents(NetworkEthernet::TYPE_ARP)
    {
        debug("ARP interface %#lx created.", this);
        this->Ethernet = Ethernet;
    }

    ARP::~ARP()
    {
        debug("ARP interface %#lx destroyed.", this);
    }

    MediaAccessControl InvalidMAC;
    InternetProtocol InvalidIP;
    DiscoveredAddress InvalidRet = {.MAC = InvalidMAC, .IP = InvalidIP};

    DiscoveredAddress *ARP::Search(InternetProtocol TargetIP)
    {
        DiscoveredAddress *ret = ManageDiscoveredAddresses(DA_SEARCH, TargetIP, MediaAccessControl());
        if (ret)
            return ret;
        warn("No address found for %s", TargetIP.v4.ToStringLittleEndian());
        return &InvalidRet;
    }

    DiscoveredAddress *ARP::Update(InternetProtocol TargetIP, MediaAccessControl TargetMAC)
    {
        DiscoveredAddress *ret = ManageDiscoveredAddresses(DA_UPDATE, TargetIP, TargetMAC);
        if (ret)
            return ret;
        warn("No address found for %s", TargetIP.v4.ToStringLittleEndian());
        return &InvalidRet;
    }

    uint48_t ARP::Resolve(InternetProtocol IP)
    {
        netdbg("Resolving %s", IP.v4.ToStringLittleEndian());
        if (IP.v4 == 0xFFFFFFFF)
            return 0xFFFFFFFFFFFF;

        /* If we can't find the MAC, return the default value which is 0xFFFFFFFFFFFF. */
        uint48_t ret = this->Search(IP)->MAC.ToHex();
        netdbg("Resolved %s to %lx", IP.v4.ToStringLittleEndian(), ret);

        if (ret == 0xFFFFFFFFFFFF)
        {
            netdbg("Sending request");
            /* If we can't find the MAC, send a request.
               Because we are going to send this over the network, we need to byteswap first.
               This is actually very confusing for me. */
            ARPHeader *Header = new ARPHeader;
            Header->HardwareType = b16(ARPHardwareType::HTYPE_ETHERNET);
            Header->ProtocolType = b16(NetworkEthernet::FrameType::TYPE_IPV4);
            Header->HardwareSize = b8(6);
            Header->ProtocolSize = b8(4);
            Header->Operation = b16(ARPOperation::REQUEST);
            Header->SenderMAC = b48(Ethernet->GetInterface()->MAC.ToHex());
            Header->SenderIP = b32(Ethernet->GetInterface()->IP.v4.ToHex());
            Header->TargetMAC = b48(0xFFFFFFFFFFFF);
            Header->TargetIP = b32(IP.v4.ToHex());
            netdbg("SIP: %s; TIP: %s", InternetProtocol().v4.FromHex(Header->SenderIP).ToStringLittleEndian(),
                   InternetProtocol().v4.FromHex(Header->TargetIP).ToStringLittleEndian());
            /* Send the request to the broadcast MAC address. */
            Ethernet->Send({.Address = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}, NetworkEthernet::FrameType::TYPE_ARP, (uint8_t *)Header, sizeof(ARPHeader));
            delete Header, Header = nullptr;
        }

        int RequestTimeout = 20;
        debug("Waiting for response");
        while (ret == 0xFFFFFFFFFFFF)
        {
            ret = this->Search(IP)->MAC.ToHex();
            if (--RequestTimeout == 0)
            {
                warn("Request timeout.");
                return 0;
            }
            netdbg("Still waiting...");
            TaskManager->Sleep(1000);
        }

        return ret;
    }

    void ARP::Broadcast(InternetProtocol IP)
    {
        netdbg("Sending broadcast");
        uint48_t ResolvedMAC = this->Resolve(IP);
        ARPHeader *Header = new ARPHeader;
        Header->HardwareType = b16(ARPHardwareType::HTYPE_ETHERNET);
        Header->ProtocolType = b16(NetworkEthernet::FrameType::TYPE_IPV4);
        Header->HardwareSize = b8(0x6);
        Header->ProtocolSize = b8(0x4);
        Header->Operation = b16(ARPOperation::REQUEST);
        Header->SenderMAC = b48(Ethernet->GetInterface()->MAC.ToHex());
        Header->SenderIP = b32(Ethernet->GetInterface()->IP.v4.ToHex());
        Header->TargetMAC = b48(ResolvedMAC);
        Header->TargetIP = b32(IP.v4.ToHex());
        Ethernet->Send(MediaAccessControl().FromHex(ResolvedMAC), NetworkEthernet::FrameType::TYPE_ARP, (uint8_t *)Header, sizeof(ARPHeader));
        delete Header, Header = nullptr;
    }

    bool ARP::OnEthernetPacketReceived(uint8_t *Data, uint64_t Length)
    {
        netdbg("Received packet");
        ARPHeader *Header = (ARPHeader *)Data;

        InternetProtocol SenderIPv4;
        SenderIPv4.v4 = SenderIPv4.v4.FromHex(b32(Header->SenderIP));

        /* We only support Ethernet and IPv4. */
        if (b16(Header->HardwareType) != ARPHardwareType::HTYPE_ETHERNET || b16(Header->ProtocolType) != NetworkEthernet::FrameType::TYPE_IPV4)
        {
            warn("Invalid hardware/protocol type (%d/%d)", b16(Header->HardwareType), b16(Header->ProtocolType));
            return false;
        }

        InternetProtocol TmpIPStruct;
        InternetProtocol().v4.FromHex(b32(Header->SenderIP));

        if (ManageDiscoveredAddresses(DA_SEARCH, TmpIPStruct, MediaAccessControl().FromHex(b48(Header->SenderMAC))) == nullptr)
        {
            netdbg("Discovered new address %s", SenderIPv4.v4.ToStringLittleEndian());
            TmpIPStruct.v4.FromHex(b32(Header->SenderIP));
            ManageDiscoveredAddresses(DA_ADD, TmpIPStruct, MediaAccessControl().FromHex(b48(Header->SenderMAC)));
        }
        else
        {
            netdbg("Updated address %s", SenderIPv4.v4.ToStringLittleEndian());
            TmpIPStruct.v4.FromHex(b32(Header->SenderIP));
            ManageDiscoveredAddresses(DA_UPDATE, TmpIPStruct, MediaAccessControl().FromHex(b48(Header->SenderMAC)));
        }

        switch (b16(Header->Operation))
        {
        case ARPOperation::REQUEST:
            netdbg("Received request from %s", SenderIPv4.v4.ToStringLittleEndian());
            /* We need to byteswap before we send it back. */
            Header->TargetMAC = b48(Header->SenderMAC);
            Header->TargetIP = b32(Header->SenderIP);
            Header->SenderMAC = b48(Ethernet->GetInterface()->MAC.ToHex());
            Header->SenderIP = b32(Ethernet->GetInterface()->IP.v4.ToHex());
            Header->Operation = b16(ARPOperation::REPLY);
            Ethernet->Send(MediaAccessControl().FromHex(Header->TargetMAC), NetworkEthernet::FrameType::TYPE_ARP, (uint8_t *)Header, sizeof(ARPHeader));
            netdbg("Sent request for %s", SenderIPv4.v4.ToStringLittleEndian());
            break;
        case ARPOperation::REPLY:
            fixme("Received reply from %s", SenderIPv4.v4.ToStringLittleEndian());
            break;
        default:
            warn("Invalid operation (%d)", b16(Header->Operation));
            break;
        }
        return false;
    }
}
