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

#include "../cmds.hpp"

#include <filesystem.hpp>

#include "../../kernel.h"

using namespace vfs;

void cmd_uptime(const char *)
{
	if (TimeManager)
	{
		uint64_t Nanoseconds =
			TimeManager->GetNanosecondsSinceClassCreation();
		uint64_t Seconds = Nanoseconds / 10000000;
		uint64_t Minutes = Seconds / 60;
		uint64_t Hours = Minutes / 60;
		uint64_t Days = Hours / 24;

		debug("Nanoseconds: %ld", Nanoseconds);

		Seconds %= 60;
		Minutes %= 60;
		Hours %= 24;

#if defined(__amd64__)
		printf("%ld days, %ld hours, %ld minutes, %ld %s\n",
			   Days, Hours, Minutes, Seconds,
			   Seconds == 1 ? "second" : "seconds");
#elif defined(__i386__)
		printf("%lld days, %lld hours, %lld minutes, %lld %s\n",
			   Days, Hours, Minutes, Seconds,
			   Seconds == 1 ? "second" : "seconds");
#endif
	}
	else
	{
		printf("Could not get uptime\n");
	}
}
