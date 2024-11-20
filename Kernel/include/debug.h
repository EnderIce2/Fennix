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
    DebugLevelFixme = 6,
    DebugLevelUbsan = 7
};

#ifdef __cplusplus

namespace SysDbg
{
    void Write(DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);
    void WriteLine(DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);
    void LockedWrite(DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);
    void LockedWriteLine(DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);
}

#define error(Format, ...) SysDbg::WriteLine(DebugLevelError, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define warn(Format, ...) SysDbg::WriteLine(DebugLevelWarning, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define info(Format, ...) SysDbg::WriteLine(DebugLevelInfo, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#ifdef DEBUG
#define debug(Format, ...) SysDbg::WriteLine(DebugLevelDebug, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define ubsan(Format, ...) SysDbg::WriteLine(DebugLevelUbsan, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#else
#define debug(Format, ...)
#define ubsan(Format, ...)
#endif
#define trace(Format, ...) SysDbg::WriteLine(DebugLevelTrace, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define fixme(Format, ...) SysDbg::WriteLine(DebugLevelFixme, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)

#define locked_error(Format, ...) SysDbg::LockedWriteLine(DebugLevelError, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define locked_warn(Format, ...) SysDbg::LockedWriteLine(DebugLevelWarning, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define locked_info(Format, ...) SysDbg::LockedWriteLine(DebugLevelInfo, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#ifdef DEBUG
#define locked_debug(Format, ...) SysDbg::LockedWriteLine(DebugLevelDebug, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define locked_ubsan(Format, ...) SysDbg::LockedWriteLine(DebugLevelUbsan, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#else
#define locked_debug(Format, ...)
#define locked_ubsan(Format, ...)
#endif
#define locked_trace(Format, ...) SysDbg::LockedWriteLine(DebugLevelTrace, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define locked_fixme(Format, ...) SysDbg::LockedWriteLine(DebugLevelFixme, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)

#else

void SysDbgWrite(enum DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);
void SysDbgWriteLine(enum DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);
void SysDbgLockedWrite(enum DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);
void SysDbgLockedWriteLine(enum DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);

#define error(Format, ...) SysDbgWriteLine(DebugLevelError, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define warn(Format, ...) SysDbgWriteLine(DebugLevelWarning, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define info(Format, ...) SysDbgWriteLine(DebugLevelInfo, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#ifdef DEBUG
#define debug(Format, ...) SysDbgWriteLine(DebugLevelDebug, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define ubsan(Format, ...) SysDbgWriteLine(DebugLevelUbsan, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#else
#define debug(Format, ...)
#define ubsan(Format, ...)
#endif
#define trace(Format, ...) SysDbgWriteLine(DebugLevelTrace, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define fixme(Format, ...) SysDbgWriteLine(DebugLevelFixme, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)

#define locked_error(Format, ...) SysDbgLockedWriteLine(DebugLevelError, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define locked_warn(Format, ...) SysDbgLockedWriteLine(DebugLevelWarning, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define locked_info(Format, ...) SysDbgLockedWriteLine(DebugLevelInfo, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#ifdef DEBUG
#define locked_debug(Format, ...) SysDbgLockedWriteLine(DebugLevelDebug, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define locked_ubsan(Format, ...) SysDbgLockedWriteLine(DebugLevelUbsan, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#else
#define locked_debug(Format, ...)
#define locked_ubsan(Format, ...)
#endif
#define locked_trace(Format, ...) SysDbgLockedWriteLine(DebugLevelTrace, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define locked_fixme(Format, ...) SysDbgLockedWriteLine(DebugLevelFixme, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)

#endif // __cplusplus

#endif // !__FENNIX_KERNEL_DEBUGGER_H__
