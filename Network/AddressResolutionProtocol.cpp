#include <net/arp.hpp>
#include <debug.h>
#include <smartptr.hpp>

#include "../kernel.h"

namespace NetworkARP
{
    DiscoveredAddress *ARP::ManageDA(DAType Type, InternetProtocol4 IP, MediaAccessControl MAC)
    {
        // TODO: Compare IPv6 too.
        switch (Type)
        {
        case DA_ADD:
        {
            DiscoveredAddress *tmp = new DiscoveredAddress;
            tmp->IP = IP;
            tmp->MAC = MAC;
            DiscoveredAddresses.push_back(tmp);
            return tmp;
        }
        case DA_DEL:
        {
            for (size_t i = 0; i < DiscoveredAddresses.size(); i++)
                if (DiscoveredAddresses[i]->IP == IP)
                {
                    DiscoveredAddress *tmp = DiscoveredAddresses[i];
                    delete tmp;
                    DiscoveredAddresses.remove(i);
                }
            return nullptr;
        }
        case DA_SEARCH:
        {
            for (size_t i = 0; i < DiscoveredAddresses.size(); i++)
                if (DiscoveredAddresses[i]->IP == IP)
                    return DiscoveredAddresses[i];

            return nullptr;
        }
        case DA_UPDATE:
        {
            for (size_t i = 0; i < DiscoveredAddresses.size(); i++)
                if (DiscoveredAddresses[i]->IP == IP)
                {
                    DiscoveredAddresses[i]->MAC = MAC;
                    return DiscoveredAddresses[i];
                }
            return nullptr;
        }
        }
        return nullptr;
    }

    ARP::ARP(NetworkEthernet::Ethernet *Ethernet) : NetworkEthernet::EthernetEvents(NetworkEthernet::TYPE_ARP)
    {
        netdbg("Initializing.");
        this->Ethernet = Ethernet;
    }

    ARP::~ARP()
    {
    }

