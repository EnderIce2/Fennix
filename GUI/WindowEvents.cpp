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
        fixme("Window::OnResize() not implemented");
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
        PutRect(this->Buffer, this->Position, /*0x121212*/ 0xFF00FF);
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
        this->OnPaintBackground(e);
        this->OnPaintForeground(e);
    }

    void Window::OnMouseDown(Event *e)
    {
    }

    void Window::OnMouseUp(Event *e)
    {
    }

    void Window::OnMouseMove(Event *e)
    {
    }
}
