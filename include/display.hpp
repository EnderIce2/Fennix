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

#ifndef __FENNIX_KERNEL_DISPLAY_H__
#define __FENNIX_KERNEL_DISPLAY_H__

#include <types.h>

#include <boot/binfo.h>
#include <memory.hpp>
#include <debug.h>
#include <cstring>

extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn10x20b_psf_end;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn10x20b_psf_size;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn10x20b_psf_start;

extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn5x9b_psf_end;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn5x9b_psf_size;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn5x9b_psf_start;

extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn5x9r_psf_end;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn5x9r_psf_size;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn5x9r_psf_start;

extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn10x20r_psf_end;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn10x20r_psf_size;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn10x20r_psf_start;

extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn6x12b_psf_end;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn6x12b_psf_size;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn6x12b_psf_start;

extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn6x12r_psf_end;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn6x12r_psf_size;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn6x12r_psf_start;

extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn7x13b_psf_end;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn7x13b_psf_size;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn7x13b_psf_start;

extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn7x13r_psf_end;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn7x13r_psf_size;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn7x13r_psf_start;

extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn7x14b_psf_end;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn7x14b_psf_size;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn7x14b_psf_start;

extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn7x14r_psf_end;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn7x14r_psf_size;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn7x14r_psf_start;

extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn8x15b_psf_end;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn8x15b_psf_size;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn8x15b_psf_start;

extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn8x15r_psf_end;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn8x15r_psf_size;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn8x15r_psf_start;

extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn8x16b_psf_end;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn8x16b_psf_size;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn8x16b_psf_start;

extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn8x16r_psf_end;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn8x16r_psf_size;
extern uintptr_t _binary_files_tamsyn_font_1_11_Tamsyn8x16r_psf_start;

namespace Video
{
#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

#define PSF2_MAGIC0 0x72
#define PSF2_MAGIC1 0xb5
#define PSF2_MAGIC2 0x4a
#define PSF2_MAGIC3 0x86

	enum DisplayFont
	{
		Tamsyn5x9r,
		Tamsyn5x9b,
		Tamsyn6x12r,
		Tamsyn6x12b,
		Tamsyn7x14r,
		Tamsyn7x14b,
		Tamsyn7x13r,
		Tamsyn7x13b,
		Tamsyn8x15r,
		Tamsyn8x15b,
		Tamsyn8x16r,
		Tamsyn8x16b,
		Tamsyn10x20r,
		Tamsyn10x20b,
	};

	struct PSF1_HEADER
	{
		uint8_t magic[2];
		uint8_t mode;
		uint8_t charsize;
	};

	struct PSF2_HEADER
	{
		uint8_t magic[4];
		uint32_t version;
		uint32_t headersize;
		uint32_t flags;
		uint32_t length;
		uint32_t charsize;
		uint32_t height, width;
	};

	typedef struct _PSF1_FONT
	{
		PSF1_HEADER *Header;
		void *GlyphBuffer;
	} PSF1_FONT;

	typedef struct _PSF2_FONT
	{
		PSF2_HEADER *Header;
		void *GlyphBuffer;
	} PSF2_FONT;

	enum FontType
	{
		None,
		PCScreenFont1,
		PCScreenFont2
	};

	struct FontInfo
	{
		uintptr_t *StartAddress;
		uintptr_t *EndAddress;
		PSF1_FONT *PSF1Font;
		PSF2_FONT *PSF2Font;
		uint32_t Width, Height;
		FontType Type;
	};

	class Font
	{
	private:
		FontInfo Info;

	public:
		FontInfo GetInfo() { return Info; }
		Font(uintptr_t *Start, uintptr_t *End, FontType Type);
		~Font();
	};

	union Pixel
	{
		struct
		{
			uint8_t Alpha;
			uint8_t Red;
			uint8_t Green;
			uint8_t Blue;
		};
		uint32_t raw;
	};

	class Display
	{
	private:
		BootInfo::FramebufferInfo framebuffer;
		Font *DefaultFont = nullptr;
		Font *CurrentFont = nullptr;
		bool ColorIteration = false;
		int ColorPickerIteration = 0;

		void *Buffer;
		size_t Size;
		uint32_t Width, Height;
		uint32_t Color;
		uint32_t CursorX, CursorY;
		bool DoNotScroll;
		bool DirectWrite;

		uint32_t RegionWidth = 64, RegionHeight = 64;
		std::vector<bool> DirtyMap;

	public:
		decltype(Buffer) &GetBuffer = Buffer;
		decltype(Size) &GetSize = Size;
		decltype(Width) &GetWidth = Width;
		decltype(Height) &GetHeight = Height;

		BootInfo::FramebufferInfo GetFramebufferStruct() { return framebuffer; }
		Font *GetCurrentFont();
		void SetCurrentFont(Font *Font);
		uint16_t GetBitsPerPixel();
		size_t GetPitch();

		void ClearBuffer();
		void SetPixel(uint32_t X, uint32_t Y, uint32_t Color);
		void Scroll(int Lines);

		void SetDoNotScroll(bool Value)
		{
			this->DoNotScroll = Value;
		}

		void SetBufferCursor(uint32_t X, uint32_t Y)
		{
			this->CursorX = X;
			this->CursorY = Y;
		}

		void GetBufferCursor(uint32_t *X, uint32_t *Y)
		{
			*X = this->CursorX;
			*Y = this->CursorY;
		}

		void UpdateRegion(size_t RegionRow, size_t RegionColumn);
		void MarkRegionDirty(size_t RegionRow, size_t RegionCol);
		void UpdateBuffer();

		char Print(char Char,
				   Video::Font *Font = nullptr,
				   bool WriteToUART = false);

		Display(BootInfo::FramebufferInfo Info,
				bool LoadDefaultFont = true,
				bool DirectWrite = true);
		~Display();
	};
}

#endif // !__FENNIX_KERNEL_DISPLAY_H__
