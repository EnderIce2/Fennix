#ifndef __FENNIX_KERNEL_STRING_H__
#define __FENNIX_KERNEL_STRING_H__

#include <types.h>
#include <convert.h>
#include <debug.h>

// show debug messages
// #define DEBUG_CPP_STRING 1

#ifdef DEBUG_CPP_STRING
#define strdbg(m, ...) debug(m, ##__VA_ARGS__)
#else
#define strdbg(m, ...)
#endif

// TODO: Somewhere the delete is called twice, causing a double free error.

/**
 * @brief String class
 * String class that can be used to store strings.
 */
class String
{
private:
    char *m_Data;
    int m_Length;
    int m_Capacity;

public:
    String(const char *Str = "")
    {
        this->m_Length = strlen(Str);
        this->m_Capacity = this->m_Length + 1;
        this->m_Data = new char[m_Capacity];
        strcpy(m_Data, Str);
        strdbg("New string created: \"%s\" (data: %#lx, length: %d, capacity: %d)", this->m_Data, this->m_Data, this->m_Length, this->m_Capacity);
    }

    ~String()
    {
        strdbg("String deleted: \"%s\" (data: %#lx, length: %d, capacity: %d)", this->m_Data, this->m_Data, this->m_Length, this->m_Capacity);
        delete[] this->m_Data;
    }

    int length() const
    {
        strdbg("String length: %d", this->m_Length);
        return this->m_Length;
    }

    int capacity() const
    {
        strdbg("String capacity: %d", this->m_Capacity);
        return this->m_Capacity;
    }

    const char *c_str() const
    {
        strdbg("String data: \"%s\"", this->m_Data);
        return this->m_Data;
    }

    void resize(int NewLength)
    {
        strdbg("String resize: %d", NewLength);
        if (NewLength > this->m_Capacity)
        {
            int newCapacity = NewLength + 1;
            char *newData = new char[newCapacity];

            strcpy(newData, this->m_Data);

            strdbg("old: %#lx, new: %#lx", this->m_Data, newData);
            delete[] this->m_Data;
            this->m_Data = newData;
            this->m_Capacity = newCapacity;
        }
        this->m_Length = NewLength;
        this->m_Data[m_Length] = '\0';
        strdbg("String resized: \"%s\" (data: %#lx, length: %d, capacity: %d)", this->m_Data, this->m_Data, this->m_Length, this->m_Capacity);
    }

    void concat(const String &Other)
    {
        int NewLength = this->m_Length + Other.m_Length;
        this->resize(NewLength);

        strcat(m_Data, Other.m_Data);
        strdbg("String concatenated: \"%s\" (data: %#lx, length: %d, capacity: %d)", this->m_Data, this->m_Data, this->m_Length, this->m_Capacity);
    }

    String operator+(const String &Other) const
    {
        String result = *this;
        result.concat(Other);
        strdbg("String added: \"%s\" (data: %#lx, length: %d, capacity: %d)", result.m_Data, result.m_Data, result.m_Length, result.m_Capacity);
        return result;
    }

    String operator+(const char *Other) const
    {
        String result = *this;
        result.concat(Other);
        strdbg("String added: \"%s\" (data: %#lx, length: %d, capacity: %d)", result.m_Data, result.m_Data, result.m_Length, result.m_Capacity);
        return result;
    }

    /* warning: implicitly-declared ‘constexpr String::String(const String&)’ is deprecated [-Wdeprecated-copy] */
    String &operator=(const String &Other) = default;

    // String &operator=(const String &Other)
    // {
    //     if (this != &Other)
    //     {
    //         delete[] this->m_Data;
    //         this->m_Data = Other.m_Data;
    //         this->m_Length = Other.m_Length;
    //         this->m_Capacity = Other.m_Capacity;
    //         strdbg("String assigned: \"%s\" (data: %#lx, length: %d, capacity: %d)", this->m_Data, this->m_Data, this->m_Length, this->m_Capacity);
    //     }
    //     return *this;
    // }

    String &operator=(const char *Other)
    {
        this->m_Length = strlen(Other);
        this->m_Capacity = this->m_Length + 1;
        delete[] this->m_Data;
        this->m_Data = new char[m_Capacity];
        strcpy(m_Data, Other);
        strdbg("String assigned: \"%s\" (data: %#lx, length: %d, capacity: %d)", this->m_Data, this->m_Data, this->m_Length, this->m_Capacity);
        return *this;
    }

    String &operator<<(const String &Other)
    {
        this->concat(Other);
        strdbg("String appended: \"%s\" (data: %#lx, length: %d, capacity: %d)", this->m_Data, this->m_Data, this->m_Length, this->m_Capacity);
        return *this;
    }

    String &operator<<(const char *Other)
    {
        this->concat(Other);
        strdbg("String appended: \"%s\" (data: %#lx, length: %d, capacity: %d)", this->m_Data, this->m_Data, this->m_Length, this->m_Capacity);
        return *this;
    }

    char &operator[](int Index)
    {
        strdbg("String index: %d", Index);
        return this->m_Data[Index];
    }

    const char &operator[](int Index) const
    {
        strdbg("String index: %d", Index);
        return this->m_Data[Index];
    }

    bool operator==(const String &Other) const
    {
        strdbg("String compared: \"%s\" == \"%s\"", this->m_Data, Other.m_Data);
        return strcmp(this->m_Data, Other.m_Data) == 0;
    }

    bool operator!=(const String &Other) const
    {
        strdbg("String compared: \"%s\" != \"%s\"", this->m_Data, Other.m_Data);
        return strcmp(this->m_Data, Other.m_Data) != 0;
    }

    bool operator==(const char *Other) const
    {
        strdbg("String compared: \"%s\" == \"%s\"", this->m_Data, Other);
        return strcmp(this->m_Data, Other) == 0;
    }

    bool operator!=(const char *Other) const
    {
        strdbg("String compared: \"%s\" != \"%s\"", this->m_Data, Other);
        return strcmp(this->m_Data, Other) != 0;
    }

    class Iterator
    {
    private:
        char *m_Pointer;

    public:
        Iterator(char *Pointer) : m_Pointer(Pointer) {}

        Iterator &operator++()
        {
            ++this->m_Pointer;
            strdbg("String iterator incremented: %#lx", this->m_Pointer);
            return *this;
        }

        char &operator*()
        {
            strdbg("String iterator dereferenced: %#lx", this->m_Pointer);
            return *this->m_Pointer;
        }

        bool operator!=(const Iterator &Other) const
        {
            strdbg("String iterator compared: %#lx != %#lx", this->m_Pointer, Other.m_Pointer);
            return this->m_Pointer != Other.m_Pointer;
        }

        bool operator==(const Iterator &Other) const
        {
            strdbg("String iterator compared: %#lx == %#lx", this->m_Pointer, Other.m_Pointer);
            return this->m_Pointer == Other.m_Pointer;
        }
    };

    Iterator begin()
    {
        strdbg("String iterator begin: %#lx", this->m_Data);
        return Iterator(this->m_Data);
    }

    Iterator end()
    {
        strdbg("String iterator end: %#lx", this->m_Data + this->m_Length);
        return Iterator(this->m_Data + this->m_Length);
    }
};

#endif // !__FENNIX_KERNEL_STRING_H__
