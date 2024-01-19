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

#include <base.h>
#include <driver.h>

#undef memcpy
#undef memset
#undef memmove
#undef strlen

extern __driverAPI *API;

void *MemoryCopy(void *Destination, const void *Source, size_t Length)
{
	return API->memcpy(API->MajorID, Destination, Source, Length);
}

void *MemorySet(void *Destination, int Value, size_t Length)
{
	return API->memset(API->MajorID, Destination, Value, Length);
}

void *MemoryMove(void *Destination, const void *Source, size_t Length)
{
	return API->memmove(API->MajorID, Destination, Source, Length);
}

size_t StringLength(const char String[])
{
	return API->strlen(API->MajorID, String);
}

char *strstr(const char *Haystack, const char *Needle)
{
	return API->strstr(API->MajorID, Haystack, Needle);
}
