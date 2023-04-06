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
        size_t FontDataLength = End - Start;

        if (Type == FontType::PCScreenFont2)
        {
            this->Info.PSF2Font = new PSF2_FONT;

            PSF2_HEADER *font2 = (PSF2_HEADER *)KernelAllocator.RequestPages(FontDataLength / PAGE_SIZE + 1);
            memcpy((void *)font2, Start, FontDataLength);

            Memory::Virtual().Map((void *)font2, (void *)font2, FontDataLength, Memory::PTFlag::RW);

            if (font2->magic[0] != PSF2_MAGIC0 || font2->magic[1] != PSF2_MAGIC1 || font2->magic[2] != PSF2_MAGIC2 || font2->magic[3] != PSF2_MAGIC3)
            {
                error("Font2 magic mismatch.");
                KernelAllocator.FreePages((void *)font2, FontDataLength / PAGE_SIZE + 1);
                return;
            }

            this->Info.PSF2Font->Header = font2;
            this->Info.PSF2Font->GlyphBuffer = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(Start) + sizeof(PSF2_HEADER));
            this->Info.Width = font2->width;
            this->Info.Height = font2->height;
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
