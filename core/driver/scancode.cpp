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

#include <driver.hpp>

#include "../../driver.h"

static char ScanCodeConversionTableLower[] = {
	[KEY_1] = '1',
	[KEY_2] = '2',
	[KEY_3] = '3',
	[KEY_4] = '4',
	[KEY_5] = '5',
	[KEY_6] = '6',
	[KEY_7] = '7',
	[KEY_8] = '8',
	[KEY_9] = '9',
	[KEY_0] = '0',

	[KEY_Q] = 'q',
	[KEY_W] = 'w',
	[KEY_E] = 'e',
	[KEY_R] = 'r',
	[KEY_T] = 't',
	[KEY_Y] = 'y',
	[KEY_U] = 'u',
	[KEY_I] = 'i',
	[KEY_O] = 'o',
	[KEY_P] = 'p',
	[KEY_A] = 'a',
	[KEY_S] = 's',
	[KEY_D] = 'd',
	[KEY_F] = 'f',
	[KEY_G] = 'g',
	[KEY_H] = 'h',
	[KEY_J] = 'j',
	[KEY_K] = 'k',
	[KEY_L] = 'l',
	[KEY_Z] = 'z',
	[KEY_X] = 'x',
	[KEY_C] = 'c',
	[KEY_V] = 'v',
	[KEY_B] = 'b',
	[KEY_N] = 'n',
	[KEY_M] = 'm',

	[KEY_F1] = 0x00,
	[KEY_F2] = 0x00,
	[KEY_F3] = 0x00,
	[KEY_F4] = 0x00,
	[KEY_F5] = 0x00,
	[KEY_F6] = 0x00,
	[KEY_F7] = 0x00,
	[KEY_F8] = 0x00,
	[KEY_F9] = 0x00,
	[KEY_F10] = 0x00,
	[KEY_F11] = 0x00,
	[KEY_F12] = 0x00,

	[KEYPAD_7] = '7',
	[KEYPAD_8] = '8',
	[KEYPAD_9] = '9',
	[KEYPAD_MINUS] = '-',
	[KEYPAD_4] = '4',
	[KEYPAD_5] = '5',
	[KEYPAD_6] = '6',
	[KEYPAD_PLUS] = '+',
	[KEYPAD_1] = '1',
	[KEYPAD_2] = '2',
	[KEYPAD_3] = '3',
	[KEYPAD_0] = '0',
	[KEYPAD_PERIOD] = '.',
	[KEYPAD_RETURN] = '\n',
	[KEYPAD_ASTERISK] = '*',
	[KEYPAD_SLASH] = '/',

	[KEY_LEFT_CTRL] = 0x00,
	[KEY_RIGHT_CTRL] = 0x00,
	[KEY_LEFT_SHIFT] = 0x00,
	[KEY_RIGHT_SHIFT] = 0x00,
	[KEY_LEFT_ALT] = 0x00,
	[KEY_RIGHT_ALT] = 0x00,
	[KEY_ESCAPE] = '\e',
	[KEY_MINUS] = '-',
	[KEY_EQUAL] = '=',
	[KEY_BACKSPACE] = '\b',
	[KEY_TAB] = '\t',
	[KEY_LEFT_BRACKET] = '[',
	[KEY_RIGHT_BRACKET] = ']',
	[KEY_RETURN] = '\n',
	[KEY_SEMICOLON] = ';',
	[KEY_APOSTROPHE] = '\'',
	[KEY_BACK_TICK] = '`',
	[KEY_BACKSLASH] = '\\',
	[KEY_COMMA] = ',',
	[KEY_PERIOD] = '.',
	[KEY_SLASH] = '/',
	[KEY_SPACE] = ' ',
	[KEY_CAPS_LOCK] = 0x00,
	[KEY_NUM_LOCK] = 0x00,
	[KEY_SCROLL_LOCK] = 0x00,
	[KEY_PRINT_SCREEN] = 0x00,

	[KEY_HOME] = 0x00,
	[KEY_UP_ARROW] = 0x00,
	[KEY_LEFT_ARROW] = 0x00,
	[KEY_RIGHT_ARROW] = 0x00,
	[KEY_DOWN_ARROW] = 0x00,
	[KEY_PAGE_UP] = 0x00,
	[KEY_PAGE_DOWN] = 0x00,
	[KEY_END] = 0x00,
	[KEY_INSERT] = 0x00,
	[KEY_DELETE] = 0x00,
	[KEY_LEFT_GUI] = 0x00,
	[KEY_RIGHT_GUI] = 0x00,
	[KEY_APPS] = 0x00,

	[KEY_MULTIMEDIA_PREV_TRACK] = 0x00,
	[KEY_MULTIMEDIA_NEXT_TRACK] = 0x00,
	[KEY_MULTIMEDIA_MUTE] = 0x00,
	[KEY_MULTIMEDIA_CALCULATOR] = 0x00,
	[KEY_MULTIMEDIA_PLAY] = 0x00,
	[KEY_MULTIMEDIA_STOP] = 0x00,
	[KEY_MULTIMEDIA_VOL_DOWN] = 0x00,
	[KEY_MULTIMEDIA_VOL_UP] = 0x00,
	[KEY_MULTIMEDIA_WWW_HOME] = 0x00,
	[KEY_MULTIMEDIA_WWW_SEARCH] = 0x00,
	[KEY_MULTIMEDIA_WWW_FAVORITES] = 0x00,
	[KEY_MULTIMEDIA_WWW_REFRESH] = 0x00,
	[KEY_MULTIMEDIA_WWW_STOP] = 0x00,
	[KEY_MULTIMEDIA_WWW_FORWARD] = 0x00,
	[KEY_MULTIMEDIA_WWW_BACK] = 0x00,
	[KEY_MULTIMEDIA_MY_COMPUTER] = 0x00,
	[KEY_MULTIMEDIA_EMAIL] = 0x00,
	[KEY_MULTIMEDIA_MEDIA_SELECT] = 0x00,

	[KEY_ACPI_POWER] = 0x00,
	[KEY_ACPI_SLEEP] = 0x00,
	[KEY_ACPI_WAKE] = 0x00};

