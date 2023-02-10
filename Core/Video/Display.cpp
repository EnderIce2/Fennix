#include <display.hpp>
#include <lock.hpp>
#include <uart.hpp>
#include <debug.h>

extern uintptr_t _binary_Files_tamsyn_font_1_11_Tamsyn7x14r_psf_start;
extern uintptr_t _binary_Files_tamsyn_font_1_11_Tamsyn7x14r_psf_end;
extern uintptr_t _binary_Files_tamsyn_font_1_11_Tamsyn7x14r_psf_size;

NewLock(PrintLock);

namespace Video
{
    Font *Display::GetCurrentFont()
    {
        return CurrentFont;
    }

    void Display::SetCurrentFont(Font *Font)
    {
        CurrentFont = Font;
    }

    void Display::CreateBuffer(uint32_t Width, uint32_t Height, int Index)
    {
        if (Width == 0 && Height == 0)
        {
            Width = this->framebuffer.Width;
            Height = this->framebuffer.Height;
            debug("No width and height specified, using %ldx%lld", Width, Height);
        }

        size_t Size = this->framebuffer.Pitch * Height;
        if (this->Buffers[Index])
        {
            if (this->Buffers[Index]->Checksum == 0xDEAD)
            {
                warn("Buffer %d already exists, skipping creation", Index);
                return;
            }
        }

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

    void Display::SetBuffer(int Index)
    {
        memcpy(this->framebuffer.BaseAddress, this->Buffers[Index]->Buffer, this->Buffers[Index]->Size);
    }

    ScreenBuffer *Display::GetBuffer(int Index)
    {
        return this->Buffers[Index];
    }

    void Display::ClearBuffer(int Index)
    {
        memset(this->Buffers[Index]->Buffer, 0, this->Buffers[Index]->Size);
    }

    void Display::DeleteBuffer(int Index)
    {
        if (this->Buffers[Index] == nullptr)
            return;
        KernelAllocator.FreePages(this->Buffers[Index]->Buffer, TO_PAGES(this->Buffers[Index]->Size));
        this->Buffers[Index]->Checksum = 0; // Making sure that the buffer is not used anymore
        delete this->Buffers[Index];
    }

    void Display::SetBufferCursor(int Index, uint32_t X, uint32_t Y)
    {
        this->Buffers[Index]->CursorX = X;
        this->Buffers[Index]->CursorY = Y;
    }

    void Display::GetBufferCursor(int Index, uint32_t *X, uint32_t *Y)
    {
        *X = this->Buffers[Index]->CursorX;
        *Y = this->Buffers[Index]->CursorY;
    }

    void Display::SetPixel(uint32_t X, uint32_t Y, uint32_t Color, int Index)
    {
        if (unlikely(X >= this->Buffers[Index]->Width))
            X = this->Buffers[Index]->Width - 1;
        if (unlikely(Y >= this->Buffers[Index]->Height))
            Y = this->Buffers[Index]->Height - 1;
        uint32_t *Pixel = (uint32_t *)((uintptr_t)this->Buffers[Index]->Buffer + (Y * this->Buffers[Index]->Width + X) * (this->framebuffer.BitsPerPixel / 8));
        *Pixel = Color;
    }

    uint32_t Display::GetPixel(uint32_t X, uint32_t Y, int Index)
    {
        if (unlikely(X >= this->Buffers[Index]->Width || Y >= this->Buffers[Index]->Height))
            return 0;
        uint32_t *Pixel = (uint32_t *)((uintptr_t)this->Buffers[Index]->Buffer + (Y * this->Buffers[Index]->Width + X) * (this->framebuffer.BitsPerPixel / 8));
        return *Pixel;
    }

    uint16_t Display::GetBitsPerPixel()
    {
        return this->framebuffer.BitsPerPixel;
    }

    uint64_t Display::GetPitch()
    {
        return this->framebuffer.Pitch;
    }

    void Display::Scroll(int Index, int Lines)
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

    char Display::Print(char Char, int Index, bool WriteToUART)
    {
        // SmartLock(PrintLock);

        if (this->ColorIteration)
        {
            // RRGGBB
            if (Char >= '0' && Char <= '9')
                this->Buffers[Index]->Color = (this->Buffers[Index]->Color << 4) | (Char - '0');
            else if (Char >= 'a' && Char <= 'f')
                this->Buffers[Index]->Color = (this->Buffers[Index]->Color << 4) | (Char - 'a' + 10);
            else if (Char >= 'A' && Char <= 'F')
                this->Buffers[Index]->Color = (this->Buffers[Index]->Color << 4) | (Char - 'A' + 10);
            else
                this->Buffers[Index]->Color = 0xFFFFFF;
            if (WriteToUART)
                UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM1).Write(Char);
            this->ColorPickerIteration++;
            if (this->ColorPickerIteration == 6)
            {
                this->ColorPickerIteration = 0;
                if (WriteToUART)
                    UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM1).Write(']');
                this->ColorIteration = false;
            }
            return Char;
        }

