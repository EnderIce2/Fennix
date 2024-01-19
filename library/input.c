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

#include <input.h>

extern __driverAPI *API;

dev_t RegisterInputDevice(DeviceDriverType Type)
{
	return API->RegisterInputDevice(API->MajorID, Type);
}

int UnregisterInputDevice(dev_t DeviceID, DeviceDriverType Type)
{
	return API->UnregisterInputDevice(API->MajorID, DeviceID, Type);
}

int ReportKeyboardEvent(dev_t DeviceID, KeyScanCodes ScanCode, uint8_t Pressed)
{
	if (Pressed)
		ScanCode |= KEY_PRESSED;
	return API->ReportKeyboardEvent(API->MajorID, DeviceID, ScanCode);
}

int ReportRelativeMouseEvent(dev_t DeviceID, MouseReport Report)
{
	__MouseButtons mb = {
		.LeftButton = Report.LeftButton,
		.RightButton = Report.RightButton,
		.MiddleButton = Report.MiddleButton,
		.Button4 = Report.Button4,
		.Button5 = Report.Button5,
	};
	return API->ReportRelativeMouseEvent(API->MajorID, DeviceID, mb, Report.X, Report.Y, Report.Z);
}

int ReportAbsoluteMouseEvent(dev_t DeviceID, MouseReport Report, uintptr_t X, uintptr_t Y)
{
	__MouseButtons mb = {
		.LeftButton = Report.LeftButton,
		.RightButton = Report.RightButton,
		.MiddleButton = Report.MiddleButton,
		.Button4 = Report.Button4,
		.Button5 = Report.Button5,
	};
	return API->ReportAbsoluteMouseEvent(API->MajorID, DeviceID, mb, X, Y, Report.Z);
}
