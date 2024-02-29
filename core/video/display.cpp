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
	Font *Display::GetCurrentFont() { return CurrentFont; }
	void Display::SetCurrentFont(Font *Font) { CurrentFont = Font; }
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

	void Display::Scroll(int Lines)
	{
		if (this->DoNotScroll)
			return;

		if (Lines == 0)
			return;

		if (Lines > 0)
		{
			uint32_t LineSize = this->Width * (this->framebuffer.BitsPerPixel / 8);
			uint32_t BytesToMove = LineSize * Lines * this->CurrentFont->GetInfo().Height;
			size_t BytesToClear = this->Size - BytesToMove;
			memmove(this->Buffer, (uint8_t *)this->Buffer + BytesToMove, BytesToClear);
			memset((uint8_t *)this->Buffer + BytesToClear, 0, BytesToMove);

			// memset(this->DirtyMap.data(), true, this->DirtyMap.size());
		}
	}

	__no_sanitize("undefined") char Display::Print(char Char,
												   Video::Font *_Font,
												   bool WriteToUART,
												   bool IgnoreSpecialChars)
	{
		// SmartLock(PrintLock);

		if (this->ColorIteration)
		{
			// RRGGBB
			if (Char >= '0' && Char <= '9')
				this->Color = (this->Color << 4) | (Char - '0');
			else if (Char >= 'a' && Char <= 'f')
				this->Color = (this->Color << 4) | (Char - 'a' + 10);
			else if (Char >= 'A' && Char <= 'F')
				this->Color = (this->Color << 4) | (Char - 'A' + 10);
			else
				this->Color = 0xFFFFFF;
			this->ColorPickerIteration++;
			if (this->ColorPickerIteration == 6)
			{
				this->ColorPickerIteration = 0;
				this->ColorIteration = false;
			}
			return Char;
		}

		if (WriteToUART && Char != '\e')
			UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM1).Write(Char);

		if (_Font == nullptr)
			_Font = this->DefaultFont;
		this->CurrentFont = _Font;
		uint32_t FontHeight;

		if (IgnoreSpecialChars)
			goto IgnoreSpecialChars;

		switch (Char)
		{
		case '\e':
		{
			this->ColorIteration = true;
			return Char;
		}
		case '\b':
		{
			switch (_Font->GetInfo().Type)
			{
			case FontType::PCScreenFont1:
			{
				fixme("PCScreenFont1");
				break;
			}
			case FontType::PCScreenFont2:
			{
				uint32_t fonthdrWidth = _Font->GetInfo().PSF2Font->Header->width;
				uint32_t fonthdrHeight = _Font->GetInfo().PSF2Font->Header->height;

				for (unsigned long Y = this->CursorY; Y < this->CursorY + fonthdrHeight; Y++)
					for (unsigned long X = this->CursorX - fonthdrWidth; X < this->CursorX; X++)
						*(uint32_t *)((uintptr_t)this->Buffer +
									  (Y * this->Width + X) * (this->framebuffer.BitsPerPixel / 8)) = 0;

				// for (size_t i = 0; i < 9; ++i)
				// {
				// 	size_t row = (CursorY / RegionHeight) + (i / 3) - 1;
				// 	size_t col = (CursorX / RegionWidth) + (i % 3) - 1;
				// 	if (row < Height / RegionHeight && col < Width / RegionWidth)
				// 		MarkRegionDirty(row, col);
				// }
				break;
			}
			default:
				warn("Unsupported font type");
				break;
			}

			if (this->CursorX > 0)
				this->CursorX -= this->GetCurrentFont()->GetInfo().Width;

			return Char;
		}
		case '\t':
		{
			this->CursorX = (this->CursorX + 8) & ~(8 - 1);
			return Char;
		}
		case '\r':
		{
			this->CursorX = 0;
			return Char;
		}
		case '\n':
		{
			this->CursorX = 0;
			this->CursorY += this->GetCurrentFont()->GetInfo().Height;
			return Char;
		}
		default:
			break;
		}

	IgnoreSpecialChars:
		FontHeight = this->GetCurrentFont()->GetInfo().Height;

		if (this->CursorX + this->GetCurrentFont()->GetInfo().Width >= this->Width)
		{
			this->CursorX = 0;
			this->CursorY += FontHeight;
		}

		if (this->CursorY + FontHeight >= this->Height)
		{
			if (!this->DoNotScroll)
			{
				this->CursorY -= FontHeight;
				this->Scroll(1);
			}
		}

		switch (_Font->GetInfo().Type)
		{
		case FontType::PCScreenFont1:
		{
			uint32_t *PixelPtr = (uint32_t *)this->Buffer;
			char *FontPtr = (char *)_Font->GetInfo().PSF1Font->GlyphBuffer + (Char * _Font->GetInfo().PSF1Font->Header->charsize);
			for (uint64_t Y = this->CursorY; Y < this->CursorY + 16; Y++)
			{
				for (uint64_t X = this->CursorX; X < this->CursorX + 8; X++)
					if ((*FontPtr & (0b10000000 >> (X - this->CursorX))) > 0)
						*(unsigned int *)(PixelPtr + X + (Y * this->Width)) = this->Color;
				FontPtr++;
			}
			this->CursorX += 8;

			break;
		}
		case FontType::PCScreenFont2:
		{
			// if (_Font->PSF2Font->GlyphBuffer == (uint16_t *)0x01) // HAS UNICODE TABLE
			//     Char = _Font->PSF2Font->GlyphBuffer[Char];

			FontInfo fInfo = _Font->GetInfo();

			int BytesPerLine = (fInfo.PSF2Font->Header->width + 7) / 8;
			char *FontAddress = (char *)fInfo.StartAddress;
			uint32_t FontHeaderSize = fInfo.PSF2Font->Header->headersize;
			uint32_t FontCharSize = fInfo.PSF2Font->Header->charsize;
			uint32_t FontLength = fInfo.PSF2Font->Header->length;
			char *FontPtr = FontAddress + FontHeaderSize + (Char > 0 && (uint32_t)Char < FontLength ? Char : 0) * FontCharSize;

			uint32_t FontHdrWidth = fInfo.PSF2Font->Header->width;
			uint32_t FontHdrHeight = fInfo.PSF2Font->Header->height;

			for (size_t Y = this->CursorY; Y < this->CursorY + FontHdrHeight; Y++)
			{
				for (size_t X = this->CursorX; X < this->CursorX + FontHdrWidth; X++)
				{
					if ((*FontPtr & (0b10000000 >> (X - this->CursorX))) > 0)
					{
						void *FramebufferAddress = (void *)((uintptr_t)this->Buffer +
															(Y * this->Width + X) *
																(this->framebuffer.BitsPerPixel / 8));
						*(uint32_t *)FramebufferAddress = this->Color;
					}
				}
				FontPtr += BytesPerLine;
			}
			this->CursorX += FontHdrWidth;
			break;
		}
		default:
			warn("Unsupported font type");
			break;
		}

		// // MarkRegionDirty(CursorY / RegionHeight, CursorX / RegionWidth);
		// for (size_t i = 0; i < 9; ++i)
		// {
		// 	size_t row = (CursorY / RegionHeight) + (i / 3) - 1;
		// 	size_t col = (CursorX / RegionWidth) + (i % 3) - 1;
		// 	if (row < Height / RegionHeight && col < Width / RegionWidth)
		// 		MarkRegionDirty(row, col);
		// }
		return Char;
	}

	void Display::PrintString(const char *String,
							  Video::Font *Font,
							  bool WriteToUART,
							  bool IgnoreSpecialChars)
	{
		for (size_t i = 0; String[i] != '\0'; ++i)
			Print(String[i], Font, WriteToUART, IgnoreSpecialChars);
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
					 bool LoadDefaultFont,
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

		if (!LoadDefaultFont)
			return;
		this->DefaultFont = new Font(&_binary_files_tamsyn_font_1_11_Tamsyn7x14r_psf_start, &_binary_files_tamsyn_font_1_11_Tamsyn7x14r_psf_end, FontType::PCScreenFont2);
#ifdef DEBUG
		FontInfo fInfo = this->DefaultFont->GetInfo();
		debug("Font loaded: %dx%d %s",
			  fInfo.Width, fInfo.Height,
			  fInfo.Type == FontType::PCScreenFont1 ? "PSF1" : "PSF2");
#endif
	}

	Display::~Display() {}
}
