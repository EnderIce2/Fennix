#ifndef __FENNIX_KERNEL_VECTOR_H__
#define __FENNIX_KERNEL_VECTOR_H__

#include <types.h>
#include <assert.h>
#include <cstring>

template <class T>
class Vector
{
private:
    size_t VectorSize = 0;
    size_t VectorCapacity = 0;
    T *VectorBuffer = nullptr;

public:
    typedef T *iterator;

    NIF Vector()
    {
#ifdef DEBUG_MEM_ALLOCATION
        debug("VECTOR INIT: Vector( )");
#endif
        VectorCapacity = 0;
        VectorSize = 0;
        VectorBuffer = 0;
    }

    NIF Vector(size_t Size)
    {
        VectorCapacity = Size;
        VectorSize = Size;
#ifdef DEBUG_MEM_ALLOCATION
        debug("VECTOR INIT: Vector( %lld )", Size);
#endif
        VectorBuffer = new T[Size];
    }

    NIF Vector(size_t Size, const T &Initial)
    {
        VectorSize = Size;
        VectorCapacity = Size;
#ifdef DEBUG_MEM_ALLOCATION
        debug("VECTOR INIT: Vector( %lld %llx )", Size, Initial);
#endif
        assert(Size > 0);
        VectorBuffer = new T[Size];
        for (size_t i = 0; i < Size; i++)
            VectorBuffer[i] = Initial;
    }

    NIF Vector(const Vector<T> &Vector)
    {
        VectorSize = Vector.VectorSize;
        VectorCapacity = Vector.VectorCapacity;
#ifdef DEBUG_MEM_ALLOCATION
        debug("VECTOR INIT: Vector( <vector> )->Size: %lld", VectorSize);
#endif
        assert(VectorSize > 0);
        VectorBuffer = new T[VectorSize];
        for (size_t i = 0; i < VectorSize; i++)
            VectorBuffer[i] = Vector.VectorBuffer[i];
    }

    NIF ~Vector()
    {
#ifdef DEBUG_MEM_ALLOCATION
        debug("VECTOR INIT: ~Vector( ~%lx )", VectorBuffer);
#endif
        VectorSize = 0;
        VectorCapacity = 0;
        if (VectorBuffer != nullptr)
        {
            delete[] VectorBuffer, VectorBuffer = nullptr;
        }
    }

    NIF void remove(size_t Position)
    {
        if (Position >= VectorSize)
            return;
        memset(&*(VectorBuffer + Position), 0, sizeof(T));
        for (size_t i = 0; i < VectorSize - 1; i++)
        {
            *(VectorBuffer + Position + i) = *(VectorBuffer + Position + i + 1);
        }
        VectorSize--;
    }

    NIF void remove(const T &Value)
    {
        for (size_t i = 0; i < VectorSize; i++)
        {
            if (VectorBuffer[i] == Value)
            {
                remove(i);
                return;
            }
        }
    }

    NIF T &null_elem()
    {
        static T null_elem;
        return null_elem;
    }

    NIF bool null_elem(size_t Index)
    {
        if (!reinterpret_cast<uintptr_t>(&VectorBuffer[Index]))
            return false;
        return true;
    }

    NIF T &next(size_t Position)
    {
        if (Position + 1 < VectorSize && reinterpret_cast<uintptr_t>(&VectorBuffer[Position + 1]))
            return VectorBuffer[Position + 1];
        warn("next( %lld ) is null (requested by %#lx)", Position, __builtin_return_address(0));
        return this->null_elem();
    }

    NIF T &prev(size_t Position)
    {
        if (Position > 0 && reinterpret_cast<uintptr_t>(&VectorBuffer[Position - 1]))
            return VectorBuffer[Position - 1];
        warn("prev( %lld ) is null (requested by %#lx)", Position, __builtin_return_address(0));
        return this->null_elem();
    }

