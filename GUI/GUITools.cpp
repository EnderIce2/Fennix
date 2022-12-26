#include <gui.hpp>

#include "../kernel.h"

namespace GraphicalUserInterface
{
    void PutRect(uint32_t X, uint32_t Y, uint32_t Width, uint32_t Height, uint32_t Color)
    {
        for (uint32_t i = 0; i < Width; i++)
            for (uint32_t j = 0; j < Height; j++)
                Display->SetPixel(X + i, Y + j, Color, 200);
    }

    void PutBorder(uint32_t X, uint32_t Y, uint32_t Width, uint32_t Height, uint32_t Color)
    {
        for (uint32_t i = 0; i < Width; i++)
        {
            Display->SetPixel(X + i, Y, Color, 200);
            Display->SetPixel(X + i, Y + Height - 1, Color, 200);
        }
        for (uint32_t i = 0; i < Height; i++)
        {
            Display->SetPixel(X, Y + i, Color, 200);
            Display->SetPixel(X + Width - 1, Y + i, Color, 200);
        }
    }

    void PutBorderWithShadow(uint32_t X, uint32_t Y, uint32_t Width, uint32_t Height, uint32_t Color)
    {
        uint32_t ShadowColor = (Color ? (Color / 2) : 0x000000);
        for (uint32_t i = 0; i < Width; i++)
        {
            Display->SetPixel(X + i, Y, Color, 200);
            Display->SetPixel(X + i, Y + Height - 1, Color, 200);
        }
        for (uint32_t i = 0; i < Height; i++)
        {
            Display->SetPixel(X, Y + i, Color, 200);
            Display->SetPixel(X + Width - 1, Y + i, Color, 200);
        }
        for (uint32_t i = 0; i < Width; i++)
        {
            Display->SetPixel(X + i, Y + 1, ShadowColor, 200);
            Display->SetPixel(X + i, Y + Height - 2, ShadowColor, 200);
        }
        for (uint32_t i = 0; i < Height; i++)
        {
            Display->SetPixel(X + 1, Y + i, ShadowColor, 200);
            Display->SetPixel(X + Width - 2, Y + i, ShadowColor, 200);
        }
    }
}
