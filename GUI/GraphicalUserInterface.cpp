#include <gui.hpp>
#include <driver.hpp>
#include <task.hpp>
#include <debug.h>

#include "../kernel.h"
#include "../DAPI.hpp"
#include "../Fex.hpp"

namespace GraphicalUserInterface
{
    char CursorArrow[] = {
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 12x19
        1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, //
        1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, //
        1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, //
        1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, //
        1, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, //
        1, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, //
        1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, //
        1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, //
        1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, //
        1, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, //
        1, 2, 2, 2, 1, 2, 2, 1, 0, 0, 0, 0, //
        1, 2, 2, 1, 0, 1, 2, 2, 1, 0, 0, 0, //
        1, 2, 1, 0, 0, 1, 2, 2, 1, 0, 0, 0, //
        1, 1, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0, //
        1, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, //
    };

    char CursorHand[] = {
        0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 17x24
        0, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 1, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 1, 2, 2, 1, 2, 2, 1, 1, 1, 0, 0, 0, 0, //
        0, 0, 0, 0, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 1, 0, 0, //
        0, 0, 0, 0, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 1, 0, //
        0, 0, 0, 0, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, //
        1, 1, 1, 0, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, //
        1, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 1, //
        1, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, //
        0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, //
        0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, //
        0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, //
        0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, //
        0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, //
        0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, //
        0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, //
        0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, //
        0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, //
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
    };

    char CursorWait[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 13x22
        1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, //
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //
        0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, //
        0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, //
        0, 1, 2, 2, 1, 2, 1, 2, 1, 2, 2, 1, 0, //
        0, 1, 2, 2, 2, 1, 2, 1, 2, 2, 2, 1, 0, //
        0, 1, 1, 2, 2, 2, 1, 2, 2, 2, 1, 1, 0, //
        0, 0, 1, 1, 2, 2, 2, 2, 2, 1, 1, 0, 0, //
        0, 0, 0, 1, 1, 2, 1, 2, 1, 1, 0, 0, 0, //
        0, 0, 0, 0, 1, 1, 2, 1, 1, 0, 0, 0, 0, //
        0, 0, 0, 0, 1, 1, 2, 1, 1, 0, 0, 0, 0, //
        0, 0, 0, 1, 1, 2, 2, 2, 1, 1, 0, 0, 0, //
        0, 0, 1, 1, 2, 2, 1, 2, 2, 1, 1, 0, 0, //
        0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, //
        0, 1, 2, 2, 2, 2, 1, 2, 2, 2, 2, 1, 0, //
        0, 1, 2, 2, 2, 1, 2, 1, 2, 2, 2, 1, 0, //
        0, 1, 2, 2, 1, 2, 1, 2, 1, 2, 2, 1, 0, //
        0, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 0, //
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //
        1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, //
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //
    };

    char CursorIBeam[] = {
        1, 1, 1, 0, 1, 1, 1, // 7x17
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        1, 1, 1, 0, 1, 1, 1, //
    };

