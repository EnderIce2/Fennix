#include <debug.h>
#include <printf.h>

enum vga_color
{
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

extern "C" void terminal_setcolor(uint8_t color);
extern "C" void putchar(char c);
extern bool DoNotWriteOnScreen;

static inline void WritePrefix(DebugLevel Level, const char *File, int Line, const char *Function)
{
    const char *DbgLvlString;
    switch (Level)
    {
    case DebugLevelError:
        DbgLvlString = "ERROR";
        terminal_setcolor(VGA_COLOR_RED);
        break;
    case DebugLevelWarning:
        DbgLvlString = "WARN ";
        terminal_setcolor(VGA_COLOR_BROWN);
        break;
    case DebugLevelInfo:
        DbgLvlString = "INFO ";
        terminal_setcolor(VGA_COLOR_LIGHT_GREY);
        break;
    case DebugLevelDebug:
        DbgLvlString = "DEBUG";
        terminal_setcolor(VGA_COLOR_GREEN);
        break;
    case DebugLevelTrace:
        DbgLvlString = "TRACE";
        terminal_setcolor(VGA_COLOR_DARK_GREY);
        break;
    case DebugLevelFixme:
        DbgLvlString = "FIXME";
        terminal_setcolor(VGA_COLOR_LIGHT_RED);
        break;
    case DebugLevelUbsan:
    {
        DbgLvlString = "UBSAN";
        printf("%s|%s: ", DbgLvlString, Function);
        terminal_setcolor(VGA_COLOR_LIGHT_RED);
        return;
    }
    default:
        DbgLvlString = "UNKNW";
        terminal_setcolor(VGA_COLOR_WHITE);
        break;
    }
    DoNotWriteOnScreen = true;
    printf("%s|%s->%s:%d: ", DbgLvlString, File, Function, Line);
    DoNotWriteOnScreen = false;
}

namespace SysDbg
{
    void Write(DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...)
    {
        WritePrefix(Level, File, Line, Function);
        va_list args;
        va_start(args, Format);
        vprintf(Format, args);
        va_end(args);
    }

    void WriteLine(DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...)
    {
        WritePrefix(Level, File, Line, Function);
        va_list args;
        va_start(args, Format);
        vprintf(Format, args);
        va_end(args);
        putchar('\n');
    }
}

// C compatibility
extern "C" void SysDbgWrite(enum DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...)
{
    WritePrefix(Level, File, Line, Function);
    va_list args;
    va_start(args, Format);
    vprintf(Format, args);
    va_end(args);
}

// C compatibility
extern "C" void SysDbgWriteLine(enum DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...)
{
    WritePrefix(Level, File, Line, Function);
    va_list args;
    va_start(args, Format);
    vprintf(Format, args);
    va_end(args);
    putchar('\n');
}
