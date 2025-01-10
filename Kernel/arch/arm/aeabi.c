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

unsigned __aeabi_uidiv(unsigned a, unsigned b) { return a / b; }
unsigned __aeabi_idiv(unsigned a, unsigned b) { return a / b; }
unsigned __aeabi_l2d(unsigned a) { return a; }
unsigned __aeabi_d2lz(unsigned a) { return a; }

unsigned __aeabi_ldivmod(unsigned a, unsigned b, unsigned *r) { *r = a % b; return a / b; }

unsigned __aeabi_uldivmod(unsigned a, unsigned b, unsigned *r) { *r = a % b; return a / b; }

unsigned __aeabi_uidivmod(unsigned a, unsigned b, unsigned *r) { *r = a % b; return a / b; }


unsigned __aeabi_idivmod(unsigned a, unsigned b, unsigned *r) { *r = a % b; return a / b; }


unsigned __aeabi_unwind_cpp_pr1(void) { return 0; }
unsigned __aeabi_atexit(void) { return 0; }
unsigned __cxa_end_cleanup(void) { return 0; }
unsigned __aeabi_unwind_cpp_pr0(void) { return 0; }




