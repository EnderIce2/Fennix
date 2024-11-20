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

#include "aip.h"

#include <driver.h>
#include <errno.h>
#include <fs.h>
#include <input.h>
#include <base.h>
#include <io.h>

uint8_t KeyboardScanCodeSet = 0;
dev_t KeyboardDevID = -1;

const unsigned short ScanCodeSet1[] =
	{KEY_NULL, KEY_ESCAPE,
	 KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
	 KEY_MINUS, KEY_EQUAL, KEY_BACKSPACE, KEY_TAB,
	 KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P,
	 KEY_LEFT_BRACKET, KEY_RIGHT_BRACKET, KEY_RETURN, KEY_LEFT_CTRL,
	 KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L,
	 KEY_SEMICOLON, KEY_APOSTROPHE, KEY_BACK_TICK, KEY_LEFT_SHIFT, KEY_BACKSLASH,
	 KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M,
	 KEY_COMMA, KEY_PERIOD, KEY_SLASH, KEY_RIGHT_SHIFT,
	 KEYPAD_ASTERISK, KEY_LEFT_ALT, KEY_SPACE, KEY_CAPS_LOCK,
	 KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,
	 KEY_NUM_LOCK, KEY_SCROLL_LOCK,
	 KEYPAD_7, KEYPAD_8, KEYPAD_9, KEYPAD_MINUS,
	 KEYPAD_4, KEYPAD_5, KEYPAD_6, KEYPAD_PLUS,
	 KEYPAD_1, KEYPAD_2, KEYPAD_3, KEYPAD_0, KEYPAD_PERIOD,
	 KEY_NULL, KEY_NULL, KEY_NULL,
	 KEY_F11, KEY_F12};

const unsigned short ScanCodeSet1mm[] = {
	[0x10] = KEY_MULTIMEDIA_PREV_TRACK,
	[0x19] = KEY_MULTIMEDIA_NEXT_TRACK,
	[0x1C] = KEYPAD_RETURN,
	[0x1D] = KEY_RIGHT_CTRL,
	[0x20] = KEY_MULTIMEDIA_MUTE,
	[0x21] = KEY_MULTIMEDIA_CALCULATOR,
	[0x22] = KEY_MULTIMEDIA_PLAY,
	[0x24] = KEY_MULTIMEDIA_STOP,
	[0x2A] = KEY_PRINT_SCREEN,
	[0x2E] = KEY_MULTIMEDIA_VOL_DOWN,
	[0x30] = KEY_MULTIMEDIA_VOL_UP,
	[0x32] = KEY_MULTIMEDIA_WWW_HOME,
	[0x35] = KEYPAD_SLASH,
	[0x37] = KEY_PRINT_SCREEN,
	[0x38] = KEY_RIGHT_ALT,
	[0x47] = KEY_HOME,
	[0x48] = KEY_UP_ARROW,
	[0x49] = KEY_PAGE_UP,
	[0x4B] = KEY_LEFT_ARROW,
	[0x4D] = KEY_RIGHT_ARROW,
	[0x4F] = KEY_END,
	[0x50] = KEY_DOWN_ARROW,
	[0x51] = KEY_PAGE_DOWN,
	[0x52] = KEY_INSERT,
	[0x53] = KEY_DELETE,
	[0x5B] = KEY_LEFT_GUI,
	[0x5C] = KEY_RIGHT_GUI,
	[0x5D] = KEY_APPS,
	[0x5E] = KEY_ACPI_POWER,
	[0x5F] = KEY_ACPI_SLEEP,
	[0x63] = KEY_ACPI_WAKE,
	[0x65] = KEY_MULTIMEDIA_WWW_SEARCH,
	[0x66] = KEY_MULTIMEDIA_WWW_FAVORITES,
	[0x67] = KEY_MULTIMEDIA_WWW_REFRESH,
	[0x68] = KEY_MULTIMEDIA_WWW_STOP,
	[0x69] = KEY_MULTIMEDIA_WWW_FORWARD,
	[0x6A] = KEY_MULTIMEDIA_WWW_BACK,
	[0x6B] = KEY_MULTIMEDIA_MY_COMPUTER,
	[0x6C] = KEY_MULTIMEDIA_EMAIL,
	[0x6D] = KEY_MULTIMEDIA_MEDIA_SELECT,

	/* RELEASED */

	[0x90] = KEY_MULTIMEDIA_PREV_TRACK,
	[0x99] = KEY_MULTIMEDIA_NEXT_TRACK,
	[0x9C] = KEYPAD_RETURN,
	[0x9D] = KEY_RIGHT_CTRL,
	[0xA0] = KEY_MULTIMEDIA_MUTE,
	[0xA1] = KEY_MULTIMEDIA_CALCULATOR,
	[0xA2] = KEY_MULTIMEDIA_PLAY,
	[0xA4] = KEY_MULTIMEDIA_STOP,
	[0xAA] = KEY_PRINT_SCREEN,
	[0xAE] = KEY_MULTIMEDIA_VOL_DOWN,
	[0xB0] = KEY_MULTIMEDIA_VOL_UP,
	[0xB2] = KEY_MULTIMEDIA_WWW_HOME,
	[0xB5] = KEYPAD_SLASH,
	[0xB7] = KEY_PRINT_SCREEN,
	[0xB8] = KEY_RIGHT_ALT,
	[0xC7] = KEY_HOME,
	[0xC8] = KEY_UP_ARROW,
	[0xC9] = KEY_PAGE_UP,
	[0xCB] = KEY_LEFT_ARROW,
	[0xCD] = KEY_RIGHT_ARROW,
	[0xCF] = KEY_END,
	[0xD0] = KEY_DOWN_ARROW,
	[0xD1] = KEY_PAGE_DOWN,
	[0xD2] = KEY_INSERT,
	[0xD3] = KEY_DELETE,
	[0xDB] = KEY_LEFT_GUI,
	[0xDC] = KEY_RIGHT_GUI,
	[0xDD] = KEY_APPS,
	[0xDE] = KEY_ACPI_POWER,
	[0xDF] = KEY_ACPI_SLEEP,
	[0xE3] = KEY_ACPI_WAKE,
	[0xE5] = KEY_MULTIMEDIA_WWW_SEARCH,
	[0xE6] = KEY_MULTIMEDIA_WWW_FAVORITES,
	[0xE7] = KEY_MULTIMEDIA_WWW_REFRESH,
	[0xE8] = KEY_MULTIMEDIA_WWW_STOP,
	[0xE9] = KEY_MULTIMEDIA_WWW_FORWARD,
	[0xEA] = KEY_MULTIMEDIA_WWW_BACK,
	[0xEB] = KEY_MULTIMEDIA_MY_COMPUTER,
	[0xEC] = KEY_MULTIMEDIA_EMAIL,
	[0xED] = KEY_MULTIMEDIA_MEDIA_SELECT};

