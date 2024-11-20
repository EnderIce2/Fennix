#include <gui.hpp>

#include <driver.hpp>
#include <task.hpp>
#include <printf.h>
#include <debug.h>

#include "../kernel.h"
#include "../DAPI.hpp"
#include "../Fex.hpp"

extern uintptr_t _binary_Files_tamsyn_font_1_11_Tamsyn10x20r_psf_start;
extern uintptr_t _binary_Files_tamsyn_font_1_11_Tamsyn10x20r_psf_end;
extern uintptr_t _binary_Files_tamsyn_font_1_11_Tamsyn10x20r_psf_size;

namespace GraphicalUserInterface
{
    Handle WidgetCollection::CreatePanel(Rect rect, uint32_t Color)
    {
        PanelObject *panel = (PanelObject *)mem->RequestPages(TO_PAGES(sizeof(PanelObject)));

        panel->Handle.Type[0] = 'P';
        panel->Handle.Type[1] = 'N';
        panel->Handle.Type[2] = 'L';
        panel->Handle.Type[3] = '\0';

        panel->rect = rect;
        panel->Color = Color;
        panel->BorderColor = 0xFF000000;
        panel->ShadowColor = 0xFF000000;
        panel->Shadow = false;

        Panels.push_back(panel);
        NeedRedraw = true;
        return (Handle)panel;
    }

    Handle WidgetCollection::CreateLabel(Rect rect, const char *Text)
    {
        LabelObject *label = (LabelObject *)mem->RequestPages(TO_PAGES(sizeof(LabelObject)));

        label->Handle.Type[0] = 'L';
        label->Handle.Type[1] = 'B';
        label->Handle.Type[2] = 'L';
        label->Handle.Type[3] = '\0';

        label->rect = rect;
        strcpy(label->Text, Text);
        label->Color = 0xFFFFFF;
        label->CharCursorX = rect.Left;
        label->CharCursorY = rect.Top;

        Labels.push_back(label);
        NeedRedraw = true;
        return (Handle)label;
    }

    Handle WidgetCollection::CreateButton(Rect rect, const char *Text, uintptr_t OnClick)
    {
        ButtonObject *button = (ButtonObject *)mem->RequestPages(TO_PAGES(sizeof(ButtonObject)));

        button->Handle.Type[0] = 'B';
        button->Handle.Type[1] = 'T';
        button->Handle.Type[2] = 'N';
        button->Handle.Type[3] = '\0';

        button->rect = rect;
        strcpy(button->Text, Text);
        button->Color = 0x252525;
        button->HoverColor = 0x353535;
        button->PressedColor = 0x555555;
        button->BorderColor = 0xFF000000;
        button->ShadowColor = 0xFF000000;
        button->Shadow = false;
        button->Hover = false;
        button->Pressed = false;
        button->OnClick = OnClick;

        Buttons.push_back(button);
        NeedRedraw = true;
        return (Handle)button;
    }

    void WidgetCollection::SetText(Handle handle, const char *Text)
    {
        HandleMeta *meta = (HandleMeta *)handle;
        if (meta->Type[0] == 'L' && meta->Type[1] == 'B' && meta->Type[2] == 'L')
        {
            LabelObject *label = (LabelObject *)handle;
            strcpy(label->Text, Text);
            NeedRedraw = true;
        }
    }

    WidgetCollection::WidgetCollection(void *ParentWindow)
    {
        if (!ParentWindow)
        {
            error("ParentWindow is null");
            return;
        }
        this->ParentWindow = ParentWindow;

        this->mem = new Memory::MemMgr;

        this->Buffer = new ScreenBitmap;
        this->Buffer->Width = ((Window *)this->ParentWindow)->GetPosition().Width;
        this->Buffer->Height = ((Window *)this->ParentWindow)->GetPosition().Height;
        this->Buffer->BitsPerPixel = Display->GetBitsPerPixel();
        this->Buffer->Pitch = Display->GetPitch();
        this->Buffer->Size = this->Buffer->Pitch * ((Window *)this->ParentWindow)->GetPosition().Height;
        this->Buffer->Data = (uint8_t *)this->mem->RequestPages(TO_PAGES(this->Buffer->Size));
        memset(this->Buffer->Data, 0, this->Buffer->Size);

        this->CurrentFont = new Video::Font(&_binary_Files_tamsyn_font_1_11_Tamsyn10x20r_psf_start, &_binary_Files_tamsyn_font_1_11_Tamsyn10x20r_psf_end, Video::FontType::PCScreenFont2);
    }

    WidgetCollection::~WidgetCollection()
    {
        delete this->mem, this->mem = nullptr;
        delete this->Buffer, this->Buffer = nullptr;
        delete this->CurrentFont, this->CurrentFont = nullptr;
    }
}
