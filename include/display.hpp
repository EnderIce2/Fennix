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
        long Checksum;
    };

    class Display
    {
    private:
        BootInfo::FramebufferInfo framebuffer;
        Font *CurrentFont;
        ScreenBuffer *Buffers[256];
        bool ColorIteration = false;
        int ColorPickerIteration = 0;

    public:
        Font *GetCurrentFont() { return CurrentFont; }
        void SetCurrentFont(Font *Font) { CurrentFont = Font; }
        void CreateBuffer(uint32_t Width, uint32_t Height, int Index)
        {
            if (Width == 0 && Height == 0)
            {
                Width = this->framebuffer.Width;
                Height = this->framebuffer.Height;
                debug("No width and height specified, using %ldx%lld", Width, Height);
            }

            size_t Size = this->framebuffer.Pitch * Height;
            if (!this->Buffers[Index])
            {
                if (this->Buffers[Index]->Checksum != 0xDEAD)
                {
                    ScreenBuffer *buffer = new ScreenBuffer;
                    buffer->Buffer = KernelAllocator.RequestPages(TO_PAGES(Size));
                    buffer->Width = Width;
                    buffer->Height = Height;
                    buffer->Size = Size;
                    buffer->Color = 0xFFFFFF;
                    buffer->CursorX = 0;
                    buffer->CursorY = 0;
                    this->Buffers[Index] = buffer;
                    memset(this->Buffers[Index]->Buffer, 0, Size);
                    this->Buffers[Index]->Checksum = 0xDEAD;
                }
                else
                    warn("Buffer %d already exists, skipping creation", Index);
            }
            else
                warn("Buffer %d already exists, skipping creation", Index);
        }
        void SetBuffer(int Index) { memcpy(this->framebuffer.BaseAddress, this->Buffers[Index]->Buffer, this->Buffers[Index]->Size); }
        ScreenBuffer *GetBuffer(int Index) { return this->Buffers[Index]; }
        void ClearBuffer(int Index) { memset(this->Buffers[Index]->Buffer, 0, this->Buffers[Index]->Size); }
        void DeleteBuffer(int Index)
        {
            if (this->Buffers[Index] == nullptr)
                return;
            KernelAllocator.FreePages(this->Buffers[Index]->Buffer, TO_PAGES(this->Buffers[Index]->Size));
            this->Buffers[Index]->Checksum = 0; // Making sure that the buffer is not used anymore
            delete this->Buffers[Index];
        }

        void SetBufferCursor(int Index, uint32_t X, uint32_t Y)
        {
            this->Buffers[Index]->CursorX = X;
            this->Buffers[Index]->CursorY = Y;
        }

        void GetBufferCursor(int Index, uint32_t *X, uint32_t *Y)
        {
            *X = this->Buffers[Index]->CursorX;
            *Y = this->Buffers[Index]->CursorY;
        }

        void SetPixel(uint32_t X, uint32_t Y, uint32_t Color, int Index)
        {
            if (X >= this->Buffers[Index]->Width)
                X = this->Buffers[Index]->Width - 1;
            if (Y >= this->Buffers[Index]->Height)
                Y = this->Buffers[Index]->Height - 1;
            uint32_t *Pixel = (uint32_t *)((uintptr_t)this->Buffers[Index]->Buffer + (Y * this->Buffers[Index]->Width + X) * (this->framebuffer.BitsPerPixel / 8));
            *Pixel = Color;
        }

        uint32_t GetPixel(uint32_t X, uint32_t Y, int Index)
        {
            if (X >= this->Buffers[Index]->Width || Y >= this->Buffers[Index]->Height)
                return 0;
            uint32_t *Pixel = (uint32_t *)((uintptr_t)this->Buffers[Index]->Buffer + (Y * this->Buffers[Index]->Width + X) * (this->framebuffer.BitsPerPixel / 8));
            return *Pixel;
        }

        void Scroll(int Index, int Lines)
        {
            if (Lines == 0)
                return;

            if (Lines > 0)
            {
                for (uint32_t i = 0; i < this->CurrentFont->GetInfo().Height; i++) // TODO: Make this more efficient.
                {
                    memmove(this->Buffers[Index]->Buffer,
                            (uint8_t *)this->Buffers[Index]->Buffer + (this->Buffers[Index]->Width * (this->framebuffer.BitsPerPixel / 8) * Lines),
                            this->Buffers[Index]->Size - (this->Buffers[Index]->Width * (this->framebuffer.BitsPerPixel / 8) * Lines));

                    memset((uint8_t *)this->Buffers[Index]->Buffer + (this->Buffers[Index]->Size - (this->Buffers[Index]->Width * (this->framebuffer.BitsPerPixel / 8) * Lines)),
                           0,
                           this->Buffers[Index]->Width * (this->framebuffer.BitsPerPixel / 8) * Lines);
                }
            }
        }

        char Print(char Char, int Index, bool WriteToUART = false);
        Display(BootInfo::FramebufferInfo Info, bool LoadDefaultFont = true);
        ~Display();
    };
}

#endif // !__FENNIX_KERNEL_DISPLAY_H__
