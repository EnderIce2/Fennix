/*
	This file is part of Fennix Drivers.

	Fennix Drivers is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Drivers is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Drivers. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __FENNIX_API_AIP_KBD_H__
#define __FENNIX_API_AIP_KBD_H__

#include <types.h>

#define PS2_KBD_CMD_SET_LEDS 0xED
#define PS2_KBD_CMD_ECHO 0xEE
#define PS2_KBD_CMD_SCAN_CODE_SET 0xF0
#define PS2_KBD_CMD_IDENTIFY 0xF2
#define PS2_KBD_CMD_TYPEMATIC 0xF3
#define PS2_KBD_CMD_ENABLE_SCANNING 0xF4
#define PS2_KBD_CMD_DISABLE_SCANNING 0xF5
#define PS2_KBD_CMD_DEFAULTS 0xF6
#define PS2_KBD_CMD_ALL_TYPEMATIC 0xF7
#define PS2_KBD_CMD_ALL_MAKE_RELEASE 0xF8
#define PS2_KBD_CMD_ALL_MAKE 0xF9
#define PS2_KBD_CMD_ALL_TYPEMATIC_MAKE_RELEASE 0xFA
#define PS2_KBD_CMD_SPECIFIC_TYPEMATIC 0xFB
#define PS2_KBD_CMD_SPECIFIC_MAKE_RELEASE 0xFC
#define PS2_KBD_CMD_SPECIFIC_MAKE 0xFD
#define PS2_KBD_CMD_RESEND 0xFE
#define PS2_KBD_CMD_RESET 0xFF

#define PS2_KBD_RESP_ACK 0xFA
#define PS2_KBD_RESP_ECHO 0xEE
#define PS2_KBD_RESP_RESEND 0xFE
#define PS2_KBD_RESP_TEST_PASSED 0xAA
#define PS2_KBD_RESP_TEST_FAILED 0xFC
#define PS2_KBD_RESP_TEST_FAILED_2 0xFD

typedef enum
{
	PS2_KBD_LED_SCROLL_LOCK = 1,
	PS2_KBD_LED_NUM_LOCK = 2,
	PS2_KBD_LED_CAPS_LOCK = 4
} PS2_KBD_LEDS;

typedef enum
{
	PS2_KBD_SCAN_CODE_GET_CURRENT = 0,
	PS2_KBD_SCAN_CODE_SET_1 = 1,
	PS2_KBD_SCAN_CODE_SET_2 = 2,
	PS2_KBD_SCAN_CODE_SET_3 = 3,

	PS2_KBD_SC_SET_1 = 0x43,
	PS2_KBD_SC_SET_2 = 0x41,
	PS2_KBD_SC_SET_3 = 0x3F
} PS2_KBD_SCAN_CODE_SET;

typedef union
{
	struct
	{
		/**
		 * 00000b - 30Hz
		 * 11111b - 2Hz
		 */
		uint8_t RepeatRate : 5;

		/**
		 * 00b - 250ms
		 * 01b - 500ms
		 * 10b - 750ms
		 * 11b - 1000ms
		 */
		uint8_t Delay : 2;

		/**
		 * Must be zero
		 */
		uint8_t Zero : 1;
	};
	uint8_t Raw;
} PS2_KBD_TYPEMATIC;

#endif // !__FENNIX_API_AIP_KBD_H__