static char ScanCodeConversionTableUpper[] = {
	[KEY_1] = '!',
	[KEY_2] = '@',
	[KEY_3] = '#',
	[KEY_4] = '$',
	[KEY_5] = '%',
	[KEY_6] = '^',
	[KEY_7] = '&',
	[KEY_8] = '*',
	[KEY_9] = '(',
	[KEY_0] = ')',

	[KEY_Q] = 'Q',
	[KEY_W] = 'W',
	[KEY_E] = 'E',
	[KEY_R] = 'R',
	[KEY_T] = 'T',
	[KEY_Y] = 'Y',
	[KEY_U] = 'U',
	[KEY_I] = 'I',
	[KEY_O] = 'O',
	[KEY_P] = 'P',
	[KEY_A] = 'A',
	[KEY_S] = 'S',
	[KEY_D] = 'D',
	[KEY_F] = 'F',
	[KEY_G] = 'G',
	[KEY_H] = 'H',
	[KEY_J] = 'J',
	[KEY_K] = 'K',
	[KEY_L] = 'L',
	[KEY_Z] = 'Z',
	[KEY_X] = 'X',
	[KEY_C] = 'C',
	[KEY_V] = 'V',
	[KEY_B] = 'B',
	[KEY_N] = 'N',
	[KEY_M] = 'M',

	[KEY_F1] = 0x00,
	[KEY_F2] = 0x00,
	[KEY_F3] = 0x00,
	[KEY_F4] = 0x00,
	[KEY_F5] = 0x00,
	[KEY_F6] = 0x00,
	[KEY_F7] = 0x00,
	[KEY_F8] = 0x00,
	[KEY_F9] = 0x00,
	[KEY_F10] = 0x00,
	[KEY_F11] = 0x00,
	[KEY_F12] = 0x00,

	[KEYPAD_7] = '7',
	[KEYPAD_8] = '8',
	[KEYPAD_9] = '9',
	[KEYPAD_MINUS] = '-',
	[KEYPAD_4] = '4',
	[KEYPAD_5] = '5',
	[KEYPAD_6] = '6',
	[KEYPAD_PLUS] = '+',
	[KEYPAD_1] = '1',
	[KEYPAD_2] = '2',
	[KEYPAD_3] = '3',
	[KEYPAD_0] = '0',
	[KEYPAD_PERIOD] = '.',
	[KEYPAD_RETURN] = '\n',
	[KEYPAD_ASTERISK] = '*',
	[KEYPAD_SLASH] = '/',

	[KEY_LEFT_CTRL] = 0x00,
	[KEY_RIGHT_CTRL] = 0x00,
	[KEY_LEFT_SHIFT] = 0x00,
	[KEY_RIGHT_SHIFT] = 0x00,
	[KEY_LEFT_ALT] = 0x00,
	[KEY_RIGHT_ALT] = 0x00,
	[KEY_ESCAPE] = '\e',
	[KEY_MINUS] = '_',
	[KEY_EQUAL] = '+',
	[KEY_BACKSPACE] = '\b',
	[KEY_TAB] = '\t',
	[KEY_LEFT_BRACKET] = '{',
	[KEY_RIGHT_BRACKET] = '}',
	[KEY_RETURN] = '\n',
	[KEY_SEMICOLON] = ':',
	[KEY_APOSTROPHE] = '\"',
	[KEY_BACK_TICK] = '~',
	[KEY_BACKSLASH] = '|',
	[KEY_COMMA] = '<',
	[KEY_PERIOD] = '>',
	[KEY_SLASH] = '/',
	[KEY_SPACE] = ' ',
	[KEY_CAPS_LOCK] = 0x00,
	[KEY_NUM_LOCK] = 0x00,
	[KEY_SCROLL_LOCK] = 0x00,
	[KEY_PRINT_SCREEN] = 0x00,

	[KEY_HOME] = 0x00,
	[KEY_UP_ARROW] = 0x00,
	[KEY_LEFT_ARROW] = 0x00,
	[KEY_RIGHT_ARROW] = 0x00,
	[KEY_DOWN_ARROW] = 0x00,
	[KEY_PAGE_UP] = 0x00,
	[KEY_PAGE_DOWN] = 0x00,
	[KEY_END] = 0x00,
	[KEY_INSERT] = 0x00,
	[KEY_DELETE] = 0x00,
	[KEY_LEFT_GUI] = 0x00,
	[KEY_RIGHT_GUI] = 0x00,
	[KEY_APPS] = 0x00,

	[KEY_MULTIMEDIA_PREV_TRACK] = 0x00,
	[KEY_MULTIMEDIA_NEXT_TRACK] = 0x00,
	[KEY_MULTIMEDIA_MUTE] = 0x00,
	[KEY_MULTIMEDIA_CALCULATOR] = 0x00,
	[KEY_MULTIMEDIA_PLAY] = 0x00,
	[KEY_MULTIMEDIA_STOP] = 0x00,
	[KEY_MULTIMEDIA_VOL_DOWN] = 0x00,
	[KEY_MULTIMEDIA_VOL_UP] = 0x00,
	[KEY_MULTIMEDIA_WWW_HOME] = 0x00,
	[KEY_MULTIMEDIA_WWW_SEARCH] = 0x00,
	[KEY_MULTIMEDIA_WWW_FAVORITES] = 0x00,
	[KEY_MULTIMEDIA_WWW_REFRESH] = 0x00,
	[KEY_MULTIMEDIA_WWW_STOP] = 0x00,
	[KEY_MULTIMEDIA_WWW_FORWARD] = 0x00,
	[KEY_MULTIMEDIA_WWW_BACK] = 0x00,
	[KEY_MULTIMEDIA_MY_COMPUTER] = 0x00,
	[KEY_MULTIMEDIA_EMAIL] = 0x00,
	[KEY_MULTIMEDIA_MEDIA_SELECT] = 0x00,

	[KEY_ACPI_POWER] = 0x00,
	[KEY_ACPI_SLEEP] = 0x00,
	[KEY_ACPI_WAKE] = 0x00};

