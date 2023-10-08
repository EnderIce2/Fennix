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

#ifndef __FENNIX_KERNEL_NTP_H__
#define __FENNIX_KERNEL_NTP_H__

#include <net/udp.hpp>
#include <types.h>

namespace NetworkNTP
{
    struct NTPHeader
    {
        /** @brief Leap indicator
         * 00 - no warning
         * 01 - last minute has 61 seconds
         * 10 - last minute has 59 seconds
         * 11 - alarm condition (clock not synchronized)
         */
        uint8_t LeapIndicator : 2;
        /** @brief Version number of the protocol
         * 3 - IPv4 only
         * 4 - IPv4, IPv6 and OSI
         * 5 - IPv4, IPv6 and OSI
         * 6 - IPv4, IPv6 and OSI
         * 7 - IPv4, IPv6 and OSI
         */
        uint8_t VersionNumber : 3;
        /** @brief Mode
         * 0 - reserved
         * 1 - symmetric active
         * 2 - symmetric passive
         * 3 - client
         * 4 - server
         * 5 - broadcast
         * 6 - reserved for NTP control message
         * 7 - reserved for private use
         */
        uint8_t Mode : 3;
        /** @brief Stratum
         * 0 - unspecified or unavailable
         * 1 - primary reference (e.g. radio clock)
         * 2-15 - secondary reference (via NTP or SNTP)
         * 16 - unsynchronized
         * 17-255 - reserved
         */
        uint8_t Stratum;
        /** @brief Polling interval
         * 4 - 16 seconds
         * 5 - 32 seconds
         * 6 - 64 seconds
         * 7 - 128 seconds
         * 8 - 256 seconds
         * 9 - 512 seconds
         * 10 - 1024 seconds
         * 11 - 2048 seconds
         * 12 - 4096 seconds
         * 13 - 8192 seconds
         * 14 - 16384 seconds
         * 15 - 32768 seconds
         */
        uint8_t Poll;
        /** @brief Precision
         * -6 - 0.015625 seconds
         * -5 - 0.03125 seconds
         * -4 - 0.0625 seconds
         * -3 - 0.125 seconds
         * -2 - 0.25 seconds
         * -1 - 0.5 seconds
         * 0 - 1 second
         * 1 - 2 seconds
         * 2 - 4 seconds
         * 3 - 8 seconds
         * 4 - 16 seconds
         * 5 - 32 seconds
         * 6 - 64 seconds
         * 7 - 128 seconds
         */
        uint8_t Precision;
        /** @brief Root delay
         * Total round trip delay to the primary reference source
         */
        uint32_t RootDelay;
        /** @brief Root dispersion
         * Nominal error relative to the primary reference source
         */
        uint32_t RootDispersion;
        /** @brief Reference identifier
         * 0x00000000 - unspecified
         * 0x00000001 - radio clock
         * 0x00000002 - atomic clock
         * 0x00000003 - GPS receiver
         * 0x00000004 - local oscillator
         * 0x00000005 - LORAN-C receiver
         * 0x00000006 - microprocessor
         * 0x00000007 - internet
         * 0x00000008 - FLL
         * 0x00000009 - other
         * 0x0000000A - WWV
         * 0x0000000B - WWVB
         * 0x0000000C - WWVH
         * 0x0000000D - NIST dialup
         * 0x0000000E - telephone
         * 0x0000000F - reserved
         */
        uint32_t ReferenceIdentifier;
        /** @brief Reference timestamp
         * The time at which the clock was last set or corrected
         */
        uint32_t ReferenceTimestamp[2];
        /** @brief Originate timestamp
         * The time at which the request departed the client for the server
         */
        uint32_t OriginateTimestamp[2];
        /** @brief Receive timestamp
         * The time at which the request arrived at the server
         */
        uint32_t ReceiveTimestamp[2];
        /** @brief Transmit timestamp
         * The time at which the reply departed the server for the client
         */
        uint32_t TransmitTimestamp[2];
        /** @brief Message authentication code
         * Key identifier
         * @note Only when the NTP authentication scheme is used
         */
        // uint32_t MessageAuthenticationCode;
    } __packed;

    class NTP : public NetworkUDP::UDPEvents
    {
    private:
        NetworkUDP::Socket *UDPSocket;
        bool TimeReceived = false;
        NTPHeader NTPPacket;

        virtual void OnUDPPacketReceived(NetworkUDP::Socket *Socket, uint8_t *Data, size_t Length);

    public:
        NTP(NetworkUDP::Socket *Socket);
        ~NTP();

        /**
         * @brief Get the time from the NTP server
         * 
         * @return Unix Timestamp
         */
        int ReadTime();
    };
}

#endif // !__FENNIX_KERNEL_NTP_H__
