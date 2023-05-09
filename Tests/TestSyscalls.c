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

#include <types.h>

#include "../syscalls.h"

#ifdef DEBUG

__aligned(0x1000) __no_stack_protector void TestSyscalls()
{
#if defined(a64)
   __asm__ __volatile__("syscall"
                        :
                        : "a"(_Print), "D"('H'), "S"(0)
                        : "rcx", "r11", "memory");

   int fork_id = -0xda;

   __asm__ __volatile__("syscall"
                        : "=a"(fork_id)
                        : "a"(_Fork)
                        : "rcx", "r11", "memory");

   __asm__ __volatile__("syscall"
                        :
                        : "a"(_Exit), "D"(fork_id)
                        : "rcx", "r11", "memory");
#elif defined(a32)
#elif defined(aa64)
#endif
   while (1)
      ;
}

#endif
