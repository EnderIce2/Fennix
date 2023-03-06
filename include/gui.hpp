#ifndef __FENNIX_KERNEL_GUI_H__
#define __FENNIX_KERNEL_GUI_H__

#include <types.h>
#include <display.hpp>
#include <memory.hpp>
#include <vector.hpp>
#include <debug.h>

namespace GraphicalUserInterface
{
    typedef uintptr_t Handle;

    struct MouseData
    {
        int64_t X;
        int64_t Y;
        int64_t Z;
        bool Left;
        bool Right;
        bool Middle;
    };

    struct ScreenBitmap
    {
        int64_t Width;
        int64_t Height;
        uint64_t Size;
        uint64_t Pitch;
        uint64_t BitsPerPixel;
        uint8_t *Data;
    };

    struct Rect
    {
        int64_t Left;
        int64_t Top;
        int64_t Width;
        int64_t Height;

        bool Contains(int64_t X, int64_t Y)
        {
            return (X >= Left && X <= Left + Width && Y >= Top && Y <= Top + Height);
        }

        bool Contains(Rect rect)
        {
            return (rect.Left >= Left && rect.Left + rect.Width <= Left + Width && rect.Top >= Top && rect.Top + rect.Height <= Top + Height);
        }
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

    struct Event
    {
        struct
        {
            int64_t Width;
            int64_t Height;
        } Resize;

        struct
        {
            int64_t X;
            int64_t Y;
            bool Left;
            bool Right;
            bool Middle;
        } MouseDown;

        struct
        {
            int64_t X;
            int64_t Y;
            bool Left;
            bool Right;
            bool Middle;
        } MouseUp;

        struct
        {
            int64_t X;
            int64_t Y;
            bool Left;
            bool Right;
            bool Middle;
        } MouseMove;
    };

    /*
        virtual void OnMouseMove(Event *e) {}
        virtual void OnMouseClick(Event *e) {}
        virtual void OnMouseDoubleClick(Event *e) {}
        virtual void OnMouseDown(Event *e) {}
        virtual void OnMouseUp(Event *e) {}
        virtual void OnMouseWheel(Event *e) {}
        virtual void OnMouseEnter(Event *e) {}
        virtual void OnMouseLeave(Event *e) {}
        virtual void OnMouseHover(Event *e) {}
        virtual void OnMouseDrag(Event *e) {}
        virtual void OnMouseDragStart(Event *e) {}
        virtual void OnMouseDragEnd(Event *e) {}
        virtual void OnMouseDragEnter(Event *e) {}
        virtual void OnMouseDragLeave(Event *e) {}
        virtual void OnMouseDragHover(Event *e) {}
        virtual void OnMouseDragDrop(Event *e) {}
        virtual void OnMouseDragDropEnter(Event *e) {}
        virtual void OnMouseDragDropLeave(Event *e) {}
        virtual void OnMouseDragDropHover(Event *e) {}
        virtual void OnMouseDragDropEnd(Event *e) {}
        virtual void OnMouseDragDropStart(Event *e) {}
        virtual void OnMouseDragDropCancel(Event *e) {}
        virtual void OnMouseDragDropComplete(Event *e) {}
        virtual void OnMouseDragDropAbort(Event *e) {}

        virtual void OnKeyDown(Event *e) {}
        virtual void OnKeyUp(Event *e) {}
        virtual void OnKeyPress(Event *e) {}

        virtual void OnFocusEnter(Event *e) {}
        virtual void OnFocusLeave(Event *e) {}
        virtual void OnFocusHover(Event *e) {}

        virtual void OnResize(Event *e) {}
        virtual void OnMinimize(Event *e) {}
        virtual void OnMaximize(Event *e) {}
        virtual void OnMove(Event *e) {}
        virtual void OnShow(Event *e) {}
        virtual void OnHide(Event *e) {}
        virtual void OnClose(Event *e) {}
        virtual void OnDestroy(Event *e) {}

        virtual void OnPaint(Event *e) {}
        virtual void OnPaintBackground(Event *e) {}
        virtual void OnPaintForeground(Event *e) {}
        virtual void OnPaintOverlay(Event *e) {}
        virtual void OnPaintAll(Event *e) {}
        virtual void OnPaintChildren(Event *e) {}
        virtual void OnPaintChildrenBackground(Event *e) {}
        virtual void OnPaintChildrenForeground(Event *e) {}
        virtual void OnPaintChildrenBorder(Event *e) {}
        virtual void OnPaintChildrenShadow(Event *e) {}
        virtual void OnPaintChildrenOverlay(Event *e) {}
        virtual void OnPaintChildrenAll(Event *e) {}
    */

