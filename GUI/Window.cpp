#include <gui.hpp>

#include <driver.hpp>
#include <task.hpp>
#include <printf.h>
#include <debug.h>

#include "../kernel.h"
#include "../DAPI.hpp"
#include "../Fex.hpp"

namespace GraphicalUserInterface
{
    char CloseButton[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
    };

    char MinimizeButton[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
    };

    char MaximizeButtonNormal[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
    };

    char MaximizeButtonMaximized[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
    };

    char ResizeHint[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
    };

    uint32_t CloseButtonFade[] = {
        0x404040,
        0x770000,
        0x990000,
        0xBB0000,
        0xDD0000,
        0xFF0000,
    };

    uint32_t MaximizeMinimizeButtonFade[] = {
        0x404040,
        0x454545,
        0x505050,
        0x5F5F5F,
    };

    void Window::Paint()
    {
        // Window background
        PutRect(this->Left, this->Top + 20, this->Width, this->Height, 0x121212);
        // Title bar
        PutRect(this->Left, this->Top, this->Width, 20, 0x404040);
        // Title bar buttons (close, minimize, maximize) on the right
        static int CloseButtonIteration = 0;
        static int MaximizeButtonIteration = 0;
        static int MinimizeButtonIteration = 0;
        if (CloseButtonFocused)
        {
            PutRect(this->Left + this->Width - 20, this->Top, 20, 20, CloseButtonFade[CloseButtonIteration]);
            if (CloseButtonIteration < 5)
                CloseButtonIteration++;
        }
        else
        {
            PutRect(this->Left + this->Width - 20, this->Top, 20, 20, CloseButtonFade[CloseButtonIteration]);
            if (CloseButtonIteration > 0)
                CloseButtonIteration--;
        }

        if (MaximizeButtonFocused)
        {
            PutRect(this->Left + this->Width - 40, this->Top, 20, 20, MaximizeMinimizeButtonFade[MaximizeButtonIteration]);
            if (MaximizeButtonIteration < 3)
                MaximizeButtonIteration++;
        }
        else
        {
            PutRect(this->Left + this->Width - 40, this->Top, 20, 20, MaximizeMinimizeButtonFade[MaximizeButtonIteration]);
            if (MaximizeButtonIteration > 0)
                MaximizeButtonIteration--;
        }

        if (MinimizeButtonFocused)
        {
            PutRect(this->Left + this->Width - 60, this->Top, 20, 20, MaximizeMinimizeButtonFade[MinimizeButtonIteration]);
            if (MinimizeButtonIteration < 3)
                MinimizeButtonIteration++;
        }
        else
        {
            PutRect(this->Left + this->Width - 60, this->Top, 20, 20, MaximizeMinimizeButtonFade[MinimizeButtonIteration]);
            if (MinimizeButtonIteration > 0)
                MinimizeButtonIteration--;
        }

        // Title bar icons (close, minimize, maximize) on the right
        for (int i = 0; i < 20; i++)
        {
            for (int j = 0; j < 20; j++)
            {
                if (CloseButton[i * 20 + j] == 1)
                {
                    Display->SetPixel(this->Left + this->Width - 20 + j, this->Top + i, 0xFFFFFF, 200);
                }
                if ((MaximizeButtonMaximized[i * 20 + j] == 1) && !this->Maximized)
                {
                    Display->SetPixel(this->Left + this->Width - 40 + j, this->Top + i, 0xFFFFFF, 200);
                }
                else if ((MaximizeButtonNormal[i * 20 + j] == 1) && this->Maximized)
                {
                    Display->SetPixel(this->Left + this->Width - 40 + j, this->Top + i, 0xFFFFFF, 200);
                }
                if (MinimizeButton[i * 20 + j] == 1)
                {
                    Display->SetPixel(this->Left + this->Width - 60 + j, this->Top + i, 0xFFFFFF, 200);
                }
            }
        }

        // Resize hint
        for (int i = 0; i < 20; i++)
        {
            for (int j = 0; j < 20; j++)
            {
                if (ResizeHint[i * 20 + j] == 1)
                {
                    Display->SetPixel(this->Left + this->Width - 20 + j, this->Top + this->Height + i, 0xFFFFFF, 200);
                }
            }
        }

        // Title bar border
        PutBorder(this->Left, this->Top, this->Width, 20, 0x000000);
        // Window border
        PutBorder(this->Left, this->Top + 20, this->Width, this->Height, 0x000000);
        // Title bar text
        Display->DrawString(this->Title, this->Left + 5, this->Top + 5, 200);

        // Window content
        foreach (auto var in this->Widgets)
        {
            var->Paint();
        }

        if (!this->Maximized)
        {
            char buf[100];
            sprintf_(buf, "Left:\eAA11FF%ld\eFFFFFF Top:\eAA11FF%ld\eFFFFFF W:\eAA11FF%ld\eFFFFFF H:\eAA11FF%ld\eFFFFFF", this->Left, this->Top, this->Width, this->Height);
            Display->DrawString(buf, this->Left + 20, this->Top + 25, 200);
        }
    }