    MediaAccessControl InvalidMAC = {.Address = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
    InternetProtocol4 InvalidIP = {.Address = {0xFF, 0xFF, 0xFF, 0xFF}};
    DiscoveredAddress InvalidRet = {.MAC = InvalidMAC, .IP = InvalidIP};

    DiscoveredAddress *ARP::Search(InternetProtocol4 TargetIP)
    {
        DiscoveredAddress *ret = ManageDA(DA_SEARCH, TargetIP, MediaAccessControl());
        if (ret)
            return ret;
        netdbg("[DA] No address found for %d.%d.%d.%d", TargetIP.Address[0], TargetIP.Address[1], TargetIP.Address[2], TargetIP.Address[3]);
        return &InvalidRet;
    }

    DiscoveredAddress *ARP::Update(InternetProtocol4 TargetIP, MediaAccessControl TargetMAC)
    {
        DiscoveredAddress *ret = ManageDA(DA_UPDATE, TargetIP, TargetMAC);
        if (ret)
            return ret;
        warn("[DA] No address found for %d.%d.%d.%d", TargetIP.Address[0], TargetIP.Address[1], TargetIP.Address[2], TargetIP.Address[3]);
        return &InvalidRet;
    }

    uint48_t ARP::Resolve(InternetProtocol4 IP)
    {
        netdbg("Resolving %d.%d.%d.%d", IP.Address[3], IP.Address[2], IP.Address[1], IP.Address[0]);
        if (IP == 0xFFFFFFFF)
            return 0xFFFFFFFFFFFF;

        uint48_t ret = this->Search(IP)->MAC.ToHex();
        netdbg("Resolved %d.%d.%d.%d to %x", IP.Address[3], IP.Address[2], IP.Address[1], IP.Address[0], ret);

        if (ret == 0xFFFFFFFFFFFF)
        {
            ARPHeader *Header = new ARPHeader;
            Header->HardwareType = ARPHardwareType::HTYPE_ETHERNET;
            Header->ProtocolType = NetworkEthernet::FrameType::TYPE_IPV4;
            Header->HardwareSize = 6;
            Header->ProtocolSize = 4;
            Header->Operation = ARPOperation::REQUEST;
            Header->SenderMAC = Ethernet->GetInterface()->MAC.ToHex();
            Header->SenderIP = Ethernet->GetInterface()->IP.ToHex();
            Header->TargetMAC = 0xFFFFFFFFFFFF;
            Header->TargetIP = IP.ToHex();
            Ethernet->Send({.Address = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}, NetworkEthernet::FrameType::TYPE_ARP, (uint8_t *)Header, sizeof(ARPHeader));
            delete Header;
            netdbg("Sent request");
        }

        int RequestTimeout = 10;
        debug("Waiting for response");
        while (ret == 0xFFFFFFFFFFFF)
        {
            ret = this->Search(IP)->MAC.ToHex();
            if (--RequestTimeout == 0)
            {
                warn("Request timeout.");
                return 0;
            }
            TaskManager->Sleep(5000);
        }

        return ret;
    }

    void ARP::Broadcast(InternetProtocol4 IP)
    {
        netdbg("Sending broadcast");
        uint64_t ResolvedMAC = this->Resolve(IP);
        ARPHeader *Header = new ARPHeader;
        Header->HardwareType = b16(ARPHardwareType::HTYPE_ETHERNET);
        Header->ProtocolType = b16(NetworkEthernet::FrameType::TYPE_IPV4);
        Header->HardwareSize = b8(0x6);
        Header->ProtocolSize = b8(0x4);
        Header->Operation = b16(ARPOperation::REQUEST);
        Header->SenderMAC = b48(Ethernet->GetInterface()->MAC.ToHex());
        Header->SenderIP = b32(Ethernet->GetInterface()->IP.ToHex());
        Header->TargetMAC = ResolvedMAC;
        Header->TargetIP = IP.ToHex();
        Ethernet->Send(MediaAccessControl().FromHex(ResolvedMAC), NetworkEthernet::FrameType::TYPE_ARP, (uint8_t *)Header, sizeof(ARPHeader));
        delete Header;
    }

    bool ARP::OnEthernetPacketReceived(uint8_t *Data, uint64_t Length)
    {
        netdbg("Received packet");
        ARPHeader *Header = (ARPHeader *)Data;

        InternetProtocol4 SenderIPv4;
        SenderIPv4.FromHex(b32(Header->SenderIP));

        if (b16(Header->HardwareType) != ARPHardwareType::HTYPE_ETHERNET || b16(Header->ProtocolType) != NetworkEthernet::FrameType::TYPE_IPV4)
        {
            warn("[DA] Invalid hardware/protocol type (%d/%d)", Header->HardwareType, Header->ProtocolType);
            return false;
        }

        if (ManageDA(DA_SEARCH, InternetProtocol4().FromHex(Header->SenderIP), MediaAccessControl().FromHex(Header->SenderMAC)) == nullptr)
        {
            netdbg("[DA] Discovered new address %d.%d.%d.%d", SenderIPv4.Address[3], SenderIPv4.Address[2], SenderIPv4.Address[1], SenderIPv4.Address[0]);
            ManageDA(DA_ADD, InternetProtocol4().FromHex(Header->SenderIP), MediaAccessControl().FromHex(Header->SenderMAC));
        }
        else
        {
            netdbg("[DA] Updated address %d.%d.%d.%d", SenderIPv4.Address[3], SenderIPv4.Address[2], SenderIPv4.Address[1], SenderIPv4.Address[0]);
            ManageDA(DA_UPDATE, InternetProtocol4().FromHex(Header->SenderIP), MediaAccessControl().FromHex(Header->SenderMAC));
        }

        switch (b16(Header->Operation))
        {
        case ARPOperation::REQUEST:
            netdbg("Received request from %d.%d.%d.%d", SenderIPv4.Address[3], SenderIPv4.Address[2], SenderIPv4.Address[1], SenderIPv4.Address[0]);
            Header->TargetMAC = Header->SenderMAC;
            Header->TargetIP = Header->SenderIP;
            Header->SenderMAC = b48(Ethernet->GetInterface()->MAC.ToHex());
            Header->SenderIP = b32(Ethernet->GetInterface()->IP.ToHex());
            Header->Operation = b16(ARPOperation::REPLY);
            Ethernet->Send(MediaAccessControl().FromHex(Header->TargetMAC), NetworkEthernet::FrameType::TYPE_ARP, (uint8_t *)Header, sizeof(ARPHeader));
            netdbg("Sent request for %d.%d.%d.%d", SenderIPv4.Address[0], SenderIPv4.Address[1], SenderIPv4.Address[2], SenderIPv4.Address[3]);
            break;
        case ARPOperation::REPLY:
            fixme("Received reply from %d.%d.%d.%d", SenderIPv4.Address[0], SenderIPv4.Address[1], SenderIPv4.Address[2], SenderIPv4.Address[3]);
            break;
        default:
            warn("Invalid operation (%d)", Header->Operation);
            break;
        }
        return false;
    }
}