    void SetPixel(ScreenBitmap *Bitmap, long X, long Y, uint32_t Color);
    void DrawOverBitmap(ScreenBitmap *DestinationBitmap,
                        ScreenBitmap *SourceBitmap,
                        long Top,
                        long Left,
                        bool IgnoreZero = true);
    void PutRect(ScreenBitmap *Bitmap, Rect rect, uint32_t Color);
    void PutBorder(ScreenBitmap *Bitmap, Rect rect, uint32_t Color);
    uint32_t BlendColors(uint32_t c1, uint32_t c2, float t);
    void PutBorderWithShadow(ScreenBitmap *Bitmap, Rect rect, uint32_t Color);
    void DrawShadow(ScreenBitmap *Bitmap, Rect rect);
    void PaintChar(Video::Font *font, ScreenBitmap *Bitmap, char c, uint32_t Color, long *CharCursorX, long *CharCursorY);
    void DrawString(ScreenBitmap *Bitmap, Rect rect, const char *Text, uint32_t Color);

    class WidgetCollection
    {
    private:
        Memory::MemMgr *mem;
        ScreenBitmap *Buffer;
        Video::Font *CurrentFont;
        void *ParentWindow;
        bool NeedRedraw;

        struct HandleMeta
        {
            char Type[4];
        };

        struct LabelObject
        {
            HandleMeta Handle;
            Rect rect;
            char Text[512];
            uint32_t Color;
            long CharCursorX, CharCursorY;
        };

        struct PanelObject
        {
            HandleMeta Handle;
            Rect rect;
            uint32_t Color;
            uint32_t BorderColor;
            uint32_t ShadowColor;
            bool Shadow;
        };

        struct ButtonObject
        {
            HandleMeta Handle;
            Rect rect;
            char Text[512];
            uint32_t Color;
            uint32_t HoverColor;
            uint32_t PressedColor;
            uint32_t BorderColor;
            uint32_t ShadowColor;
            long CharCursorX, CharCursorY;
            bool Shadow;
            bool Hover;
            bool Pressed;
            uintptr_t OnClick;
        };

        Vector<LabelObject *> Labels;
        Vector<PanelObject *> Panels;
        Vector<ButtonObject *> Buttons;

    public:
        Handle CreatePanel(Rect rect, uint32_t Color);
        Handle CreateButton(Rect rect, const char *Text, uintptr_t OnClick = (uintptr_t) nullptr);
        Handle CreateLabel(Rect rect, const char *Text);
        Handle CreateTextBox(Rect rect, const char *Text);
        Handle CreateCheckBox(Rect rect, const char *Text);
        Handle CreateRadioButton(Rect rect, const char *Text);
        Handle CreateComboBox(Rect rect, const char *Text);
        Handle CreateListBox(Rect rect, const char *Text);
        Handle CreateProgressBar(Rect rect, const char *Text);
        Handle CreateContextMenu(Rect rect, const char *Text);

        void SetText(Handle handle, const char *Text);

        WidgetCollection(void /* Window */ *ParentWindow);
        ~WidgetCollection();

        void OnMouseMove(Event *e);
        void OnMouseClick(Event *e);
        void OnMouseDoubleClick(Event *e);
        void OnMouseDown(Event *e);
        void OnMouseUp(Event *e);
        void OnMouseWheel(Event *e);
        void OnMouseEnter(Event *e);
        void OnMouseLeave(Event *e);
        void OnMouseHover(Event *e);
        void OnMouseDrag(Event *e);
        void OnMouseDragStart(Event *e);
        void OnMouseDragEnd(Event *e);

