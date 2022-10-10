#include <display.hpp>
#include <lock.hpp>
#include <debug.h>

extern uint64_t _binary_Files_ter_powerline_v12n_psf_start;
extern uint64_t _binary_Files_ter_powerline_v12n_psf_end;
extern uint64_t _binary_Files_ter_powerline_v12n_psf_size;

NEWLOCK(PrintLock);

namespace Video
{
    char Display::Print(char Char, int Index)
    {
        SMARTLOCK(PrintLock);
        switch (Char)
        {
        case '\b':
        {
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
                        *(uint32_t *)((uint64_t)this->Buffers[Index]->Buffer +
                                      (Y * this->Buffers[Index]->Width + X) * (this->framebuffer.BitsPerPixel / 8)) = 0xFFFFFF;

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

    Display::Display(BootInfo::FramebufferInfo Info, bool LoadDefaultFont)
    {
        this->framebuffer = Info;
        if (LoadDefaultFont)
        {
            this->CurrentFont = new Font(&_binary_Files_ter_powerline_v12n_psf_start, &_binary_Files_ter_powerline_v12n_psf_end, FontType::PCScreenFont2);
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
