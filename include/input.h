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

#ifndef __FENNIX_API_INPUT_H__
#define __FENNIX_API_INPUT_H__

#include <types.h>
#include <driver.h>

typedef struct
{
	union
	{
		struct
		{
			uint8_t LeftButton : 1;
			uint8_t RightButton : 1;
			uint8_t MiddleButton : 1;
			uint8_t Button4 : 1;
			uint8_t Button5 : 1;
		};
		uint8_t Buttons;
	};

	/**
	 * @note Ignored if is absolute
	 */
	int X;

	/**
	 * @note Ignored if is absolute
	 */
	int Y;
	int8_t Z;
} MouseReport;

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

	dev_t RegisterInputDevice(DeviceDriverType Type);
	int UnregisterInputDevice(dev_t DeviceID, DeviceDriverType Type);
	int ReportKeyboardEvent(dev_t DeviceID, KeyScanCodes ScanCode, uint8_t Pressed);
	int ReportRelativeMouseEvent(dev_t DeviceID, MouseReport Report);
	int ReportAbsoluteMouseEvent(dev_t DeviceID, MouseReport Report, uintptr_t X, uintptr_t Y);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !__FENNIX_API_INPUT_H__