        void OnKeyDown(Event *e);
        void OnKeyUp(Event *e);
        void OnKeyPress(Event *e);

        void OnShow(Event *e);
        void OnHide(Event *e);
        void OnDestroy(Event *e);

        void OnPaint(Event *e);
        void OnPaintBackground(Event *e);
        void OnPaintForeground(Event *e);
    };

    class Window
    {
    private:
        Memory::MemMgr *mem;
        ScreenBitmap *Buffer;
        Rect Position;
        Rect LastPosition;
        char Title[256];
        Vector<WidgetCollection *> Widgets;
        void *ParentGUI;

        bool Maximized;
        bool Minimized;

    public:
        bool IsMaximized() { return Maximized; }
        bool IsMinimized() { return Minimized; }
        ScreenBitmap *GetBuffer() { return Buffer; }
        Rect GetPosition() { return Position; }
        Rect *GetPositionPtr() { return &Position; }
        const char *GetTitle() { return (const char *)Title; }
        void SetTitle(const char *Title) { strcpy(this->Title, Title); }
        void AddWidget(WidgetCollection *widget);

        Window(void *ParentGUI, Rect rect, const char *Title);
        ~Window();

        void OnMouseMove(Event *e);
        void OnMouseClick(Event *e);
        void OnMouseDoubleClick(Event *e);
        void OnMouseDown(Event *e);
        void OnMouseUp(Event *e);
        void OnMouseWheel(Event *e);
        void OnMouseEnter(Event *e);
        void OnMouseLeave(Event *e);
        void OnMouseHover(Event *e);
        void OnMouseDrag(Event *e);
        void OnMouseDragStart(Event *e);
        void OnMouseDragEnd(Event *e);

        void OnKeyDown(Event *e);
        void OnKeyUp(Event *e);
        void OnKeyPress(Event *e);

        void OnFocusEnter(Event *e);
        void OnFocusLeave(Event *e);
        void OnFocusHover(Event *e);

        void OnResize(Event *e);
        void OnMinimize(Event *e);
        void OnMaximize(Event *e);
        void OnMove(Event *e);
        void OnShow(Event *e);
        void OnHide(Event *e);
        void OnClose(Event *e);
        void OnDestroy(Event *e);

        void OnPaint(Event *e);
        void OnPaintBackground(Event *e);
        void OnPaintForeground(Event *e);
        void OnPaintOverlay(Event *e);
        void OnPaintAll(Event *e);
        void OnPaintChildren(Event *e);
        void OnPaintChildrenBackground(Event *e);
        void OnPaintChildrenForeground(Event *e);
        void OnPaintChildrenBorder(Event *e);
        void OnPaintChildrenShadow(Event *e);
        void OnPaintChildrenOverlay(Event *e);
        void OnPaintChildrenAll(Event *e);
    };

    class GUI
    {
    private:
        MouseData MouseArray[256];
        Memory::MemMgr *mem;
        Video::Font *CurrentFont;
        Rect Desktop;
        ScreenBitmap *BackBuffer;
        ScreenBitmap *DesktopBuffer;
        ScreenBitmap *OverlayBuffer;
        ScreenBitmap *CursorBuffer;
        Vector<WidgetCollection *> Widgets;
        Vector<Window *> Windows;
        CursorType Cursor = CursorType::Arrow;
        CursorType LastCursor = CursorType::Arrow;
        bool CursorVisible = true;
        bool IsRunning = false;

        bool DesktopBufferRepaint = true;
        bool OverlayBufferRepaint = true;
        bool OverlayFullRepaint = true;
        bool CursorBufferRepaint = true;

        void FetchInputs();
        void PaintDesktop();
        void PaintWidgets();
        void PaintWindows();
        void PaintCursor();

    public:
        void SetCursorType(CursorType Type = CursorType::Visible) { this->Cursor = Type; }
        void Loop();
        void AddWindow(Window *window);
        void AddWidget(WidgetCollection *widget);
        GUI();
        ~GUI();
    };
}

#endif // !__FENNIX_KERNEL_GUI_H__
