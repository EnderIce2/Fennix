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

#ifndef __FENNIX_API_AIP_MOUSE_H__
#define __FENNIX_API_AIP_MOUSE_H__

#include <types.h>

#define PS2_MOUSE_CMD_SET_SCALING_1_1 0xE6
#define PS2_MOUSE_CMD_SET_SCALING_2_1 0xE7
#define PS2_MOUSE_CMD_SET_RESOLUTION 0xE8
#define PS2_MOUSE_CMD_GET_STATUS 0xE9
#define PS2_MOUSE_CMD_SET_STREAM_MODE 0xEA
#define PS2_MOUSE_CMD_READ_DATA 0xEB
#define PS2_MOUSE_CMD_RESET_WRAP_MODE 0xEC
#define PS2_MOUSE_CMD_SET_WRAP_MODE 0xEE
#define PS2_MOUSE_CMD_SET_REMOTE_MODE 0xF0
#define PS2_MOUSE_CMD_READ_ID 0xF2
/** Values: 10, 20, 40, 60, 80, 100, 200 */
#define PS2_MOUSE_CMD_SET_SAMPLE_RATE 0xF3
#define PS2_MOUSE_CMD_ENABLE_DATA_REPORTING 0xF4
#define PS2_MOUSE_CMD_DISABLE_DATA_REPORTING 0xF5
#define PS2_MOUSE_CMD_SET_DEFAULTS 0xF6
#define PS2_MOUSE_CMD_RESEND 0xFE
#define PS2_MOUSE_CMD_RESET 0xFF

#define PS2_MOUSE_RESP_ACK 0xFA
#define PS2_MOUSE_RESP_RESEND 0xFE
#define PS2_MOUSE_RESP_TEST_PASSED 0xAA
#define PS2_MOUSE_RESP_TEST_FAILED 0xFC

typedef enum
{
	PS2_MOUSE_RES_1 = 0,
	PS2_MOUSE_RES_2 = 1,
	PS2_MOUSE_RES_4 = 2,
	PS2_MOUSE_RES_8 = 3
} PS2_MOUSE_RESOLUTION;

typedef struct
{
	union
	{
		struct
		{
			uint8_t LeftButton : 1;
			uint8_t RightButton : 1;
			uint8_t MiddleButton : 1;
			uint8_t Always1 : 1;
			uint8_t XSign : 1;
			uint8_t YSign : 1;
			uint8_t XOverflow : 1;
			uint8_t YOverflow : 1;
		} __attribute__((packed));
		uint8_t Raw;
	} Base;

	uint8_t XMovement;
	uint8_t YMovement;

	union
	{
		struct
		{
			uint8_t Z : 4;
			uint8_t Button4 : 1;
			uint8_t Button5 : 1;
			uint8_t Always0 : 1;
			uint8_t Always0_2 : 1;
		} __attribute__((packed));
		uint8_t Raw;
	} ZMovement;
} PS2_MOUSE_PACKET;

#endif // !__FENNIX_API_AIP_MOUSE_H__
