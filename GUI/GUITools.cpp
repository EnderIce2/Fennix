#include <gui.hpp>

#include "../kernel.h"

namespace GraphicalUserInterface
{
    inline void InlineSetPixel(ScreenBitmap *Bitmap, long X, long Y, uint32_t Color)
    {
        if (unlikely(!Bitmap))
            return;

        if (unlikely(!Bitmap->Data))
            return;

        if (unlikely(X < 0 || Y < 0 || X >= Bitmap->Width || Y >= Bitmap->Height))
            return;

        uint32_t *Pixel = (uint32_t *)((uintptr_t)Bitmap->Data + (Y * Bitmap->Width + X) * (Bitmap->BitsPerPixel / 8));
        *Pixel = Color;
        // Bitmap->Data[Y * Bitmap->Width + X] = Color;
    }

    void SetPixel(ScreenBitmap *Bitmap, long X, long Y, uint32_t Color)
    {
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

    void DrawOverBitmap(ScreenBitmap *DestinationBitmap,
                        ScreenBitmap *SourceBitmap,
                        long Top,
                        long Left, bool IgnoreZero)
    {
        for (uint32_t i = 0; i < SourceBitmap->Width; i++)
            for (uint32_t j = 0; j < SourceBitmap->Height; j++)
            {
                uint32_t *Pixel = (uint32_t *)((uintptr_t)SourceBitmap->Data + (j * SourceBitmap->Width + i) * (SourceBitmap->BitsPerPixel / 8));
                if (IgnoreZero && (*Pixel != 0x000000))
                    InlineSetPixel(DestinationBitmap, Left + i, Top + j, *Pixel);
            }
    }

    void PutRect(ScreenBitmap *Bitmap, Rect rect, uint32_t Color)
    {
        for (uint32_t i = 0; i < rect.Width; i++)
            for (uint32_t j = 0; j < rect.Height; j++)
                InlineSetPixel(Bitmap, rect.Left + i, rect.Top + j, Color);
    }

    void PutBorder(ScreenBitmap *Bitmap, Rect rect, uint32_t Color)
    {
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

    void DrawString(ScreenBitmap *Bitmap, Rect rect, const char *Text, uint32_t Color)
    {
    }
}
