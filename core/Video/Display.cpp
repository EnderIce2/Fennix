#include <display.hpp>
#include <lock.hpp>
#include <debug.h>

extern uint64_t _binary_files_ter_powerline_v12n_psf_start;
extern uint64_t _binary_files_ter_powerline_v12n_psf_end;
extern uint64_t _binary_files_ter_powerline_v12n_psf_size;

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
            if (Buffers[Index]->CursorX > 0)
                Buffers[Index]->CursorX -= this->GetCurrentFont()->GetInfo().Width;
            return Char;
        }
        case '\t':
        {
            Buffers[Index]->CursorX = (Buffers[Index]->CursorX + 8) & ~(8 - 1);
            return Char;
        }
        case '\r':
        {
            Buffers[Index]->CursorX = 0;
            return Char;
        }
        case '\n':
        {
            Buffers[Index]->CursorX = 0;
            Buffers[Index]->CursorY += this->GetCurrentFont()->GetInfo().Height;
            return Char;
        }
        }

        if (Buffers[Index]->CursorX + this->GetCurrentFont()->GetInfo().Width >= Buffers[Index]->Width)
        {
            Buffers[Index]->CursorX = 0;
            Buffers[Index]->CursorY += this->GetCurrentFont()->GetInfo().Height;
        }

        if (Buffers[Index]->CursorY + this->GetCurrentFont()->GetInfo().Height >= Buffers[Index]->Height)
        {
            Buffers[Index]->CursorY = Buffers[Index]->Height - this->GetCurrentFont()->GetInfo().Height;
            this->SetBuffer(Index);
            Scroll(Index, 1);
        }

        if (CurrentFont->GetInfo().Type == FontType::PCScreenFont2)
        {
            // if (CurrentFont->PSF2Font->GlyphBuffer == (uint16_t *)0x01) // HAS UNICODE TABLE
            //     Char = CurrentFont->PSF2Font->GlyphBuffer[Char];
            int bytesperline = (CurrentFont->GetInfo().PSF2Font->Header->width + 7) / 8;
            char *FontPtr = (char *)CurrentFont->GetInfo().StartAddress +
                            CurrentFont->GetInfo().PSF2Font->Header->headersize +
                            (Char > 0 && (unsigned char)Char < CurrentFont->GetInfo().PSF2Font->Header->length ? Char : 0) *
                                CurrentFont->GetInfo().PSF2Font->Header->charsize;

            for (unsigned long Y = Buffers[Index]->CursorY; Y < Buffers[Index]->CursorY + CurrentFont->GetInfo().PSF2Font->Header->height; Y++)
            {
                for (unsigned long X = Buffers[Index]->CursorX; X < Buffers[Index]->CursorX + CurrentFont->GetInfo().PSF2Font->Header->width; X++)
                    if ((*FontPtr & (0b10000000 >> (X - Buffers[Index]->CursorX))) > 0)
                        *(uint32_t *)((uint64_t)Buffers[Index]->Buffer + (Y * Buffers[Index]->Width + X) * (framebuffer.BitsPerPixel / 8)) = 0xFFFFFF;

                FontPtr += bytesperline;
            }
            Buffers[Index]->CursorX += CurrentFont->GetInfo().PSF2Font->Header->width;
            return Char;
        }
        else if (CurrentFont->GetInfo().Type == FontType::PCScreenFont1)
        {
            uint32_t *PixelPtr = (uint32_t *)Buffers[Index]->Buffer;
            char *FontPtr = (char *)CurrentFont->GetInfo().PSF1Font->GlyphBuffer + (Char * CurrentFont->GetInfo().PSF1Font->Header->charsize);
            for (unsigned long Y = Buffers[Index]->CursorY; Y < Buffers[Index]->CursorY + 16; Y++)
            {
                for (unsigned long X = Buffers[Index]->CursorX; X < Buffers[Index]->CursorX + 8; X++)
                    if ((*FontPtr & (0b10000000 >> (X - Buffers[Index]->CursorX))) > 0)
                        *(unsigned int *)(PixelPtr + X + (Y * Buffers[Index]->Width)) = Buffers[Index]->Color;
                FontPtr++;
            }
            Buffers[Index]->CursorX += 8;
            return Char;
        }
        return Char;
    }

    Display::Display(BootInfo::FramebufferInfo Info, bool LoadDefaultFont)
    {
        framebuffer = Info;
        if (LoadDefaultFont)
        {
            CurrentFont = new Font(&_binary_files_ter_powerline_v12n_psf_start, &_binary_files_ter_powerline_v12n_psf_end, FontType::PCScreenFont2);
            FontInfo Info = CurrentFont->GetInfo();
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
