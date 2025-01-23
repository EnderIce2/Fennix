/*
	This file is part of Lynx Bootloader.

	Lynx Bootloader is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Lynx Bootloader is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Lynx Bootloader. If not, see <https://www.gnu.org/licenses/>.
*/

#include <types.h>

uintptr_t __stack_chk_guard = 0;

__noreturn __no_stack_protector void __stack_chk_fail(void)
{
	while (1)
		;
}

int main();
__attribute__((section(".bootstrap.text"))) void _aarch64_start(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
{
	main();
}
