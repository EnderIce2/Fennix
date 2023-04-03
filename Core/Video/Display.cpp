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

extern uintptr_t _binary_Files_tamsyn_font_1_11_Tamsyn7x14r_psf_start;
extern uintptr_t _binary_Files_tamsyn_font_1_11_Tamsyn7x14r_psf_end;
extern uintptr_t _binary_Files_tamsyn_font_1_11_Tamsyn7x14r_psf_size;

NewLock(PrintLock);

namespace Video
{
    Font *Display::GetCurrentFont() { return CurrentFont; }
    void Display::SetCurrentFont(Font *Font) { CurrentFont = Font; }
    uint16_t Display::GetBitsPerPixel() { return this->framebuffer.BitsPerPixel; }
    uint64_t Display::GetPitch() { return this->framebuffer.Pitch; }

    void Display::CreateBuffer(uint32_t Width, uint32_t Height, int Index)
    {
        if (Width == 0 || Height == 0)
        {
            Width = this->framebuffer.Width;
            Height = this->framebuffer.Height;
            debug("Buffer %d created with default size (%d, %d)", Index, Width, Height);
        }

        if (this->Buffers[Index].Checksum == 0xBBFFE515A117E)
        {
            warn("Buffer %d already exists, skipping creation", Index);
            return;
        }

        size_t Size = this->framebuffer.Pitch * Height;

        this->Buffers[Index].Buffer = KernelAllocator.RequestPages(TO_PAGES(Size));
        memset(this->Buffers[Index].Buffer, 0, Size);

        this->Buffers[Index].Width = Width;
        this->Buffers[Index].Height = Height;
        this->Buffers[Index].Size = Size;
        this->Buffers[Index].Color = 0xFFFFFF;
        this->Buffers[Index].CursorX = 0;
        this->Buffers[Index].CursorY = 0;
        this->Buffers[Index].Brightness = 100;
        this->Buffers[Index].Checksum = 0xBBFFE515A117E;
        debug("Buffer %d created", Index);
    }

    void Display::SetBuffer(int Index)
    {
        if (unlikely(this->Buffers[Index].Checksum != 0xBBFFE515A117E))
        {
            debug("Invalid buffer %d", Index);
            return;
        }

        if (this->Buffers[Index].Brightness != 100)
            this->SetBrightness(this->Buffers[Index].Brightness, Index);

        if (this->Buffers[Index].Brightness == 0) /* Just clear the buffer */
            memset(this->Buffers[Index].Buffer, 0, this->Buffers[Index].Size);

        memcpy(this->framebuffer.BaseAddress, this->Buffers[Index].Buffer, this->Buffers[Index].Size);
    }

    ScreenBuffer *Display::GetBuffer(int Index) { return &this->Buffers[Index]; }

    void Display::ClearBuffer(int Index)
    {
        if (unlikely(this->Buffers[Index].Checksum != 0xBBFFE515A117E))
        {
            debug("Invalid buffer %d", Index);
            return;
        }

        memset(this->Buffers[Index].Buffer, 0, this->Buffers[Index].Size);
    }

    void Display::DeleteBuffer(int Index)
    {
        if (unlikely(this->Buffers[Index].Checksum != 0xBBFFE515A117E))
        {
            debug("Invalid buffer %d", Index);
            return;
        }

        KernelAllocator.FreePages(this->Buffers[Index].Buffer, TO_PAGES(this->Buffers[Index].Size));
        this->Buffers[Index].Buffer = nullptr;
        this->Buffers[Index].Checksum = 0;
        debug("Buffer %d deleted", Index);
    }

    void Display::SetBrightness(int Value, int Index)
    {
        if (unlikely(this->Buffers[Index].Checksum != 0xBBFFE515A117E))
        {
            debug("Invalid buffer %d", Index);
            return;
        }

        if (Value > 100)
            Value = 100;
        else if (Value < 0)
            Value = 0;

        uint32_t *pixel = (uint32_t *)this->Buffers[Index].Buffer;

        for (uint32_t y = 0; y < this->Buffers[Index].Height; y++)
        {
            for (uint32_t x = 0; x < this->Buffers[Index].Width; x++)
            {
                uint32_t color = pixel[y * this->Buffers[Index].Width + x];

                uint8_t r = color & 0xff;
                uint8_t g = (color >> 8) & 0xff;
                uint8_t b = (color >> 16) & 0xff;

                r = s_cst(uint8_t, (r * Value) / 100);
                g = s_cst(uint8_t, (g * Value) / 100);
                b = s_cst(uint8_t, (b * Value) / 100);

                pixel[y * this->Buffers[Index].Width + x] = (b << 16) | (g << 8) | r;
            }
        }
        this->Buffers[Index].Brightness = s_cst(char, Value);
    }

