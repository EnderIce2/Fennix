#ifndef __FENNIX_KERNEL_GUI_H__
#define __FENNIX_KERNEL_GUI_H__

#include <types.h>
#include <memory.hpp>
#include <vector.hpp>

namespace GraphicalUserInterface
{
    typedef uintptr_t Handle;

    struct MouseData
    {
        uint32_t X;
        uint32_t Y;
        uint32_t Z;
        bool Left;
        bool Right;
        bool Middle;
    };

    enum CursorType
    {
        Visible = 0,
        Hidden,
        Arrow,
        Hand,
        Wait,
        IBeam,
        ResizeHorizontal,
        ResizeVertical,
        ResizeDiagonalLeft,
        ResizeDiagonalRight,
        ResizeAll,
        Cross,
        Help,
        No,
        AppStarting,
    };

    enum EventType
    {
        MouseEvent,
        KeyboardEvent,
        FocusEvent,
        WidgetEvent
    };

    struct Event
    {
        EventType Type;
        Handle Source;
        Handle Target;
        MouseData Mouse;
        MouseData LastMouse;

        struct
        {
            bool MouseMove;
            bool MouseClick;
            bool MouseDoubleClick;
            bool MouseDown;
            bool MouseUp;
            bool MouseWheel;
            bool MouseEnter;
            bool MouseLeave;
            bool MouseHover;
            bool MouseDrag;
            bool MouseDragStart;
            bool MouseDragEnd;
            bool MouseDragEnter;
            bool MouseDragLeave;
            bool MouseDragHover;
            bool MouseDragDrop;
            bool MouseDragDropEnter;
            bool MouseDragDropLeave;
            bool MouseDragDropHover;
            bool MouseDragDropEnd;
            bool MouseDragDropStart;
            bool MouseDragDropCancel;
            bool MouseDragDropComplete;
            bool MouseDragDropAbort;
        } MouseEventData;

        struct
        {
            bool KeyDown;
            bool KeyUp;
            bool KeyPress;
        } KeyboardEventData;

        struct
        {
            bool FocusEnter;
            bool FocusLeave;
            bool FocusHover;
        } FocusEventData;

        struct
        {
            bool Resize;
            bool Move;
            bool Show;
            bool Hide;
            bool Close;
            bool Destroy;
            bool Paint;
            bool PaintBackground;
            bool PaintForeground;
            bool PaintBorder;
            bool PaintShadow;
            bool PaintOverlay;
            bool PaintAll;
            bool PaintChildren;
            bool PaintChildrenBackground;
            bool PaintChildrenForeground;
            bool PaintChildrenBorder;
            bool PaintChildrenShadow;
            bool PaintChildrenOverlay;
            bool PaintChildrenAll;
        } WidgetEventData;
    };

    void PutRect(uint32_t X, uint32_t Y, uint32_t Width, uint32_t Height, uint32_t Color);
    void PutBorder(uint32_t X, uint32_t Y, uint32_t Width, uint32_t Height, uint32_t Color);
    void PutBorderWithShadow(uint32_t X, uint32_t Y, uint32_t Width, uint32_t Height, uint32_t Color);

    class Widget
    {
    private:
        Memory::MemMgr *mem;

    public:
        void Paint();
        Handle CreatePanel(uint32_t Left, uint32_t Top, uint32_t Width, uint32_t Height, const char *Text);
        Handle CreateButton(uint32_t Left, uint32_t Top, uint32_t Width, uint32_t Height, const char *Text);
        Handle CreateLabel(uint32_t Left, uint32_t Top, uint32_t Width, uint32_t Height, const char *Text);
        Handle CreateTextBox(uint32_t Left, uint32_t Top, uint32_t Width, uint32_t Height, const char *Text);
        Handle CreateCheckBox(uint32_t Left, uint32_t Top, uint32_t Width, uint32_t Height, const char *Text);
        Handle CreateRadioButton(uint32_t Left, uint32_t Top, uint32_t Width, uint32_t Height, const char *Text);
        Handle CreateComboBox(uint32_t Left, uint32_t Top, uint32_t Width, uint32_t Height, const char *Text);
        Handle CreateListBox(uint32_t Left, uint32_t Top, uint32_t Width, uint32_t Height, const char *Text);
        Handle CreateProgressBar(uint32_t Left, uint32_t Top, uint32_t Width, uint32_t Height, const char *Text);
        Handle CreateContextMenu(uint32_t Left, uint32_t Top, uint32_t Width, uint32_t Height, const char *Text);
        void HandleEvent(Event *e);
        Widget(void /* Window */ *ParentWindow);
        ~Widget();
    };

    class Window
    {
    private:
        Memory::MemMgr *mem;
        void *ParentGUI;
        long Left, Top, Width, Height;
        long Last_Left, Last_Top, Last_Width, Last_Height;
        char Title[256];
        Vector<Widget *> Widgets;
        bool Maximized;
        bool Minimized;

        bool CloseButtonFocused;
        bool MaximizeButtonFocused;
        bool MinimizeButtonFocused;
        bool WindowDragging;

    public:
        void Close();
        void Maximize();
        void Minimize();
        void HandleEvent(Event *e);
        void Paint();
        void AddWidget(Widget *widget);
        Window(void *ParentGUI, uint32_t Left, uint32_t Top, uint32_t Width, uint32_t Height, const char *Title);
        ~Window();
    };

    class GUI
    {
    private:
        MouseData Mouse;
        MouseData LastMouse;
        Memory::MemMgr *mem;
        Vector<Window *> Windows;
        bool IsRunning;
        CursorType Cursor = CursorType::Arrow;
        bool CursorVisible = true;

        void PaintDesktop();
        void PaintWindows();
        void PaintCursor();

    public:
        void SetCursorType(CursorType Type = CursorType::Visible) { this->Cursor = Type; }
        void Loop();
        void AddWindow(Window *window);
        GUI();
        ~GUI();
    };
}

#endif // !__FENNIX_KERNEL_GUI_H__
