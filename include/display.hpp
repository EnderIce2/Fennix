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

namespace Video
{
#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

#define PSF2_MAGIC0 0x72
#define PSF2_MAGIC1 0xb5
#define PSF2_MAGIC2 0x4a
#define PSF2_MAGIC3 0x86

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

    struct ScreenBuffer
    {
        void *Buffer = nullptr;
        uint32_t Width, Height;
        size_t Size;
        uint32_t Color;
        uint32_t CursorX, CursorY;
        char Brightness;
        bool DoNotScroll;
        long long Checksum;
    };

    class Display
    {
    private:
        BootInfo::FramebufferInfo framebuffer;
        Font *CurrentFont = nullptr;
        ScreenBuffer Buffers[256];
        bool ColorIteration = false;
        int ColorPickerIteration = 0;

    public:
        Font *GetCurrentFont();
        void SetCurrentFont(Font *Font);
        uint16_t GetBitsPerPixel();
        uint64_t GetPitch();

        /**
         * @brief Create a new buffer
         *
         * This function creates a new buffer.
         * 
         * For @see Width and @see Height. Both values must be greater than 0.
         *
         * @note Some indexes are reserved for the kernel.
         * 0 - Main buffer
         * 1 - Loading screen buffer
         * 200 - GUI buffer
         * 250 - Empty (crash screen)
         * 251 - Console (crash screen)
         * 252 - Tasks (crash screen)
         * 253 - Frames (crash screen)
         * 254 - Details (crash screen)
         * 255 - Main (crash screen)
         *
         * @param Width The width of the buffer
         * @param Height The height of the buffer
         * @param Index The index of the buffer (0-255)
         */
        void CreateBuffer(uint32_t Width, uint32_t Height, int Index);
        void SetBuffer(int Index);
        ScreenBuffer *GetBuffer(int Index);
        void ClearBuffer(int Index);
        void DeleteBuffer(int Index);
        void SetBrightness(int Value, int Index);
        void SetBufferCursor(int Index, uint32_t X, uint32_t Y);
        void GetBufferCursor(int Index, uint32_t *X, uint32_t *Y);
        void SetPixel(uint32_t X, uint32_t Y, uint32_t Color, int Index);
        uint32_t GetPixel(uint32_t X, uint32_t Y, int Index);
        void Scroll(int Index, int Lines);
        void SetDoNotScroll(bool Value, int Index);

        char Print(char Char, int Index, bool WriteToUART = false);
        void DrawString(const char *String, uint32_t X, uint32_t Y, int Index, bool WriteToUART = false);
        Display(BootInfo::FramebufferInfo Info, bool LoadDefaultFont = true);
        ~Display();
    };
}

#endif // !__FENNIX_KERNEL_DISPLAY_H__
