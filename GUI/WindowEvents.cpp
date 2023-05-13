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
        this->mem->FreePages(this->Buffer->Data, TO_PAGES(this->Buffer->Size + 1));
        this->Buffer->Data = nullptr;
        delete this->Buffer, this->Buffer = nullptr;

        this->Buffer = new ScreenBitmap;
        this->Buffer->Width = e->Resize.Width;
        this->Buffer->Height = e->Resize.Height;
        this->Buffer->BitsPerPixel = Display->GetBitsPerPixel();
        this->Buffer->Pitch = Display->GetPitch();
        this->Buffer->Size = this->Buffer->Pitch * e->Resize.Height;
        this->Buffer->Data = (uint8_t *)this->mem->RequestPages(TO_PAGES(this->Buffer->Size + 1));
        memset(this->Buffer->Data, 0, this->Buffer->Size);
        this->OnPaint(e);
    }

    void Window::OnMinimize(Event *e)
    {
        UNUSED(e);
        fixme("Window::OnMinimize() not implemented");
    }

    void Window::OnMaximize(Event *e)
    {
        UNUSED(e);
        fixme("Window::OnMaximize() not implemented");
    }

    void Window::OnClose(Event *e)
    {
        UNUSED(e);
        fixme("Window::OnClose() not implemented");
    }

    void Window::OnPaintBackground(Event *e)
    {
        UNUSED(e);
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
            sprintf(buf, "Left:\eAA11FF%lld\eFFFFFF Top:\eAA11FF%lld\eFFFFFF W:\eAA11FF%ld\eFFFFFF H:\eAA11FF%ld\eFFFFFF", this->Position.Left, this->Position.Top, this->Position.Width, this->Position.Height);
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