        if (WriteToUART)
            UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM1).Write(Char);

        switch (Char)
        {
        case '\e':
        {
            if (WriteToUART)
                UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM1).Write('[');
            this->ColorIteration = true;
            return Char;
        }
        case '\b':
        {
            switch (this->CurrentFont->GetInfo().Type)
            {
            case FontType::PCScreenFont1:
            {
                fixme("PCScreenFont1");
                break;
            }
            case FontType::PCScreenFont2:
            {
                uint32_t fonthdrWidth = this->CurrentFont->GetInfo().PSF2Font->Header->width;
                uint32_t fonthdrHeight = this->CurrentFont->GetInfo().PSF2Font->Header->height;

                for (unsigned long Y = this->Buffers[Index]->CursorY; Y < this->Buffers[Index]->CursorY + fonthdrHeight; Y++)
                    for (unsigned long X = this->Buffers[Index]->CursorX - fonthdrWidth; X < this->Buffers[Index]->CursorX; X++)
                        *(uint32_t *)((uintptr_t)this->Buffers[Index]->Buffer +
                                      (Y * this->Buffers[Index]->Width + X) * (this->framebuffer.BitsPerPixel / 8)) = 0;
                break;
            }
            default:
                warn("Unsupported font type");
                break;
            }

            if (this->Buffers[Index]->CursorX > 0)
                this->Buffers[Index]->CursorX -= this->GetCurrentFont()->GetInfo().Width;

            return Char;
        }
        case '\t':
        {
            this->Buffers[Index]->CursorX = (this->Buffers[Index]->CursorX + 8) & ~(8 - 1);
            return Char;
        }
        case '\r':
        {
            this->Buffers[Index]->CursorX = 0;
            return Char;
        }
        case '\n':
        {
            this->Buffers[Index]->CursorX = 0;
            this->Buffers[Index]->CursorY += this->GetCurrentFont()->GetInfo().Height;
            return Char;
        }
        }

        if (this->Buffers[Index]->CursorY + this->GetCurrentFont()->GetInfo().Height >= this->Buffers[Index]->Height)
        {
            this->Buffers[Index]->CursorY -= this->GetCurrentFont()->GetInfo().Height;
            this->Scroll(Index, 1);
        }

        if (this->Buffers[Index]->CursorX + this->GetCurrentFont()->GetInfo().Width >= this->Buffers[Index]->Width)
        {
            this->Buffers[Index]->CursorX = 0;
            this->Buffers[Index]->CursorY += this->GetCurrentFont()->GetInfo().Height;
        }

        switch (this->CurrentFont->GetInfo().Type)
        {
        case FontType::PCScreenFont1:
        {
            uint32_t *PixelPtr = (uint32_t *)this->Buffers[Index]->Buffer;
            char *FontPtr = (char *)this->CurrentFont->GetInfo().PSF1Font->GlyphBuffer + (Char * this->CurrentFont->GetInfo().PSF1Font->Header->charsize);
            for (uint64_t Y = this->Buffers[Index]->CursorY; Y < this->Buffers[Index]->CursorY + 16; Y++)
            {
                for (uint64_t X = this->Buffers[Index]->CursorX; X < this->Buffers[Index]->CursorX + 8; X++)
                    if ((*FontPtr & (0b10000000 >> (X - this->Buffers[Index]->CursorX))) > 0)
                        *(unsigned int *)(PixelPtr + X + (Y * this->Buffers[Index]->Width)) = this->Buffers[Index]->Color;
                FontPtr++;
            }
            this->Buffers[Index]->CursorX += 8;

            break;
        }
        case FontType::PCScreenFont2:
        {
            // if (this->CurrentFont->PSF2Font->GlyphBuffer == (uint16_t *)0x01) // HAS UNICODE TABLE
            //     Char = this->CurrentFont->PSF2Font->GlyphBuffer[Char];
            int BytesPerLine = (this->CurrentFont->GetInfo().PSF2Font->Header->width + 7) / 8;
            char *FontPtr = (char *)this->CurrentFont->GetInfo().StartAddress +
                            this->CurrentFont->GetInfo().PSF2Font->Header->headersize +
                            (Char > 0 && (unsigned char)Char < this->CurrentFont->GetInfo().PSF2Font->Header->length ? Char : 0) *
                                this->CurrentFont->GetInfo().PSF2Font->Header->charsize;

            uint32_t fonthdrWidth = this->CurrentFont->GetInfo().PSF2Font->Header->width;
            uint32_t fonthdrHeight = this->CurrentFont->GetInfo().PSF2Font->Header->height;

            for (uint64_t Y = this->Buffers[Index]->CursorY; Y < this->Buffers[Index]->CursorY + fonthdrHeight; Y++)
            {
                for (uint64_t X = this->Buffers[Index]->CursorX; X < this->Buffers[Index]->CursorX + fonthdrWidth; X++)
                    if ((*FontPtr & (0b10000000 >> (X - this->Buffers[Index]->CursorX))) > 0)
                        *(uint32_t *)((uintptr_t)this->Buffers[Index]->Buffer +
                                      (Y * this->Buffers[Index]->Width + X) * (this->framebuffer.BitsPerPixel / 8)) = this->Buffers[Index]->Color;

                FontPtr += BytesPerLine;
            }
            this->Buffers[Index]->CursorX += fonthdrWidth;
            break;
        }
        default:
            warn("Unsupported font type");
            break;
        }
        return Char;
    }

    void Display::DrawString(const char *String, uint32_t X, uint32_t Y, int Index, bool WriteToUART)
    {
        this->Buffers[Index]->CursorX = X;
        this->Buffers[Index]->CursorY = Y;

        for (int i = 0; String[i] != '\0'; i++)
            this->Print(String[i], Index, WriteToUART);
    }

    Display::Display(BootInfo::FramebufferInfo Info, bool LoadDefaultFont)
    {
        this->framebuffer = Info;
        if (LoadDefaultFont)
        {
            this->CurrentFont = new Font(&_binary_Files_tamsyn_font_1_11_Tamsyn7x14r_psf_start, &_binary_Files_tamsyn_font_1_11_Tamsyn7x14r_psf_end, FontType::PCScreenFont2);
            FontInfo Info = this->CurrentFont->GetInfo();
            debug("Font loaded: %dx%d %s",
                  Info.Width, Info.Height, Info.Type == FontType::PCScreenFont1 ? "PSF1" : "PSF2");
        }
        this->CreateBuffer(Info.Width, Info.Height, 0);
    }

    Display::~Display()
    {
        for (int i = 0; i < 16; i++)
            DeleteBuffer(i);
    }
}