const unsigned short ScanCodeSet3[] = {
	[0x15] = KEY_Q,
	[0x1A] = KEY_Z,
	[0x1B] = KEY_S,
	[0x1C] = KEY_A,
	[0x1D] = KEY_W,

	[0x21] = KEY_C,
	[0x22] = KEY_X,
	[0x23] = KEY_D,
	[0x24] = KEY_E,
	[0x2A] = KEY_V,
	[0x2B] = KEY_F,
	[0x2C] = KEY_T,
	[0x2D] = KEY_R,

	[0x31] = KEY_N,
	[0x32] = KEY_B,
	[0x33] = KEY_H,
	[0x34] = KEY_G,
	[0x35] = KEY_Y,
	[0x3A] = KEY_M,
	[0x3B] = KEY_J,
	[0x3C] = KEY_U,

	[0x42] = KEY_K,
	[0x43] = KEY_I,
	[0x44] = KEY_O,
	[0x4B] = KEY_L,
	[0x4D] = KEY_P};

InputReport kir = {0};
int ReportKeyboardEvent(dev_t Device, KeyScanCodes ScanCode, uint8_t Pressed)
{
	kir.Type = INPUT_TYPE_KEYBOARD;
	kir.Device = Device;
	kir.Keyboard.Key = ScanCode;
	kir.Keyboard.Key |= Pressed ? KEY_PRESSED : 0;
	ReportInputEvent(&kir);
	return 0;
}

