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
    void Window::OnResize(Event *e)
    {
        // TODO: Optimize this
        this->mem->FreePages(this->Buffer->Data, TO_PAGES(this->Buffer->Size));
        delete this->Buffer;

        this->Buffer = new ScreenBitmap;
        this->Buffer->Width = e->Resize.Width;
        this->Buffer->Height = e->Resize.Height;
        this->Buffer->BitsPerPixel = Display->GetBitsPerPixel();
        this->Buffer->Pitch = Display->GetPitch();
        this->Buffer->Size = this->Buffer->Pitch * e->Resize.Height;
        this->Buffer->Data = (uint8_t *)this->mem->RequestPages(TO_PAGES(this->Buffer->Size));
        memset(this->Buffer->Data, 0, this->Buffer->Size);
        this->OnPaint(e);
    }

    void Window::OnMinimize(Event *e)
    {
        fixme("Window::OnMinimize() not implemented");
    }

    void Window::OnMaximize(Event *e)
    {
        fixme("Window::OnMaximize() not implemented");
    }

    void Window::OnClose(Event *e)
    {
        fixme("Window::OnClose() not implemented");
    }

    void Window::OnPaintBackground(Event *e)
    {
        Rect PaintPosition = this->Position;
        PaintPosition.Left = 0;
        PaintPosition.Top = 0;
        PutRect(this->Buffer, PaintPosition, 0x121212);
    }

    void Window::OnPaintForeground(Event *e)
    {
        // Window content
        if (!this->Maximized)
        {
            char buf[256];
            sprintf_(buf, "Left:\eAA11FF%ld\eFFFFFF Top:\eAA11FF%ld\eFFFFFF W:\eAA11FF%ld\eFFFFFF H:\eAA11FF%ld\eFFFFFF", this->Position.Left, this->Position.Top, this->Position.Width, this->Position.Height);
            // Display->DrawString(buf, this->Position.Left + 20, this->Position.Top + 25, 200);
        }

        foreach (auto var in this->Widgets)
        {
            var->OnPaint(e);
        }
    }

    void Window::OnPaint(Event *e)
    {
        memset(this->Buffer->Data, 0, this->Buffer->Size);
        this->OnPaintBackground(e);
        this->OnPaintForeground(e);
    }

    void Window::OnMouseDown(Event *e)
    {
        Event WindowPos = *e;

        WindowPos.MouseDown.X -= this->Position.Left;
        WindowPos.MouseDown.Y -= this->Position.Top;

        foreach (auto var in this->Widgets)
        {
            var->OnMouseDown(&WindowPos);
        }
    }

    void Window::OnMouseUp(Event *e)
    {
        Event WindowPos = *e;

        WindowPos.MouseUp.X -= this->Position.Left;
        WindowPos.MouseUp.Y -= this->Position.Top;

        foreach (auto var in this->Widgets)
        {
            var->OnMouseUp(&WindowPos);
        }
    }

    void Window::OnMouseMove(Event *e)
    {
        Event WindowPos = *e;

        WindowPos.MouseMove.X -= this->Position.Left;
        WindowPos.MouseMove.Y -= this->Position.Top;

        foreach (auto var in this->Widgets)
        {
            var->OnMouseMove(&WindowPos);
        }
    }
}
