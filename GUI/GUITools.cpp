#include <gui.hpp>

#include "../kernel.h"

namespace GraphicalUserInterface
{
    Ofast inline void InlineSetPixel(ScreenBitmap *Bitmap, long X, long Y, uint32_t Color)
    {
        if (unlikely(X < 0 || Y < 0 || X >= Bitmap->Width || Y >= Bitmap->Height))
            return;

        uint32_t *Pixel = (uint32_t *)((uintptr_t)Bitmap->Data + (Y * Bitmap->Width + X) * (Bitmap->BitsPerPixel / 8));
        *Pixel = Color;
        // Bitmap->Data[Y * Bitmap->Width + X] = Color;
    }

    void SetPixel(ScreenBitmap *Bitmap, long X, long Y, uint32_t Color)
    {
        if (unlikely(!Bitmap))
            return;

        if (unlikely(!Bitmap->Data))
            return;

        InlineSetPixel(Bitmap, X, Y, Color);
    }

    uint32_t GetPixel(ScreenBitmap *Bitmap, long X, long Y)
    {
        if (unlikely(!Bitmap))
            return 0;

        if (unlikely(!Bitmap->Data))
            return 0;

        if (unlikely(X < 0 || Y < 0 || X >= Bitmap->Width || Y >= Bitmap->Height))
            return 0;

        uint32_t *Pixel = (uint32_t *)((uintptr_t)Bitmap->Data + (Y * Bitmap->Width + X) * (Bitmap->BitsPerPixel / 8));
        return *Pixel;
    }

    Ofast void DrawOverBitmap(ScreenBitmap *DestinationBitmap,
                              ScreenBitmap *SourceBitmap,
                              long Top,
                              long Left, bool IgnoreZero)
    {
        if (unlikely(!SourceBitmap) || unlikely(!SourceBitmap->Data) ||
            unlikely(!DestinationBitmap) || unlikely(!DestinationBitmap->Data))
            return;

        // for (uint32_t i = 0; i < SourceBitmap->Width; i++)
        //     for (uint32_t j = 0; j < SourceBitmap->Height; j++)
        //     {
        //         uint32_t *Pixel = (uint32_t *)((uintptr_t)SourceBitmap->Data + (j * SourceBitmap->Width + i) * (SourceBitmap->BitsPerPixel / 8));
        //         if (IgnoreZero && (*Pixel != 0x000000))
        //             InlineSetPixel(DestinationBitmap, Left + i, Top + j, *Pixel);
        //     }

        for (uint32_t j = 0; j < SourceBitmap->Height; j++)
        {
            uint32_t *Pixel = (uint32_t *)((uintptr_t)SourceBitmap->Data + j * SourceBitmap->Width * (SourceBitmap->BitsPerPixel / 8));
            if (IgnoreZero)
            {
                for (uint32_t i = 0; i < SourceBitmap->Width; i++)
                {
                    if (Pixel[i] != 0x000000)
                        InlineSetPixel(DestinationBitmap, Left + i, Top + j, Pixel[i]);
                }
            }
            else
            {
                memcpy((void *)((uintptr_t)DestinationBitmap->Data + (Top + j) * DestinationBitmap->Width * (DestinationBitmap->BitsPerPixel / 8) + Left * (DestinationBitmap->BitsPerPixel / 8)),
                       (void *)((uintptr_t)SourceBitmap->Data + j * SourceBitmap->Width * (SourceBitmap->BitsPerPixel / 8)),
                       SourceBitmap->Width * (SourceBitmap->BitsPerPixel / 8));
            }
        }
    }

    Ofast void PutRect(ScreenBitmap *Bitmap, Rect rect, uint32_t Color)
    {
        if (unlikely(!Bitmap))
            return;

        if (unlikely(!Bitmap->Data))
            return;

        for (uint32_t i = 0; i < rect.Width; i++)
            for (uint32_t j = 0; j < rect.Height; j++)
                InlineSetPixel(Bitmap, rect.Left + i, rect.Top + j, Color);
    }

