#pragma once

#ifndef LYNX_BITMAP_TYPES_H
#define LYNX_BITMAP_TYPES_H

typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __UINT64_TYPE__ uint64_t;
typedef __SIZE_TYPE__ size_t;
typedef __UINTPTR_TYPE__ uintptr_t;
#ifndef NULL
#define NULL ((void *)0)
#endif

#define ALIGN_UP(x, align) ((__typeof__(x))(((uint64_t)(x) + ((align)-1)) & (~((align)-1))))
#define ALIGN_DOWN(x, align) ((__typeof__(x))((x) & (~((align)-1))))

#endif // !LYNX_BITMAP_TYPES_H

#ifdef __cplusplus

class Bitmap
{
public:
    size_t Size;
    uint8_t *Buffer;
    bool operator[](uint64_t index);
    bool Set(uint64_t index, bool value);
    bool Get(uint64_t index);
};

#endif // __cplusplus