#ifdef DEBUG
static const char *ScanCodeDebugNames[] = {
	"KEY_1", "KEY_2", "KEY_3", "KEY_4", "KEY_5", "KEY_6", "KEY_7", "KEY_8",
	"KEY_9", "KEY_0", "KEY_Q", "KEY_W", "KEY_E", "KEY_R", "KEY_T", "KEY_Y",
	"KEY_U", "KEY_I", "KEY_O", "KEY_P", "KEY_A", "KEY_S", "KEY_D", "KEY_F",
	"KEY_G", "KEY_H", "KEY_J", "KEY_K", "KEY_L", "KEY_Z", "KEY_X", "KEY_C",
	"KEY_V", "KEY_B", "KEY_N", "KEY_M", "KEY_F1", "KEY_F2", "KEY_F3", "KEY_F4",
	"KEY_F5", "KEY_F6", "KEY_F7", "KEY_F8", "KEY_F9", "KEY_F10", "KEY_F11",
	"KEY_F12", "KEYPAD_7", "KEYPAD_8", "KEYPAD_9", "KEYPAD_MINUS", "KEYPAD_4",
	"KEYPAD_5", "KEYPAD_6", "KEYPAD_PLUS", "KEYPAD_1", "KEYPAD_2", "KEYPAD_3",
	"KEYPAD_0", "KEYPAD_PERIOD", "KEYPAD_RETURN", "KEYPAD_ASTERISK", "KEYPAD_SLASH",
	"KEY_LEFT_CTRL", "KEY_RIGHT_CTRL", "KEY_LEFT_SHIFT", "KEY_RIGHT_SHIFT",
	"KEY_LEFT_ALT", "KEY_RIGHT_ALT", "KEY_ESCAPE", "KEY_MINUS", "KEY_EQUAL",
	"KEY_BACKSPACE", "KEY_TAB", "KEY_LEFT_BRACKET", "KEY_RIGHT_BRACKET",
	"KEY_RETURN", "KEY_SEMICOLON", "KEY_APOSTROPHE", "KEY_BACK_TICK",
	"KEY_BACKSLASH", "KEY_COMMA", "KEY_PERIOD", "KEY_SLASH", "KEY_SPACE",
	"KEY_CAPS_LOCK", "KEY_NUM_LOCK", "KEY_SCROLL_LOCK", "KEY_PRINT_SCREEN",
	"KEY_HOME", "KEY_UP_ARROW", "KEY_LEFT_ARROW", "KEY_RIGHT_ARROW",
	"KEY_DOWN_ARROW", "KEY_PAGE_UP", "KEY_PAGE_DOWN", "KEY_END", "KEY_INSERT",
	"KEY_DELETE", "KEY_LEFT_GUI", "KEY_RIGHT_GUI", "KEY_APPS",
	"KEY_MULTIMEDIA_PREV_TRACK", "KEY_MULTIMEDIA_NEXT_TRACK", "KEY_MULTIMEDIA_MUTE",
	"KEY_MULTIMEDIA_CALCULATOR", "KEY_MULTIMEDIA_PLAY", "KEY_MULTIMEDIA_STOP",
	"KEY_MULTIMEDIA_VOL_DOWN", "KEY_MULTIMEDIA_VOL_UP", "KEY_MULTIMEDIA_WWW_HOME",
	"KEY_MULTIMEDIA_WWW_SEARCH", "KEY_MULTIMEDIA_WWW_FAVORITES",
	"KEY_MULTIMEDIA_WWW_REFRESH", "KEY_MULTIMEDIA_WWW_STOP",
	"KEY_MULTIMEDIA_WWW_FORWARD", "KEY_MULTIMEDIA_WWW_BACK",
	"KEY_MULTIMEDIA_MY_COMPUTER", "KEY_MULTIMEDIA_EMAIL",
	"KEY_MULTIMEDIA_MEDIA_SELECT", "KEY_ACPI_POWER", "KEY_ACPI_SLEEP", "KEY_ACPI_WAKE"};