    void PutBorder(ScreenBitmap *Bitmap, Rect rect, uint32_t Color)
    {
        if (unlikely(!Bitmap))
            return;

        if (unlikely(!Bitmap->Data))
            return;

        for (uint32_t i = 0; i < rect.Width; i++)
        {
            InlineSetPixel(Bitmap, rect.Left + i, rect.Top, Color);
            InlineSetPixel(Bitmap, rect.Left + i, rect.Top + rect.Height - 1, Color);
        }
        for (uint32_t i = 0; i < rect.Height; i++)
        {
            InlineSetPixel(Bitmap, rect.Left, rect.Top + i, Color);
            InlineSetPixel(Bitmap, rect.Left + rect.Width - 1, rect.Top + i, Color);
        }
    }

    uint32_t BlendColors(uint32_t c1, uint32_t c2, float t)
    {
        uint8_t r1 = (c1 >> 16) & 0xFF;
        uint8_t g1 = (c1 >> 8) & 0xFF;
        uint8_t b1 = c1 & 0xFF;
        uint8_t r2 = (c2 >> 16) & 0xFF;
        uint8_t g2 = (c2 >> 8) & 0xFF;
        uint8_t b2 = c2 & 0xFF;

        uint8_t r = (uint8_t)(r1 + t * (r2 - r1));
        uint8_t g = (uint8_t)(g1 + t * (g2 - g1));
        uint8_t b = (uint8_t)(b1 + t * (b2 - b1));

        return (r << 16) | (g << 8) | b;
    }

    void PutBorderWithShadow(ScreenBitmap *Bitmap, Rect rect, uint32_t Color)
    {
    }

#define SHADOW_SIZE 12

    void DrawShadow(ScreenBitmap *Bitmap, Rect rect)
    {
    }

    void PaintChar(Video::Font *font, ScreenBitmap *Bitmap, char c, uint32_t Color, long *CharCursorX, long *CharCursorY)
    {
        switch (font->GetInfo().Type)
        {
        case Video::FontType::PCScreenFont1:
        {
            uint32_t *PixelPtr = (uint32_t *)Bitmap->Data;
            char *FontPtr = (char *)font->GetInfo().PSF1Font->GlyphBuffer + (c * font->GetInfo().PSF1Font->Header->charsize);
            for (int64_t Y = *CharCursorY; Y < *CharCursorY + 16; Y++)
            {
                for (int64_t X = *CharCursorX; X < *CharCursorX + 8; X++)
                    if ((*FontPtr & (0b10000000 >> (X - *CharCursorX))) > 0)
                        InlineSetPixel(Bitmap, X, Y, Color);
                FontPtr++;
            }
            *CharCursorX += 8;
            break;
        }
        case Video::FontType::PCScreenFont2:
        {
            // if (font->PSF2Font->GlyphBuffer == (uint16_t *)0x01) // HAS UNICODE TABLE
            //     Char = font->PSF2Font->GlyphBuffer[Char];
            int BytesPerLine = (font->GetInfo().PSF2Font->Header->width + 7) / 8;
            char *FontPtr = (char *)font->GetInfo().StartAddress +
                            font->GetInfo().PSF2Font->Header->headersize +
                            (c > 0 && (unsigned char)c < font->GetInfo().PSF2Font->Header->length ? c : 0) *
                                font->GetInfo().PSF2Font->Header->charsize;

            uint32_t fonthdrWidth = font->GetInfo().PSF2Font->Header->width;
            uint32_t fonthdrHeight = font->GetInfo().PSF2Font->Header->height;

            for (int64_t Y = *CharCursorY; Y < *CharCursorY + fonthdrHeight; Y++)
            {
                for (int64_t X = *CharCursorX; X < *CharCursorX + fonthdrWidth; X++)
                    if ((*FontPtr & (0b10000000 >> (X - *CharCursorX))) > 0)
                        InlineSetPixel(Bitmap, X, Y, Color);
                FontPtr += BytesPerLine;
            }
            *CharCursorX += fonthdrWidth;
            break;
        }
        default:
            warn("Unsupported font type");
            break;
        }
    }

    void DrawString(ScreenBitmap *Bitmap, Rect rect, const char *Text, uint32_t Color)
    {
    }
}
