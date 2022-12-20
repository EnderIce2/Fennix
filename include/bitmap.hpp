#pragma once
#include <types.h>

class Bitmap
{
public:
    size_t Size;
    uint8_t *Buffer;
    bool operator[](uintptr_t index);
    bool Set(uintptr_t index, bool value);
    bool Get(uintptr_t index);
};