    char CursorResizeAll[] ={
        0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0, // 23x23
        0,0,0,0,0,0,0,0,0,0,1,2,1,0,0,0,0,0,0,0,0,0,0, //
        0,0,0,0,0,0,0,0,0,1,2,2,2,1,0,0,0,0,0,0,0,0,0, //
        0,0,0,0,0,0,0,0,1,2,2,2,2,2,1,0,0,0,0,0,0,0,0, //
        0,0,0,0,0,0,0,1,1,1,1,2,1,1,1,1,0,0,0,0,0,0,0, //
        0,0,0,0,0,0,0,0,0,0,1,2,1,0,0,0,0,0,0,0,0,0,0, //
        0,0,0,0,0,0,0,0,0,0,1,2,1,0,0,0,0,0,0,0,0,0,0, //
        0,0,0,0,1,0,0,0,0,0,1,2,1,0,0,0,0,0,1,0,0,0,0, //
        0,0,0,1,1,0,0,0,0,0,1,2,1,0,0,0,0,0,1,1,0,0,0, //
        0,0,1,2,1,0,0,0,0,0,1,2,1,0,0,0,0,0,1,2,1,0,0, //
        0,1,2,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,2,1,0, //
        1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1, //
        0,1,2,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,2,1,0, //
        0,0,1,2,1,0,0,0,0,0,1,2,1,0,0,0,0,0,1,2,1,0,0, //
        0,0,0,1,1,0,0,0,0,0,1,2,1,0,0,0,0,0,1,1,0,0,0, //
        0,0,0,0,1,0,0,0,0,0,1,2,1,0,0,0,0,0,1,0,0,0,0, //
        0,0,0,0,0,0,0,0,0,0,1,2,1,0,0,0,0,0,0,0,0,0,0, //
        0,0,0,0,0,0,0,0,0,0,1,2,1,0,0,0,0,0,0,0,0,0,0, //
        0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0, //
        0,0,0,0,0,0,0,0,1,2,2,2,2,2,1,0,0,0,0,0,0,0,0, //
        0,0,0,0,0,0,0,0,0,1,2,2,2,1,0,0,0,0,0,0,0,0,0, //
        0,0,0,0,0,0,0,0,0,0,1,2,1,0,0,0,0,0,0,0,0,0,0, //
        0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0, //
    };

    void GUI::PaintDesktop()
    {
        PutRect(0, 0, Display->GetBuffer(200)->Width, Display->GetBuffer(200)->Height, 0x282828);
    }

    void GUI::PaintWindows()
    {
        foreach (auto wnd in this->Windows)
            wnd->Paint();
    }

