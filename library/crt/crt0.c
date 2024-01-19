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

#include <driver.h>
#include <errno.h>

extern __driverAPI *API;

int DriverEntry();
int DriverFinal();
int DriverPanic();
int DriverProbe();
void __IdentifyDriver(
	dev_t ID,
	int (*GetDriverInfo)(dev_t, const char *, const char *,
						 const char *, const char *, const char *));

typedef void (*CallPtr)(void);
extern CallPtr __init_array_start[0], __init_array_end[0];
extern CallPtr __fini_array_start[0], __fini_array_end[0];

int _entry()
{
	for (CallPtr *func = __init_array_start; func != __init_array_end; func++)
		(*func)();

	return DriverEntry();
}

int _final()
{
	int err = DriverFinal();

	for (CallPtr *func = __fini_array_start; func != __fini_array_end; func++)
		(*func)();

	return err;
}

#define API_MajorRequiredVersion 0
#define API_MinorRequiredVersion 0
#define API_PatchRequiredVersion 0

int _start(__driverAPI *__API)
{
	if (unlikely(__API == NULL))
		return -EINVAL;

	if (unlikely(__API->APIVersion.Major != API_MajorRequiredVersion ||
				 __API->APIVersion.Minor != API_MinorRequiredVersion ||
				 __API->APIVersion.Patch != API_PatchRequiredVersion))
		return -EPROTO;

	if (unlikely(__API->RegisterFunction == NULL))
		return -ENOSYS;

	if (unlikely(DriverEntry == NULL ||
				 DriverFinal == NULL ||
				 DriverPanic == NULL ||
				 DriverProbe == NULL ||
				 __IdentifyDriver == NULL))
		return -EFAULT;

	API = __API;
	__API->RegisterFunction(__API->MajorID, _entry, _drf_Entry);
	__API->RegisterFunction(__API->MajorID, _final, _drf_Final);
	__API->RegisterFunction(__API->MajorID, DriverPanic, _drf_Panic);
	__API->RegisterFunction(__API->MajorID, DriverProbe, _drf_Probe);
	__IdentifyDriver(__API->MajorID, __API->GetDriverInfo);
	return 0;
}
