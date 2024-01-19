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

#ifndef __FENNIX_API_DRIVER_FUNCTIONS_H__
#define __FENNIX_API_DRIVER_FUNCTIONS_H__

#include <types.h>

typedef enum
{
	_drf_Entry,
	_drf_Final,
	_drf_Panic,
	_drf_Probe,
} __driverRegFunc;

typedef union
{
	struct
	{
		uint8_t LeftButton : 1;
		uint8_t RightButton : 1;
		uint8_t MiddleButton : 1;
		uint8_t Button4 : 1;
		uint8_t Button5 : 1;
		uint8_t Button6 : 1;
		uint8_t Button7 : 1;
		uint8_t Button8 : 1;
	};
	uint8_t Value;
} __MouseButtons;

typedef struct
{
	/* PCIDevice */ void *Device;
	/* __PCIArray */ void *Next;
} __PCIArray;

/* ========================================== */

#define PCI_END 0x0000
#define KEY_NULL 0x00

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

typedef enum
{
	ddt_Keyboard,
	ddt_Mouse,
	ddt_Joystick,
	ddt_Gamepad,
	ddt_Touchpad,
	ddt_Touchscreen,

	ddt_SATA,
	ddt_ATA,
	ddt_NVMe,

	ddt_Audio,

	ddt_Network,
} DeviceDriverType;

typedef enum
{
	IOCTL_AUDIO_GET_VOLUME,
	IOCTL_AUDIO_SET_VOLUME,

	IOCTL_AUDIO_GET_MUTE,
	IOCTL_AUDIO_SET_MUTE,

	IOCTL_AUDIO_GET_SAMPLE_RATE,
	IOCTL_AUDIO_SET_SAMPLE_RATE,

	IOCTL_AUDIO_GET_CHANNELS,
	IOCTL_AUDIO_SET_CHANNELS,
} AudioIoctl;

typedef enum
{
	IOCTL_NET_GET_MAC,
} NetIoctl;

typedef enum
{
	MAP_PRESENT = 1 << 0,
	MAP_WRITE = 1 << 1,
	MAP_USER = 1 << 2,
	MAP_WRITE_THROUGH = 1 << 3,
	MAP_CACHE_DISABLE = 1 << 4,
} PageMapFlags;

typedef struct
{
	struct
	{
		uint8_t Major;
		uint8_t Minor;
		uint8_t Patch;
	} APIVersion;

	dev_t MajorID;
	uintptr_t Base;

	/* Internal */
	int (*RegisterFunction)(dev_t MajorID, void *Function, __driverRegFunc Type);
	int (*GetDriverInfo)(dev_t MajorID, const char *Name, const char *Description, const char *Author, const char *Version, const char *License);

	/* Interrupts */
	int (*RegisterInterruptHandler)(dev_t MajorID, uint8_t IRQ, void *Handler);
	int (*OverrideInterruptHandler)(dev_t MajorID, uint8_t IRQ, void *Handler);
	int (*UnregisterInterruptHandler)(dev_t MajorID, uint8_t IRQ, void *Handler);
	int (*UnregisterAllInterruptHandlers)(dev_t MajorID, void *Handler);

	/* Input */
	dev_t (*RegisterInputDevice)(dev_t MajorID, DeviceDriverType Type);
	int (*UnregisterInputDevice)(dev_t MajorID, dev_t MinorID, DeviceDriverType Type);
	int (*ReportKeyboardEvent)(dev_t MajorID, dev_t MinorID, uint8_t ScanCode);
	int (*ReportRelativeMouseEvent)(dev_t MajorID, dev_t MinorID, __MouseButtons Button, int X, int Y, int8_t Z);
	int (*ReportAbsoluteMouseEvent)(dev_t MajorID, dev_t MinorID, __MouseButtons Button, uintptr_t X, uintptr_t Y, int8_t Z);

	/* Storage */
	dev_t (*RegisterBlockDevice)(dev_t MajorID, DeviceDriverType Type, void *Open, void *Close, void *Read, void *Write, void *Ioctl);
	int (*UnregisterBlockDevice)(dev_t MajorID, dev_t MinorID, DeviceDriverType Type);

	/* Audio */
	dev_t (*RegisterAudioDevice)(dev_t MajorID, DeviceDriverType Type, void *Open, void *Close, void *Read, void *Write, void *Ioctl);
	int (*UnregisterAudioDevice)(dev_t MajorID, dev_t MinorID, DeviceDriverType Type);

	/* Network */
	dev_t (*RegisterNetDevice)(dev_t MajorID, DeviceDriverType Type, void *Open, void *Close, void *Read, void *Write, void *Ioctl);
	int (*UnregisterNetDevice)(dev_t MajorID, dev_t MinorID, DeviceDriverType Type);
	int (*ReportNetworkPacket)(dev_t MajorID, dev_t MinorID, void *Buffer, size_t Size);

	/* Logging */
	void (*KPrint)(dev_t MajorID, const char *Format, va_list args);
	void (*KernelLog)(dev_t MajorID, const char *Format, va_list args);

	/* Memory */
	void *(*RequestPages)(dev_t MajorID, size_t Pages);
	void (*FreePages)(dev_t MajorID, void *Pointer, size_t Pages);

	/* Mapping */
	void (*AppendMapFlag)(dev_t MajorID, void *Address, PageMapFlags Flag);
	void (*RemoveMapFlag)(dev_t MajorID, void *Address, PageMapFlags Flag);
	void (*MapPages)(dev_t MajorID, void *PhysicalAddress, void *VirtualAddress, size_t Pages, uint32_t Flags);
	void (*UnmapPages)(dev_t MajorID, void *VirtualAddress, size_t Pages);

	/* Scheduling */
	pid_t (*CreateKernelProcess)(dev_t MajorID, const char *Name);
	pid_t (*CreateKernelThread)(dev_t MajorID, pid_t pId, const char *Name, void *EntryPoint, void *Argument);
	int (*KillProcess)(dev_t MajorID, pid_t pId, int ExitCode);
	int (*KillThread)(dev_t MajorID, pid_t tId, int ExitCode);
	void (*Yield)(dev_t MajorID);
	void (*Sleep)(dev_t MajorID, uint64_t Milliseconds);

	/* PCI */
	__PCIArray *(*GetPCIDevices)(dev_t MajorID, uint16_t Vendors[], uint16_t Devices[]);
	void (*InitializePCI)(dev_t MajorID, void *Header);
	uint32_t (*GetBAR)(dev_t MajorID, uint8_t Index, void *Header);

	/* Kernel std API */
	void *(*memcpy)(dev_t MajorID, void *Destination, const void *Source, size_t Length);
	void *(*memset)(dev_t MajorID, void *Destination, int Value, size_t Length);
	void *(*memmove)(dev_t MajorID, void *Destination, const void *Source, size_t Length);
	int (*memcmp)(dev_t MajorID, const void *Left, const void *Right, size_t Length);
	size_t (*strlen)(dev_t MajorID, const char *String);
	char *(*strcpy)(dev_t MajorID, char *Destination, const char *Source);
	char *(*strcat)(dev_t MajorID, char *Destination, const char *Source);
	int (*strcmp)(dev_t MajorID, const char *Left, const char *Right);
	int (*strncmp)(dev_t MajorID, const char *Left, const char *Right, size_t Length);
	char *(*strchr)(dev_t MajorID, const char *String, int Character);
	char *(*strrchr)(dev_t MajorID, const char *String, int Character);
	char *(*strstr)(dev_t MajorID, const char *Haystack, const char *Needle);
} __driverAPI;

#endif // !__FENNIX_API_DRIVER_FUNCTIONS_H__