    NIF T &next(const T &Value)
    {
        for (size_t i = 0; i < VectorSize; i++)
        {
            if (VectorBuffer[i] == Value)
            {
                if (i + 1 < VectorSize && reinterpret_cast<uintptr_t>(&VectorBuffer[i + 1]))
                    return VectorBuffer[i + 1];
                else
                    break;
            }
        }
        warn("next( %#lx ) is null (requested by %#lx)", Value, __builtin_return_address(0));
        return this->null_elem();
    }

    NIF T &prev(const T &Value)
    {
        for (size_t i = 0; i < VectorSize; i++)
        {
            if (VectorBuffer[i] == Value)
            {
                if (i > 0 && reinterpret_cast<uintptr_t>(&VectorBuffer[i - 1]))
                    return VectorBuffer[i - 1];
                else
                    break;
            }
        }
        warn("prev( %#lx ) is null (requested by %#lx)", Value, __builtin_return_address(0));
        return this->null_elem();
    }

    NIF size_t capacity() const { return VectorCapacity; }

    NIF size_t size() const { return VectorSize; }

    NIF bool empty() const;

    NIF iterator begin() { return VectorBuffer; }

    NIF iterator end() { return VectorBuffer + size(); }

    NIF T &front() { return VectorBuffer[0]; }

    NIF T &back() { return VectorBuffer[VectorSize - 1]; }

    NIF void push_back(const T &Value)
    {
        if (VectorSize >= VectorCapacity)
            reserve(VectorCapacity + 5);
        VectorBuffer[VectorSize++] = Value;
    }

    NIF void pop_back() { VectorSize--; }

    NIF void reverse()
    {
        if (VectorSize <= 1)
            return;
        for (size_t i = 0, j = VectorSize - 1; i < j; i++, j--)
        {
            T c = *(VectorBuffer + i);
            *(VectorBuffer + i) = *(VectorBuffer + j);
            *(VectorBuffer + j) = c;
        }
    }

    NIF void reserve(size_t Capacity)
    {
        if (VectorBuffer == 0)
        {
            VectorSize = 0;
            VectorCapacity = 0;
        }
#ifdef DEBUG_MEM_ALLOCATION
        debug("VECTOR ALLOCATION: reverse( %lld )", Capacity);
#endif
        T *NewBuffer = new T[Capacity];
        size_t _Size = Capacity < VectorSize ? Capacity : VectorSize;
        for (size_t i = 0; i < _Size; i++)
            NewBuffer[i] = VectorBuffer[i];
        VectorCapacity = Capacity;
#ifdef DEBUG_MEM_ALLOCATION
        debug("VECTOR ALLOCATION: reverse( <Capacity> )->Buffer:~%lld", VectorBuffer);
#endif
        delete[] VectorBuffer;
        VectorBuffer = NewBuffer;
    }

    NIF void resize(size_t Size)
    {
        reserve(Size);
        VectorSize = Size;
    }

    NIF void clear()
    {
        VectorCapacity = 0;
        VectorSize = 0;
        if (VectorBuffer != nullptr)
        {
            delete[] VectorBuffer, VectorBuffer = nullptr;
        }
    }

    NIF T *data() { return VectorBuffer; }

    NIF T &operator[](size_t Index)
    {
        if (!reinterpret_cast<uintptr_t>(&VectorBuffer[Index]))
        {
            warn("operator[]( %lld ) is null (requested by %#lx)", Index, __builtin_return_address(0));
            return this->null_elem();
        }
        return VectorBuffer[Index];
    }

    NIF Vector<T> &operator=(const Vector<T> &Vector)
    {
        delete[] VectorBuffer;
        VectorSize = Vector.VectorSize;
        VectorCapacity = Vector.VectorCapacity;
#ifdef DEBUG_MEM_ALLOCATION
        debug("VECTOR ALLOCATION: operator=( <vector> )->Size:%lld", VectorSize);
#endif
        VectorBuffer = new T[VectorSize];
        for (size_t i = 0; i < VectorSize; i++)
            VectorBuffer[i] = Vector.VectorBuffer[i];
        return *this;
    }
};

#endif // !__FENNIX_KERNEL_VECTOR_H__
