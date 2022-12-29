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

#define TASKBAR_HEIGHT 25


        // if (!Maximized)
        // {
        //     this->LastPosition.Left = this->Position.Left;
        //     this->LastPosition.Top = this->Position.Top;
        //     this->LastPosition.Width = this->Position.Width;
        //     this->LastPosition.Height = this->Position.Height;
        //     this->Position.Left = 0;
        //     this->Position.Top = 0;
        //     this->Position.Width = Display->GetBuffer(200)->Width;
        //     this->Position.Height = Display->GetBuffer(200)->Height - 20 - TASKBAR_HEIGHT;
        //     Maximized = true;
        // }
        // else
        // {
        //     this->Position.Left = this->LastPosition.Left;
        //     this->Position.Top = this->LastPosition.Top;
        //     this->Position.Width = this->LastPosition.Width;
        //     this->Position.Height = this->LastPosition.Height;
        //     Maximized = false;
        // }

    void Window::AddWidget(WidgetCollection *widget)
    {
        this->Widgets.push_back(widget);
    }

    Window::Window(void *ParentGUI, Rect rect, const char *Title)
    {
        this->mem = new Memory::MemMgr;
        this->Buffer = new ScreenBitmap;
        this->Buffer->Width = rect.Width;
        this->Buffer->Height = rect.Height;
        this->Buffer->BitsPerPixel = Display->GetBitsPerPixel();
        this->Buffer->Pitch = Display->GetPitch();
        this->Buffer->Size = this->Buffer->Pitch * rect.Height;
        this->Buffer->Data = (uint8_t*)this->mem->RequestPages(TO_PAGES(this->Buffer->Size));
        memset(this->Buffer->Data, 0, this->Buffer->Size);
        this->ParentGUI = ParentGUI;
        this->Position = rect;
        strcpy(this->Title, Title);
        this->Maximized = false;
        this->Minimized = false;
    }

    Window::~Window()
    {
        delete this->mem;
    }
}