#endif

namespace Driver
{
	char GetScanCode(uint8_t ScanCode, bool Upper)
	{
		ScanCode &= 0x7F; /* Remove KEY_PRESSED bit */
		if (ScanCode >= sizeof(ScanCodeConversionTableLower))
		{
			warn("Unknown scancode %x", ScanCode);
			return 0x00;
		}

		// debug("Scancode %x (%s)", ScanCode, ScanCodeDebugNames[ScanCode]);
		return Upper
				   ? ScanCodeConversionTableUpper[ScanCode]
				   : ScanCodeConversionTableLower[ScanCode];
	}

	bool IsValidChar(uint8_t ScanCode)
	{
		ScanCode &= 0x7F; /* Remove KEY_PRESSED bit */
		if (ScanCode >= sizeof(ScanCodeConversionTableLower))
			return false;

		if (ScanCode > KEY_M)
		{
			if (ScanCode < KEYPAD_7)
				return false; /* F1 - F12 */

			switch (ScanCode)
			{
			case KEY_MINUS:
			case KEY_EQUAL:
			case KEY_LEFT_BRACKET:
			case KEY_RIGHT_BRACKET:
			case KEY_RETURN:
			case KEY_SEMICOLON:
			case KEY_APOSTROPHE:
			case KEY_BACK_TICK:
			case KEY_BACKSLASH:
			case KEY_COMMA:
			case KEY_PERIOD:
			case KEY_SLASH:
			case KEY_SPACE:
				return true;

			default:
				return false;
			}
		}
		return true;
	}
}
