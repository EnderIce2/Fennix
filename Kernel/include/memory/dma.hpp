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

#include <memory/physical.hpp>
#include <memory/virtual.hpp>

namespace Memory
{
	class DMA
	{
	private:
	public:
		/**
		 * Allocate a DMA buffer of the specified size and alignment.
		 *
		 * @param Size Size of the buffer in bytes
		 * @param Alignment Alignment of the buffer in bytes (must be a power of 2)
		 * @param Boundary Boundary for the buffer (must be a power of 2)
		 * @param MaxAddress Maximum physical address (optional, 0 = no limit)
		 * @return Physical address of the allocated buffer, or nullptr on failure
		 */
		fnx::void_t Allocate(size_t Size, size_t Alignment, size_t Boundary, size_t MaxAddress = 0);

		DMA() = default;
		~DMA() = default;
	};
}