    void Window::Close()
    {
        fixme("Window::Close() not implemented");
    }

    void Window::Maximize()
    {
        if (!Maximized)
        {
            this->Last_Left = this->Left;
            this->Last_Top = this->Top;
            this->Last_Width = this->Width;
            this->Last_Height = this->Height;
            this->Left = 0;
            this->Top = 0;
            this->Width = Display->GetBuffer(200)->Width;
            this->Height = Display->GetBuffer(200)->Height;
            Maximized = true;
        }
        else
        {
            this->Left = this->Last_Left;
            this->Top = this->Last_Top;
            this->Width = this->Last_Width;
            this->Height = this->Last_Height;
            Maximized = false;
        }
    }

    void Window::Minimize()
    {
        fixme("Window::Minimize() not implemented");
    }

    void Window::HandleEvent(Event *e)
    {
        if (e->Type == EventType::MouseEvent)
        {
            // close button
            if (!WindowDragging &&
                e->Mouse.X >= this->Left + this->Width - 20 &&
                e->Mouse.X <= this->Left + this->Width &&
                e->Mouse.Y >= this->Top &&
                e->Mouse.Y <= this->Top + 20)
            {
                if (e->Mouse.Left)
                {
                    this->Close();
                }
                CloseButtonFocused = true;
            }
            else
                CloseButtonFocused = false;

            // maximize button
            if (!WindowDragging &&
                e->Mouse.X >= this->Left + this->Width - 40 &&
                e->Mouse.X <= this->Left + this->Width - 20 &&
                e->Mouse.Y >= this->Top &&
                e->Mouse.Y <= this->Top + 20)
            {
                if (e->Mouse.Left)
                {
                    this->Maximize();
                }
                MaximizeButtonFocused = true;
            }
            else
                MaximizeButtonFocused = false;

            // minimize button
            if (!WindowDragging &&
                e->Mouse.X >= this->Left + this->Width - 60 &&
                e->Mouse.X <= this->Left + this->Width - 40 &&
                e->Mouse.Y >= this->Top &&
                e->Mouse.Y <= this->Top + 20)
            {
                if (e->Mouse.Left)
                {
                    this->Minimize();
                }
                MinimizeButtonFocused = true;
            }
            else
                MinimizeButtonFocused = false;

            // window dragging
            if (e->Mouse.X >= this->Left &&
                e->Mouse.X <= this->Left + this->Width &&
                e->Mouse.Y >= this->Top &&
                e->Mouse.Y <= this->Top + 20)
            {
                if (e->Mouse.Left &&
                    !Maximized)
                {
                    long X = (long)this->Left + (long)e->Mouse.X - (long)e->LastMouse.X;
                    long Y = (long)this->Top + (long)e->Mouse.Y - (long)e->LastMouse.Y;

                    if (X < 0)
                        X = 0;
                    else if (X + this->Width > Display->GetBuffer(200)->Width)
                        X = Display->GetBuffer(200)->Width - this->Width;

                    if (Y < 0)
                        Y = 0;
                    else if (Y + this->Height + 20 > Display->GetBuffer(200)->Height)
                        Y = Display->GetBuffer(200)->Height - this->Height - 20;

                    this->Left = X;
                    this->Top = Y;
                    WindowDragging = true;
                }
                else
                    WindowDragging = false;
            }

            // resize
            if (!WindowDragging &&
                e->Mouse.X >= this->Left + this->Width - 20 &&
                e->Mouse.X <= this->Left + this->Width &&
                e->Mouse.Y >= this->Top + this->Height &&
                e->Mouse.Y <= this->Top + this->Height + 20)
            {
                if (e->Mouse.Left)
                {
                    long X = this->Width + (long)e->Mouse.X - (long)e->LastMouse.X;
                    long Y = this->Height + (long)e->Mouse.Y - (long)e->LastMouse.Y;
                    if (X < 250)
                        X = 250;
                    if (Y < 150)
                        Y = 150;
                    this->Width = X;
                    this->Height = Y;
                }
                ((GUI *)this->ParentGUI)->SetCursorType(CursorType::ResizeAll);
            }
            else
            {
                ((GUI *)this->ParentGUI)->SetCursorType(CursorType::Arrow);
            }
        }
    }

    void Window::AddWidget(Widget *widget)
    {
        this->Widgets.push_back(widget);
    }

    Window::Window(void *ParentGUI, uint32_t Left, uint32_t Top, uint32_t Width, uint32_t Height, const char *Title)
    {
        this->mem = new Memory::MemMgr;
        this->ParentGUI = ParentGUI;
        this->Left = Left;
        this->Top = Top;
        this->Width = Width;
        this->Height = Height;
        strcpy(this->Title, Title);
        this->Maximized = false;
        this->Minimized = false;
        this->CloseButtonFocused = false;
        this->MaximizeButtonFocused = false;
        this->MinimizeButtonFocused = false;
        this->WindowDragging = false;
    }

    Window::~Window()
    {
        delete this->mem;
    }
}