bool IsE0 = false;
bool IsE1 = false;
void PS2KbdInterruptHandler(TrapFrame *)
{
	uint8_t sc = inb(PS2_DATA);
	if (sc == PS2_KBD_RESP_ACK ||
		sc == PS2_KBD_RESP_ECHO ||
		sc == PS2_KBD_RESP_RESEND)
		return;

	if (sc == 0xE0)
	{
		IsE0 = true;
		return;
	}

	if (sc == 0xE1)
	{
		IsE1 = true;
		return;
	}

	switch (KeyboardScanCodeSet)
	{
	case PS2_KBD_SC_SET_1:
	case PS2_KBD_SC_SET_2:
	{
		if (IsE0)
		{
			IsE0 = false;
			ReportKeyboardEvent(KeyboardDevID, ScanCodeSet1mm[sc], sc < 0x90);
			return;
		}
		else
		{
			bool released = sc & 0x80;
			uint8_t scFinal = released ? sc & 0x7F : sc;
			ReportKeyboardEvent(KeyboardDevID, ScanCodeSet1[scFinal], !released);
			return;
		}
	}
	/* FIXME: https://wiki.osdev.org/PS/2_Keyboard */
	// case PS2_KBD_SC_SET_2:
	// {
	// 	break;
	// }
	case PS2_KBD_SC_SET_3:
	{
		ReportKeyboardEvent(KeyboardDevID, ScanCodeSet3[sc], true);
		ReportKeyboardEvent(KeyboardDevID, ScanCodeSet3[sc], false);
		break;
	}
	default:
	{
		if (IsE0)
			IsE0 = false;
		KernelLog("Unknown PS/2 Keyboard Scan Code Set: %#x", KeyboardScanCodeSet);
		break;
	}
	}
}

int __fs_kb_Ioctl(struct Inode *, unsigned long, void *)
{
	return 0;
}

const struct InodeOperations KbdOps = {
	.Ioctl = __fs_kb_Ioctl,
};

int InitializeKeyboard()
{
	// PS2WriteData(PS2_KBD_CMD_RESET);
	// uint8_t test = PS2ReadData();
	// if (test != PS2_KBD_RESP_TEST_PASSED &&
	// 	test != PS2_KBD_RESP_ACK)
	// {
	// 	KernelLog("PS/2 keyboard reset failed (%#x)", test);
	// 	return -EFAULT;
	// }

	PS2WriteData(PS2_KBD_CMD_DEFAULTS);
	if (PS2ACKTimeout() != 0)
		KernelLog("PS/2 keyboard failed to set defaults");

	PS2WriteData(PS2_KBD_CMD_SCAN_CODE_SET);
	if (PS2ACKTimeout() != 0)
		KernelLog("PS/2 keyboard failed to set scan code set");

	/* We want Scan Code Set 1 */
	PS2WriteData(PS2_KBD_SCAN_CODE_SET_2); /* It will set to 1 but with translation? */
	if (PS2ACKTimeout() != 0)
		KernelLog("PS/2 keyboard failed to set scan code set 2");

	PS2WriteData(PS2_KBD_CMD_SCAN_CODE_SET);
	if (PS2ACKTimeout() != 0)
		KernelLog("PS/2 keyboard failed to set scan code set");

	PS2WriteData(PS2_KBD_SCAN_CODE_GET_CURRENT);
	if (PS2ACKTimeout() != 0)
		KernelLog("PS/2 keyboard failed to get current scan code set");

	KeyboardScanCodeSet = PS2ReadAfterACK();
	KernelLog("PS/2 Keyboard Scan Code Set: 0x%X", KeyboardScanCodeSet);
	PS2ClearOutputBuffer();

	PS2WriteData(PS2_KBD_CMD_ENABLE_SCANNING);

	RegisterInterruptHandler(1, PS2KbdInterruptHandler);

	KeyboardDevID = RegisterDevice(INPUT_TYPE_KEYBOARD, &KbdOps);
	return 0;
}

int FinalizeKeyboard()
{
	PS2WriteData(PS2_KBD_CMD_DISABLE_SCANNING);
	if (PS2ACKTimeout() != 0)
		KernelLog("PS/2 keyboard failed to disable scanning");

	UnregisterDevice(KeyboardDevID);
	return 0;
}

int DetectPS2Keyboard()
{
	PS2WriteData(PS2_KBD_CMD_DISABLE_SCANNING);
	if (PS2ACKTimeout() != 0)
		KernelLog("PS/2 keyboard failed to disable scanning");

	PS2WriteData(PS2_KBD_CMD_IDENTIFY);
	if (PS2ACKTimeout() != 0)
		KernelLog("PS/2 keyboard failed to identify");

	uint8_t recByte;
	int timeout = 1000000;
	while (timeout--)
	{
		recByte = PS2ReadData();
		if (recByte != PS2_ACK)
			break;
	}
	Device1ID[0] = recByte;

	timeout = 1000000;
	while (timeout--)
	{
		recByte = PS2ReadData();
		if (recByte != PS2_ACK)
			break;
	}
	if (timeout == 0)
		KernelLog("PS/2 keyboard second byte timed out");
	else
		Device1ID[1] = recByte;

	KernelLog("PS2 Keyboard Device: 0x%X 0x%X", Device1ID[0], Device1ID[1]);
	return 0;
}