    void Display::SetBufferCursor(int Index, uint32_t X, uint32_t Y)
    {
        if (unlikely(this->Buffers[Index].Checksum != 0xBBFFE515A117E))
        {
            debug("Invalid buffer %d", Index);
            return;
        }

        this->Buffers[Index].CursorX = X;
        this->Buffers[Index].CursorY = Y;
    }

    void Display::GetBufferCursor(int Index, uint32_t *X, uint32_t *Y)
    {
        if (unlikely(this->Buffers[Index].Checksum != 0xBBFFE515A117E))
        {
            debug("Invalid buffer %d", Index);
            return;
        }

        *X = this->Buffers[Index].CursorX;
        *Y = this->Buffers[Index].CursorY;
    }

    void Display::SetPixel(uint32_t X, uint32_t Y, uint32_t Color, int Index)
    {
        if (unlikely(this->Buffers[Index].Checksum != 0xBBFFE515A117E))
        {
            debug("Invalid buffer %d", Index);
            return;
        }

        if (unlikely(X >= this->Buffers[Index].Width))
            X = this->Buffers[Index].Width - 1;

        if (unlikely(Y >= this->Buffers[Index].Height))
            Y = this->Buffers[Index].Height - 1;

        uint32_t *Pixel = (uint32_t *)((uintptr_t)this->Buffers[Index].Buffer + (Y * this->Buffers[Index].Width + X) * (this->framebuffer.BitsPerPixel / 8));
        *Pixel = Color;
    }

    uint32_t Display::GetPixel(uint32_t X, uint32_t Y, int Index)
    {
        if (unlikely(this->Buffers[Index].Checksum != 0xBBFFE515A117E))
            return 0;

        if (unlikely(X >= this->Buffers[Index].Width || Y >= this->Buffers[Index].Height))
            return 0;

        uint32_t *Pixel = (uint32_t *)((uintptr_t)this->Buffers[Index].Buffer + (Y * this->Buffers[Index].Width + X) * (this->framebuffer.BitsPerPixel / 8));
        return *Pixel;
    }

    void Display::Scroll(int Index, int Lines)
    {
        if (unlikely(this->Buffers[Index].Checksum != 0xBBFFE515A117E))
        {
            debug("Invalid buffer %d", Index);
            return;
        }

        if (this->Buffers[Index].DoNotScroll)
            return;

        if (Lines == 0)
            return;

        if (Lines > 0)
        {
            uint32_t LineSize = this->Buffers[Index].Width * (this->framebuffer.BitsPerPixel / 8);
            uint32_t BytesToMove = LineSize * Lines * this->CurrentFont->GetInfo().Height;
            size_t BytesToClear = this->Buffers[Index].Size - BytesToMove;
            memmove(this->Buffers[Index].Buffer, (uint8_t *)this->Buffers[Index].Buffer + BytesToMove, BytesToClear);
            memset((uint8_t *)this->Buffers[Index].Buffer + BytesToClear, 0, BytesToMove);
        }
    }

    void Display::SetDoNotScroll(bool Value, int Index)
    {
        if (unlikely(this->Buffers[Index].Checksum != 0xBBFFE515A117E))
        {
            debug("Invalid buffer %d", Index);
            return;
        }

        this->Buffers[Index].DoNotScroll = Value;
    }

    char Display::Print(char Char, int Index, bool WriteToUART)
    {
        if (unlikely(this->Buffers[Index].Checksum != 0xBBFFE515A117E))
            return 0;

        // SmartLock(PrintLock);

        if (this->ColorIteration)
        {
            // RRGGBB
            if (Char >= '0' && Char <= '9')
                this->Buffers[Index].Color = (this->Buffers[Index].Color << 4) | (Char - '0');
            else if (Char >= 'a' && Char <= 'f')
                this->Buffers[Index].Color = (this->Buffers[Index].Color << 4) | (Char - 'a' + 10);
            else if (Char >= 'A' && Char <= 'F')
                this->Buffers[Index].Color = (this->Buffers[Index].Color << 4) | (Char - 'A' + 10);
            else
                this->Buffers[Index].Color = 0xFFFFFF;
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

                for (unsigned long Y = this->Buffers[Index].CursorY; Y < this->Buffers[Index].CursorY + fonthdrHeight; Y++)
                    for (unsigned long X = this->Buffers[Index].CursorX - fonthdrWidth; X < this->Buffers[Index].CursorX; X++)
                        *(uint32_t *)((uintptr_t)this->Buffers[Index].Buffer +
                                      (Y * this->Buffers[Index].Width + X) * (this->framebuffer.BitsPerPixel / 8)) = 0;
                break;
            }
            default:
                warn("Unsupported font type");
                break;
            }

            if (this->Buffers[Index].CursorX > 0)
                this->Buffers[Index].CursorX -= this->GetCurrentFont()->GetInfo().Width;

