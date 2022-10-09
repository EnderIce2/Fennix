#ifndef __FENNIX_KERNEL_DISPLAY_H__
#define __FENNIX_KERNEL_DISPLAY_H__

#include <types.h>

#include <boot/binfo.h>
#include <memory.hpp>
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
        uint64_t *StartAddress;
        uint64_t *EndAddress;
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
        Font(uint64_t *Start, uint64_t *End, FontType Type);
        ~Font();
    };

    struct ScreenBuffer
    {
        void *Buffer;
        uint32_t Width, Height;
        uint64_t Size;
        uint32_t Color;
        uint32_t CursorX, CursorY;
    };

    class Display
    {
    private:
        BootInfo::FramebufferInfo framebuffer;
        Font *CurrentFont;
        ScreenBuffer *Buffers[16];

    public:
        Font *GetCurrentFont() { return CurrentFont; }
        void SetCurrentFont(Font *Font) { CurrentFont = Font; }
        void CreateBuffer(uint32_t Width, uint32_t Height, int Index)
        {
            uint64_t Size = framebuffer.Pitch * Height;
            ScreenBuffer *buffer = new ScreenBuffer;
            buffer->Buffer = KernelAllocator.RequestPages(TO_PAGES(Size));
            buffer->Width = Width;
            buffer->Height = Height;
            buffer->Size = Size;
            buffer->Color = 0x000000;
            buffer->CursorX = 0;
            buffer->CursorY = 0;
            Buffers[Index] = buffer;
            memset(buffer->Buffer, 0, Size);
        }
        void SetBuffer(int Index) { memcpy(framebuffer.BaseAddress, Buffers[Index]->Buffer, Buffers[Index]->Size); }
        void ClearBuffer(int Index) { memset(Buffers[Index]->Buffer, 0, Buffers[Index]->Size); }
        void DeleteBuffer(int Index)
        {
            if (Buffers[Index] == nullptr)
                return;
            KernelAllocator.FreePages(Buffers[Index]->Buffer, TO_PAGES(Buffers[Index]->Size));
            delete Buffers[Index];
        }

        void SetPixel(uint32_t X, uint32_t Y, uint32_t Color, int Index)
        {
            if (X >= Buffers[Index]->Width || Y >= Buffers[Index]->Height)
                return;
            uint32_t *Pixel = (uint32_t *)((uint64_t)Buffers[Index]->Buffer + (Y * Buffers[Index]->Width + X) * (framebuffer.BitsPerPixel / 8));
            *Pixel = Color;
        }

        uint32_t GetPixel(uint32_t X, uint32_t Y, int Index)
        {
            if (X >= Buffers[Index]->Width || Y >= Buffers[Index]->Height)
                return 0;
            uint32_t *Pixel = (uint32_t *)((uint64_t)Buffers[Index]->Buffer + (Y * Buffers[Index]->Width + X) * (framebuffer.BitsPerPixel / 8));
            return *Pixel;
        }

        void Scroll(int Index, int Lines)
        {
            if (Lines == 0)
                return;

            if (Lines > 0)
            {
                memmove(Buffers[Index]->Buffer,
                        (uint8_t *)Buffers[Index]->Buffer + (Buffers[Index]->Width * (framebuffer.BitsPerPixel / 8) * Lines),
                        Buffers[Index]->Size - (Buffers[Index]->Width * (framebuffer.BitsPerPixel / 8) * Lines));

                memset((uint8_t *)Buffers[Index]->Buffer + (Buffers[Index]->Size - (Buffers[Index]->Width * (framebuffer.BitsPerPixel / 8) * Lines)),
                       0,
                       Buffers[Index]->Width * (framebuffer.BitsPerPixel / 8) * Lines);
            }
        }

        char Print(char Char, int Index);
        Display(BootInfo::FramebufferInfo Info, bool LoadDefaultFont = true);
        ~Display();
    };
}

#endif // !__FENNIX_KERNEL_DISPLAY_H__
