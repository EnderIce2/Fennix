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

#include <display.hpp>
#include <lock.hpp>
#include <uart.hpp>
#include <debug.h>

NewLock(PrintLock);

namespace Video
{
	uint16_t Display::GetBitsPerPixel() { return this->framebuffer.BitsPerPixel; }
	size_t Display::GetPitch() { return this->framebuffer.Pitch; }

	void Display::ClearBuffer()
	{
		memset(this->Buffer, 0, this->Size);
		// std::fill(this->DirtyMap.begin(), this->DirtyMap.end(), true);
	}

	__no_sanitize("undefined") void Display::SetPixel(uint32_t X,
													  uint32_t Y,
													  uint32_t Color)
	{
		if (unlikely(X >= this->Width))
			X = this->Width - 1;

		if (unlikely(Y >= this->Height))
			Y = this->Height - 1;

		uint32_t *Pixel = (uint32_t *)((uintptr_t)this->Buffer + (Y * this->Width + X) * (this->framebuffer.BitsPerPixel / 8));
		*Pixel = Color;
		// MarkRegionDirty(Y / RegionHeight, X / RegionWidth);
	}

	__no_sanitize("undefined") uint32_t Display::GetPixel(uint32_t X,
														  uint32_t Y)
	{
		if (unlikely(X >= this->Width))
			X = this->Width - 1;

		if (unlikely(Y >= this->Height))
			Y = this->Height - 1;

		uint32_t *Pixel = (uint32_t *)((uintptr_t)this->Buffer + (Y * this->Width + X) * (this->framebuffer.BitsPerPixel / 8));
		return *Pixel;
	}

	__no_sanitize("undefined") void Display::DrawRectangle(uint32_t X,
														   uint32_t Y,
														   uint32_t Width,
														   uint32_t Height,
														   uint32_t Color)
	{
		for (uint32_t i = 0; i < Width; i++)
		{
			for (uint32_t j = 0; j < Height; j++)
			{
				uint32_t *Pixel =
					(uint32_t *)((uintptr_t)this->Buffer + ((Y + j) *
																this->Width +
															(X + i)) *
															   (this->framebuffer.BitsPerPixel / 8));
				*Pixel = Color;
			}
		}
	}

	void Display::UpdateBuffer()
	{
		if (!DirectWrite)
			memcpy(this->framebuffer.BaseAddress, this->Buffer, this->Size);

		// for (size_t i = 0; i < DirtyMap.size(); ++i)
		// {
		// 	if (DirtyMap[i])
		// 	{
		// 		size_t rRow = i / (framebuffer.Width / RegionWidth);
		// 		size_t rCol = i % (framebuffer.Width / RegionWidth);
		// 		UpdateRegion(rRow, rCol);
		// 		DirtyMap[i] = false;
		// 	}
		// }
	}

	void Display::UpdateRegion(size_t RegionRow, size_t RegionColumn)
	{
		size_t startRow = RegionRow * RegionHeight;
		size_t endRow = startRow + RegionHeight;
		size_t startCol = RegionColumn * RegionWidth;
		size_t endCol = startCol + RegionWidth;

		uint8_t *framebufferPtr = (uint8_t *)framebuffer.BaseAddress;
		uint8_t *bufferPtr = (uint8_t *)Buffer;

		for (size_t row = startRow; row < endRow; ++row)
		{
			uint8_t *framebufferRowPtr = framebufferPtr + (row % framebuffer.Height) * framebuffer.Width * (framebuffer.BitsPerPixel / 8);
			uint8_t *bufferRowPtr = bufferPtr + row * framebuffer.Width * (framebuffer.BitsPerPixel / 8);

			for (size_t col = startCol; col < endCol; ++col)
			{
				uint8_t *framebufferPixelPtr = framebufferRowPtr + (col % framebuffer.Width) * (framebuffer.BitsPerPixel / 8);
				uint8_t *bufferPixelPtr = bufferRowPtr + col * (framebuffer.BitsPerPixel / 8);

				memcpy(framebufferPixelPtr, bufferPixelPtr, framebuffer.BitsPerPixel / 8);
			}
		}
		/*
		size_t startRow = RegionRow * RegionHeight;
		size_t endRow = startRow + RegionHeight;
		size_t startCol = RegionColumn * RegionWidth;
		size_t endCol = startCol + RegionWidth;

		for (size_t row = startRow; row < endRow; ++row)
		{
			for (size_t col = startCol; col < endCol; ++col)
			{
				size_t bufferIndex = row * framebuffer.Width + col;
				size_t framebufferIndex = (row % framebuffer.Height) * framebuffer.Width + (col % framebuffer.Width);
				memcpy((uint8_t *)framebuffer.BaseAddress + framebufferIndex * (framebuffer.BitsPerPixel / 8),
					   (uint8_t *)Buffer + bufferIndex * (framebuffer.BitsPerPixel / 8),
					   framebuffer.BitsPerPixel / 8);
			}
		}
		*/
	}

	void Display::MarkRegionDirty(size_t RegionRow, size_t RegionCol)
	{
		size_t index = RegionRow * (framebuffer.Width / RegionWidth) + RegionCol;
		DirtyMap[index] = true;
	}

	Display::Display(BootInfo::FramebufferInfo Info,
					 bool _DirectWrite)
		: framebuffer(Info), DirectWrite(_DirectWrite)
	{
		Width = Info.Width;
		Height = Info.Height;
		Size = this->framebuffer.Pitch * Height;
		if (DirectWrite)
			Buffer = (void *)this->framebuffer.BaseAddress;
		else
			Buffer = KernelAllocator.RequestPages(TO_PAGES(Size));

		// DirtyMap.resize((Width / RegionWidth) * (Height / RegionHeight), false);
	}

	Display::~Display() {}
}