            return Char;
        }
        case '\t':
        {
            this->Buffers[Index].CursorX = (this->Buffers[Index].CursorX + 8) & ~(8 - 1);
            return Char;
        }
        case '\r':
        {
            this->Buffers[Index].CursorX = 0;
            return Char;
        }
        case '\n':
        {
            this->Buffers[Index].CursorX = 0;
            this->Buffers[Index].CursorY += this->GetCurrentFont()->GetInfo().Height;
            return Char;
        }
        default:
            break;
        }

        uint32_t FontHeight = this->GetCurrentFont()->GetInfo().Height;

        if (this->Buffers[Index].CursorX + this->GetCurrentFont()->GetInfo().Width >= this->Buffers[Index].Width)
        {
            this->Buffers[Index].CursorX = 0;
            this->Buffers[Index].CursorY += FontHeight;
        }

        if (this->Buffers[Index].CursorY + FontHeight >= this->Buffers[Index].Height)
        {
            if (!this->Buffers[Index].DoNotScroll)
            {
                this->Buffers[Index].CursorY -= FontHeight;
                this->Scroll(Index, 1);
            }
        }

        switch (this->CurrentFont->GetInfo().Type)
        {
        case FontType::PCScreenFont1:
        {
            uint32_t *PixelPtr = (uint32_t *)this->Buffers[Index].Buffer;
            char *FontPtr = (char *)this->CurrentFont->GetInfo().PSF1Font->GlyphBuffer + (Char * this->CurrentFont->GetInfo().PSF1Font->Header->charsize);
            for (uint64_t Y = this->Buffers[Index].CursorY; Y < this->Buffers[Index].CursorY + 16; Y++)
            {
                for (uint64_t X = this->Buffers[Index].CursorX; X < this->Buffers[Index].CursorX + 8; X++)
                    if ((*FontPtr & (0b10000000 >> (X - this->Buffers[Index].CursorX))) > 0)
                        *(unsigned int *)(PixelPtr + X + (Y * this->Buffers[Index].Width)) = this->Buffers[Index].Color;
                FontPtr++;
            }
            this->Buffers[Index].CursorX += 8;

            break;
        }
        case FontType::PCScreenFont2:
        {
            // if (this->CurrentFont->PSF2Font->GlyphBuffer == (uint16_t *)0x01) // HAS UNICODE TABLE
            //     Char = this->CurrentFont->PSF2Font->GlyphBuffer[Char];
            int BytesPerLine = (this->CurrentFont->GetInfo().PSF2Font->Header->width + 7) / 8;
            char *FontAddress = (char *)this->CurrentFont->GetInfo().StartAddress;
            uint32_t FontHeaderSize = this->CurrentFont->GetInfo().PSF2Font->Header->headersize;
            uint32_t FontCharSize = this->CurrentFont->GetInfo().PSF2Font->Header->charsize;
            uint32_t FontLength = this->CurrentFont->GetInfo().PSF2Font->Header->length;
            char *FontPtr = FontAddress + FontHeaderSize + (Char > 0 && (uint32_t)Char < FontLength ? Char : 0) * FontCharSize;

            uint32_t FontHdrWidth = this->CurrentFont->GetInfo().PSF2Font->Header->width;
            uint32_t FontHdrHeight = this->CurrentFont->GetInfo().PSF2Font->Header->height;

            for (size_t Y = this->Buffers[Index].CursorY; Y < this->Buffers[Index].CursorY + FontHdrHeight; Y++)
            {
                for (size_t X = this->Buffers[Index].CursorX; X < this->Buffers[Index].CursorX + FontHdrWidth; X++)
                {
                    if ((*FontPtr & (0b10000000 >> (X - this->Buffers[Index].CursorX))) > 0)
                    {
                        void *FramebufferAddress = (void *)((uintptr_t)this->Buffers[Index].Buffer +
                                                            (Y * this->Buffers[Index].Width + X) *
                                                                (this->framebuffer.BitsPerPixel / 8));
                        *(uint32_t *)FramebufferAddress = this->Buffers[Index].Color;
                    }
                }
                FontPtr += BytesPerLine;
            }
            this->Buffers[Index].CursorX += FontHdrWidth;
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
        if (unlikely(this->Buffers[Index].Checksum != 0xBBFFE515A117E))
        {
            debug("Invalid buffer %d", Index);
            return;
        }

        this->Buffers[Index].CursorX = X;
        this->Buffers[Index].CursorY = Y;

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
        debug("Destructor called");
        this->ClearBuffer(0);
        this->SetBuffer(0);

        for (int i = 0; i < s_cst(int, sizeof(this->Buffers) / sizeof(this->Buffers[0])); i++)
        {
            if (this->Buffers[i].Checksum == 0xBBFFE515A117E)
                this->DeleteBuffer(i);
        }
    }
}
