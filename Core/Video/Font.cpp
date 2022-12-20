#include <display.hpp>
#include <debug.h>
#include <cstring>

namespace Video
{
    Font::Font(uintptr_t *Start, uintptr_t *End, FontType Type)
    {
        trace("Initializing font with start %#llx and end %#llx Type: %d", Start, End, Type);
        this->Info.StartAddress = Start;
        this->Info.EndAddress = End;
        this->Info.Type = Type;
        if (Type == FontType::PCScreenFont2)
        {
            this->Info.PSF2Font = new PSF2_FONT;

            uintptr_t FontDataLength = End - Start;
            PSF2_HEADER *font2 = (PSF2_HEADER *)KernelAllocator.RequestPages(FontDataLength / PAGE_SIZE + 1);
            for (uintptr_t i = 0; i < FontDataLength / PAGE_SIZE + 1; i++)
                Memory::Virtual().Map((void *)(font2 + (i * PAGE_SIZE)), (void *)(font2 + (i * PAGE_SIZE)), Memory::PTFlag::RW);
            memcpy((void *)font2, Start, FontDataLength);

            this->Info.Width = font2->width;
            this->Info.Height = font2->height;
            if (font2->magic[0] != PSF2_MAGIC0 || font2->magic[1] != PSF2_MAGIC1 || font2->magic[2] != PSF2_MAGIC2 || font2->magic[3] != PSF2_MAGIC3)
                error("Font2 magic mismatch.");

            this->Info.PSF2Font->Header = font2;
            this->Info.PSF2Font->GlyphBuffer = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(Start) + sizeof(PSF2_HEADER));
        }
        else if (Type == FontType::PCScreenFont1)
        {
            this->Info.PSF1Font = new PSF1_FONT;
            PSF1_HEADER *font1 = (PSF1_HEADER *)Start;
            if (font1->magic[0] != PSF1_MAGIC0 || font1->magic[1] != PSF1_MAGIC1)
                error("Font1 magic mismatch.");
            uint32_t glyphBufferSize = font1->charsize * 256;
            if (font1->mode == 1) // 512 glyph mode
                glyphBufferSize = font1->charsize * 512;
            void *glyphBuffer = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(Start) + sizeof(PSF1_HEADER));
            this->Info.PSF1Font->Header = font1;
            this->Info.PSF1Font->GlyphBuffer = glyphBuffer;
            UNUSED(glyphBufferSize); // TODO: Use this in the future?

            // TODO: Get font size.
            this->Info.Width = 16;
            this->Info.Height = 8;
        }
    }

    Font::~Font()
    {
    }
}