    void GUI::PaintCursor()
    {
        KernelCallback callback;
        if (DriverManager->GetDrivers().size() > 0)
        {
            foreach (auto Driver in DriverManager->GetDrivers())
            {
                if (((FexExtended *)((uintptr_t)Driver->Address + EXTENDED_SECTION_ADDRESS))->Driver.Type == FexDriverType::FexDriverType_Input)
                {
                    memset(&callback, 0, sizeof(KernelCallback));
                    callback.Reason = FetchReason;
                    DriverManager->IOCB(Driver->DriverUID, (void *)&callback);
                    // TODO: I think I should check somehow what driver is the one that is mouse and not keyboard
                    Mouse.X = (callback.InputCallback.Mouse.X * Display->GetBuffer(200)->Width) / 0xFFFF;  // VMWARE mouse
                    Mouse.Y = (callback.InputCallback.Mouse.Y * Display->GetBuffer(200)->Height) / 0xFFFF; // VMWARE mouse
                    Mouse.Z = (callback.InputCallback.Mouse.Z);
                    Mouse.Left = callback.InputCallback.Mouse.Buttons.Left;
                    Mouse.Right = callback.InputCallback.Mouse.Buttons.Right;
                    Mouse.Middle = callback.InputCallback.Mouse.Buttons.Middle;
                    break;
                }
            }
        }

        uint32_t CursorColorInner = 0xFFFFFFFF;
        uint32_t CursorColorOuter = 0x00000000;

        switch (this->Cursor)
        {
        case CursorType::Visible:
        {
            CursorVisible = true;
            break;
        }
        case CursorType::Hidden:
        {
            CursorVisible = false;
            break;
        }
        default:
            fixme("Unknown cursor type %d", this->Cursor);
            [[fallthrough]];
        case CursorType::Arrow:
        {
            if (CursorVisible)
                for (int i = 0; i < 19; i++)
                {
                    for (int j = 0; j < 12; j++)
                    {
                        if (CursorArrow[i * 12 + j] == 1)
                        {
                            Display->SetPixel(Mouse.X + j, Mouse.Y + i, CursorColorOuter, 200);
                        }
                        else if (CursorArrow[i * 12 + j] == 2)
                        {
                            Display->SetPixel(Mouse.X + j, Mouse.Y + i, CursorColorInner, 200);
                        }
                    }
                }
            break;
        }
        case CursorType::Hand:
        {
            if (CursorVisible)
                for (int i = 0; i < 24; i++)
                {
                    for (int j = 0; j < 17; j++)
                    {
                        if (CursorHand[i * 17 + j] == 1)
                        {
                            Display->SetPixel(Mouse.X + j, Mouse.Y + i, CursorColorOuter, 200);
                        }
                        else if (CursorHand[i * 17 + j] == 2)
                        {
                            Display->SetPixel(Mouse.X + j, Mouse.Y + i, CursorColorInner, 200);
                        }
                    }
                }
            break;
        }
        case CursorType::Wait:
        {
            if (CursorVisible)
                for (int i = 0; i < 22; i++)
                {
                    for (int j = 0; j < 13; j++)
                    {
                        if (CursorWait[i * 13 + j] == 1)
                        {
                            Display->SetPixel(Mouse.X + j, Mouse.Y + i, CursorColorOuter, 200);
                        }
                        else if (CursorWait[i * 13 + j] == 2)
                        {
                            Display->SetPixel(Mouse.X + j, Mouse.Y + i, CursorColorInner, 200);
                        }
                    }
                }
            break;
        }
        case CursorType::IBeam:
        {
            if (CursorVisible)
                for (int i = 0; i < 22; i++)
                {
                    for (int j = 0; j < 13; j++)
                    {
                        if (CursorIBeam[i * 13 + j] == 1)
                        {
                            Display->SetPixel(Mouse.X + j, Mouse.Y + i, CursorColorOuter, 200);
                        }
                        else if (CursorIBeam[i * 13 + j] == 2)
                        {
                            Display->SetPixel(Mouse.X + j, Mouse.Y + i, CursorColorInner, 200);
                        }
                    }
                }
            break;
        }
        case CursorType::ResizeAll:
        {
            if (CursorVisible)
                for (int i = 0; i < 23; i++)
                {
                    for (int j = 0; j < 23; j++)
                    {
                        if (CursorResizeAll[i * 23 + j] == 1)
                        {
                            Display->SetPixel(Mouse.X + j, Mouse.Y + i, CursorColorOuter, 200);
                        }
                        else if (CursorResizeAll[i * 23 + j] == 2)
                        {
                            Display->SetPixel(Mouse.X + j, Mouse.Y + i, CursorColorInner, 200);
                        }
                    }
                }
            break;
        }
        }

        foreach (auto wnd in this->Windows)
        {
            Event ev;
            ev.Type = EventType::MouseEvent;
            ev.Mouse.X = Mouse.X;
            ev.Mouse.Y = Mouse.Y;
            ev.Mouse.Z = Mouse.Z;
            ev.Mouse.Left = Mouse.Left;
            ev.Mouse.Right = Mouse.Right;
            ev.Mouse.Middle = Mouse.Middle;
            ev.LastMouse.X = LastMouse.X;
            ev.LastMouse.Y = LastMouse.Y;
            ev.LastMouse.Z = LastMouse.Z;
            ev.LastMouse.Left = LastMouse.Left;
            ev.LastMouse.Right = LastMouse.Right;
            ev.LastMouse.Middle = LastMouse.Middle;
            wnd->HandleEvent(&ev);
        }

        LastMouse = Mouse;
    }

    void GUI::Loop()
    {
        while (IsRunning)
        {
            PaintDesktop();
            PaintWindows();
            PaintCursor();
            Display->SetBuffer(200);
        }
    }

    void GUI::AddWindow(Window *window)
    {
        this->Windows.push_back(window);
    }

    GUI::GUI()
    {
        this->mem = new Memory::MemMgr;
        Display->CreateBuffer(0, 0, 200);
        this->IsRunning = true;
    }

    GUI::~GUI()
    {
        delete this->mem;
        Display->DeleteBuffer(200);
        for (size_t i = 0; i < this->Windows.size(); i++)
            this->Windows.remove(i);
    }
}
