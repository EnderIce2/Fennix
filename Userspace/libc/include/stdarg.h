#ifndef _STDARG_H
#define _STDARG_H

#ifndef __va_list__
typedef __builtin_va_list va_list;
#endif

#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_copy(dest, src) __builtin_va_copy(dest, src)

#endif // !_STDARG_H
