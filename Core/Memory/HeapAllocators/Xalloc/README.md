# Xalloc

Xalloc is a custom memory allocator designed for hobby operating systems. It is written in C++ and provides a simple and efficient way to manage memory in your hobby OS.

#### ❗ This project is still in development and is not ready for use in production environments. ❗

---

## Features

- **Simple API** - Xalloc provides a simple API for allocating and freeing memory. It is designed to be easy to use and understand.

- [ ] todo complete this

---

## Getting Started

### Implementing missing functions

You will need to implement the following functions in your OS:

##### Wrapper.cpp
```cpp
extern "C" void *Xalloc_REQUEST_PAGES(Xsize_t Pages)
{
    // ...
}

extern "C" void Xalloc_FREE_PAGES(void *Address, Xsize_t Pages)
{
    // ...
}

extern "C" void Xalloc_MAP_MEMORY(void *VirtualAddress, void *PhysicalAddress, Xsize_t Flags)
{
    // ...
}

extern "C" void Xalloc_UNMAP_MEMORY(void *VirtualAddress)
{
    // ...
}
```

##### Xalloc.hpp
```cpp
#define Xalloc_PAGE_SIZE <page size> /* <-- Replace with your page size */
#define Xalloc_trace(m, ...) <trace function>
#define Xalloc_warn(m, ...) <warning function>
#define Xalloc_err(m, ...) <error function>
#define Xalloc_def <define a lock> /* eg. std::mutex Xalloc_lock; */
#define Xalloc_lock <lock function>
#define Xalloc_unlock <unlock function>
```

### Typical usage

```cpp
#include "Xalloc.hpp"

Xalloc::V1 *XallocV1Allocator = nullptr;

int main()
{
    /*                                            Virtual Base     User   SMAP */
    XallocV1Allocator = new Xalloc::V1((void *)0xFFFFA00000000000, false, false);

    void *p = XallocV1Allocator->malloc(1234);
    /* ... */
    XallocV1Allocator->free(p);
    delete XallocV1Allocator;
    return 0;
}
```

or

```cpp
#include "Xalloc.hpp"

int main()
{
    /*                                      Virtual Base     User   SMAP */
    Xalloc::V1 XallocV1Allocator((void *)0xFFFFA00000000000, false, false);

    void *p = XallocV1Allocator.malloc(1234);
    /* ... */
    XallocV1Allocator.free(p);
    return 0;
}
```

---

## API

### Xalloc::V1

```cpp
void *malloc(Xsize_t Size);
```
Allocates a block of memory of size `Size` bytes.
If `Size` is 0, then `nullptr` is returned.
- `Size` - The size of the block to allocate in bytes.

<br><br>

```cpp
void free(void *Address);
```
Frees the memory block pointed to by `Address`.
If `Address` is `nullptr`, then no operation is performed.
- `Address` - The address of the memory block to free.

<br><br>

```cpp
void *calloc(Xsize_t NumberOfBlocks, Xsize_t Size);
```
Allocates a block of memory for an array of `NumberOfBlocks` elements, each of them `Size` bytes long.
If `NumberOfBlocks` or `Size` is 0, then `nullptr` is returned.
- `NumberOfBlocks` - The number of elements to allocate.
- `Size` - The size of each element in bytes.

<br><br>

```cpp
void *realloc(void *Address, Xsize_t Size);
```
Changes the size of the memory block pointed to by `Address` to `Size` bytes.
If `Address` is `nullptr`, then the call is equivalent to `malloc(Size)`.
If `Size` is equal to zero, and `Address` is not `nullptr`, then the call is equivalent to `free(Address)`.
- `Address` - The address of the memory block to resize.
- `Size` - The new size of the memory block in bytes.

---
