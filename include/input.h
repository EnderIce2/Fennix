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

#ifndef __FENNIX_API_INPUT_H__
#define __FENNIX_API_INPUT_H__

#include <types.h>

#if __has_include(<interface/device.h>)
#include <interface/device.h>
#else
#include <device.h>
#endif

struct InodeOperations;

typedef enum
{
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_0,

	KEY_Q,
	KEY_W,
	KEY_E,
	KEY_R,
	KEY_T,
	KEY_Y,
	KEY_U,
	KEY_I,
	KEY_O,
	KEY_P,
	KEY_A,
	KEY_S,
	KEY_D,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_Z,
	KEY_X,
	KEY_C,
	KEY_V,
	KEY_B,
	KEY_N,
	KEY_M,

	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,

	KEYPAD_7,
	KEYPAD_8,
	KEYPAD_9,
	KEYPAD_MINUS,
	KEYPAD_4,
	KEYPAD_5,
	KEYPAD_6,
	KEYPAD_PLUS,
	KEYPAD_1,
	KEYPAD_2,
	KEYPAD_3,
	KEYPAD_0,
	KEYPAD_PERIOD,
	KEYPAD_RETURN,
	KEYPAD_ASTERISK,
	KEYPAD_SLASH,

	KEY_LEFT_CTRL,
	KEY_RIGHT_CTRL,
	KEY_LEFT_SHIFT,
	KEY_RIGHT_SHIFT,
	KEY_LEFT_ALT,
	KEY_RIGHT_ALT,
	KEY_ESCAPE,
	KEY_MINUS,
	KEY_EQUAL,
	KEY_BACKSPACE,
	KEY_TAB,
	KEY_LEFT_BRACKET,
	KEY_RIGHT_BRACKET,
	KEY_RETURN,
	KEY_SEMICOLON,
	KEY_APOSTROPHE,
	KEY_BACK_TICK,
	KEY_BACKSLASH,
	KEY_COMMA,
	KEY_PERIOD,
	KEY_SLASH,
	KEY_SPACE,
	KEY_CAPS_LOCK,
	KEY_NUM_LOCK,
	KEY_SCROLL_LOCK,
	KEY_PRINT_SCREEN,

	KEY_HOME,
	KEY_UP_ARROW,
	KEY_LEFT_ARROW,
	KEY_RIGHT_ARROW,
	KEY_DOWN_ARROW,
	KEY_PAGE_UP,
	KEY_PAGE_DOWN,
	KEY_END,
	KEY_INSERT,
	KEY_DELETE,
	KEY_LEFT_GUI,
	KEY_RIGHT_GUI,
	KEY_APPS,

	KEY_MULTIMEDIA_PREV_TRACK,
	KEY_MULTIMEDIA_NEXT_TRACK,
	KEY_MULTIMEDIA_MUTE,
	KEY_MULTIMEDIA_CALCULATOR,
	KEY_MULTIMEDIA_PLAY,
	KEY_MULTIMEDIA_STOP,
	KEY_MULTIMEDIA_VOL_DOWN,
	KEY_MULTIMEDIA_VOL_UP,
	KEY_MULTIMEDIA_WWW_HOME,
	KEY_MULTIMEDIA_WWW_SEARCH,
	KEY_MULTIMEDIA_WWW_FAVORITES,
	KEY_MULTIMEDIA_WWW_REFRESH,
	KEY_MULTIMEDIA_WWW_STOP,
	KEY_MULTIMEDIA_WWW_FORWARD,
	KEY_MULTIMEDIA_WWW_BACK,
	KEY_MULTIMEDIA_MY_COMPUTER,
	KEY_MULTIMEDIA_EMAIL,
	KEY_MULTIMEDIA_MEDIA_SELECT,

	KEY_ACPI_POWER,
	KEY_ACPI_SLEEP,
	KEY_ACPI_WAKE,

	KEY_PRESSED = 0x80,
} KeyScanCodes;

typedef struct
{
	KeyScanCodes Key;

	union
	{
		struct
		{
			char IsScanCode : 1;
		};
		char Value;
	};
	unsigned char Character;
} KeyboardReport;

typedef struct
{
	long X, Y;
	int8_t Z;
	uint8_t Absolute : 1;
	uint8_t LeftButton : 1;
	uint8_t RightButton : 1;
	uint8_t MiddleButton : 1;
	uint8_t Button4 : 1;
	uint8_t Button5 : 1;
	uint8_t Button6 : 1;
	uint8_t Button7 : 1;
	uint8_t Button8 : 1;
} MouseReport;

typedef struct
{
} JoystickReport;

typedef struct
{
	uint16_t X, Y;
	uint8_t Pressure;
} TouchScreenReport;

typedef struct
{
} GamepadReport;

typedef struct
{
} AccelerometerReport;

typedef struct
{
} GyroscopeReport;

typedef struct
{
} MagnetometerReport;

typedef struct
{
	DeviceType Type;
	dev_t Device;
	union
	{
		KeyboardReport Keyboard;
		MouseReport Mouse;
		JoystickReport Joystick;
		TouchScreenReport TouchScreen;
		GamepadReport Gamepad;
		AccelerometerReport Accelerometer;
		GyroscopeReport Gyroscope;
		MagnetometerReport Magnetometer;
		/* ... */
	};
} InputReport;

EXTERNC int ReportInputEvent(InputReport *Report);

#endif // !__FENNIX_API_INPUT_H__
