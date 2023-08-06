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

using namespace VirtualFileSystem;

void cmd_uptime(const char *)
{
	if (TimeManager)
	{
		size_t Nanoseconds =
			TimeManager->GetNanosecondsSinceClassCreation();
		size_t Seconds = Nanoseconds / 10000000;
		size_t Minutes = Seconds / 60;
		size_t Hours = Minutes / 60;
		size_t Days = Hours / 24;

		debug("Nanoseconds: %ld", Nanoseconds);

		Seconds %= 60;
		Minutes %= 60;
		Hours %= 24;

		printf("%ld days, %ld hours, %ld minutes, %ld %s\n",
			   Days, Hours, Minutes, Seconds,
			   Seconds == 1 ? "second" : "seconds");
	}
	else
	{
		printf("Could not get uptime\n");
	}
}
