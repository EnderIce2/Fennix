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
#include <base.h>

extern __driverAPI *API;

pid_t CreateKernelProcess(const char *Name)
{
	return API->CreateKernelProcess(API->MajorID,
									Name);
}

pid_t CreateKernelThread(pid_t pId, const char *Name, void *EntryPoint, void *Argument)
{
	return API->CreateKernelThread(API->MajorID,
								   pId,
								   Name,
								   EntryPoint,
								   Argument);
}

pid_t GetCurrentProcess()
{
	return API->GetCurrentProcess(API->MajorID);
}

int KillProcess(pid_t pId, int ExitCode)
{
	return API->KillProcess(API->MajorID,
							pId, ExitCode);
}

int KillThread(pid_t tId, pid_t pId, int ExitCode)
{
	return API->KillThread(API->MajorID,
						   tId, pId, ExitCode);
}

void Yield()
{
	API->Yield(API->MajorID);
}

void Sleep(uint64_t Milliseconds)
{
	API->Sleep(API->MajorID,
			   Milliseconds);
}
