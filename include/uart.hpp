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

#ifndef __FENNIX_KERNEL_UART_H__
#define __FENNIX_KERNEL_UART_H__

#include <types.h>

namespace UniversalAsynchronousReceiverTransmitter
{
    /**
     * @brief Serial ports. (if available)
     */
    enum SerialPorts
    {
        COMNULL = 0,
        COM1 = 0x3F8,
        COM2 = 0x2F8,
        COM3 = 0x3E8,
        COM4 = 0x2E8,
        COM5 = 0x5F8,
        COM6 = 0x4F8,
        COM7 = 0x5E8,
        COM8 = 0x4E8
    };

    class UART
    {
    private:
        SerialPorts Port;
        bool IsAvailable;

    public:
        UART(SerialPorts Port = COMNULL);
        ~UART();
        void Write(uint8_t Char);
        uint8_t Read();
    };

    class Events
    {
    private:
        SerialPorts Port;

    protected:
        /**
         * @brief UART events.
         * @param Port if none, all ports are registered for events.
         */
        Events(SerialPorts Port = COMNULL);
        ~Events();

    public:
        /**
         * @brief Get the Registered Port object
         * @return SerialPorts
         */
        SafeFunction NIF SerialPorts GetRegisteredPort() { return this->Port; }

        /**
         * @brief Called when a character is sent.
         * @param Char the sent character.
         */

        virtual void OnSent(uint8_t Char) { UNUSED(Char); }
        /**
         * @brief Called when a character is received.
         * @param Char the received character.
         */
        virtual void OnReceived(uint8_t Char) { UNUSED(Char); }
    };

}

#endif // !__FENNIX_KERNEL_UART_H__
