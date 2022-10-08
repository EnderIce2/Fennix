#ifndef __FENNIX_KERNEL_DEBUGGER_H__
#define __FENNIX_KERNEL_DEBUGGER_H__

#include <types.h>

enum DebugLevel
{
    DebugLevelNone = 0,
    DebugLevelError = 1,
    DebugLevelWarning = 2,
    DebugLevelInfo = 3,
    DebugLevelDebug = 4,
    DebugLevelTrace = 5,
    DebugLevelFixme = 6
};

#ifdef __cplusplus

namespace SysDbg
{
    void Write(DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);
    void WriteLine(DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);
}

#define error(Format, ...) SysDbg::WriteLine(DebugLevelError, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define warn(Format, ...) SysDbg::WriteLine(DebugLevelWarning, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define info(Format, ...) SysDbg::WriteLine(DebugLevelInfo, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define debug(Format, ...) SysDbg::WriteLine(DebugLevelDebug, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define trace(Format, ...) SysDbg::WriteLine(DebugLevelTrace, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define fixme(Format, ...) SysDbg::WriteLine(DebugLevelFixme, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)

#else

void SysDbgWrite(enum DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);
void SysDbgWriteLine(enum DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);

#define error(Format, ...) SysDbgWriteLine(DebugLevelError, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define warn(Format, ...) SysDbgWriteLine(DebugLevelWarning, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define info(Format, ...) SysDbgWriteLine(DebugLevelInfo, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define debug(Format, ...) SysDbgWriteLine(DebugLevelDebug, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define trace(Format, ...) SysDbgWriteLine(DebugLevelTrace, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define fixme(Format, ...) SysDbgWriteLine(DebugLevelFixme, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)

#endif // __cplusplus

#endif // !__FENNIX_KERNEL_DEBUGGER_H__
