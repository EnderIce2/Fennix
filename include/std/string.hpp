#ifndef __FENNIX_KERNEL_STD_STRING_H__
#define __FENNIX_KERNEL_STD_STRING_H__

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

namespace std
{
    /**
     * @brief String class
     * String class that can be used to store strings.
     */
    class string
    {
    private:
        char *m_Data;
        size_t m_Length;
        size_t m_Capacity;

    public:
        static const size_t npos = -1;

        string(const char *Str = "")
        {
            this->m_Length = strlen(Str);
            this->m_Capacity = this->m_Length + 1;
            this->m_Data = new char[m_Capacity];
            strcpy(m_Data, Str);
            strdbg("New string created: \"%s\" (data: %#lx, length: %d, capacity: %d)", this->m_Data, this->m_Data, this->m_Length, this->m_Capacity);
        }

        ~string()
        {
            strdbg("String deleted: \"%s\" (data: %#lx, length: %d, capacity: %d)", this->m_Data, this->m_Data, this->m_Length, this->m_Capacity);
            delete[] this->m_Data, this->m_Data = nullptr;
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

        void resize(size_t NewLength)
        {
            strdbg("String resize: %d", NewLength);
            if (NewLength > this->m_Capacity)
            {
                size_t newCapacity = NewLength + 1;
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

        void concat(const string &Other)
        {
            int NewLength = this->m_Length + Other.m_Length;
            this->resize(NewLength);

            strcat(m_Data, Other.m_Data);
            strdbg("String concatenated: \"%s\" (data: %#lx, length: %d, capacity: %d)", this->m_Data, this->m_Data, this->m_Length, this->m_Capacity);
        }

        bool empty() const
        {
            strdbg("String empty: %d", this->m_Length == 0);
            return this->m_Length == 0;
        }

        size_t size() const
        {
            strdbg("String size: %d", this->m_Length);
            return this->m_Length;
        }

        void clear()
        {
            strdbg("String clear");
            this->resize(0);
        }

        size_t find(const char *Str, size_t Pos = 0) const
        {
            strdbg("String find: \"%s\", %d", Str, Pos);
            if (Pos >= this->m_Length)
                return npos;

            for (size_t i = Pos; i < this->m_Length; i++)
            {
                bool found = true;
                for (size_t j = 0; Str[j] != '\0'; j++)
                {
                    if (this->m_Data[i + j] != Str[j])
                    {
                        found = false;
                        break;
                    }
                }
                if (found)
                    return i;
            }
            return npos;
        }

        size_t find(const string &Str, size_t Pos = 0) const
        {
            strdbg("String find: \"%s\", %d", Str.c_str(), Pos);
            return this->find(Str.c_str(), Pos);
        }

        void erase(int Index, int Count = 1)
        {
            strdbg("String erase: %d, %d", Index, Count);
            if (Index < 0 || (size_t)Index >= this->m_Length)
                return;

            if (Count < 0)
                return;

            if ((size_t)(Index + Count) > this->m_Length)
                Count = this->m_Length - Index;

            for (size_t i = Index; i < this->m_Length - Count; i++)
                this->m_Data[i] = this->m_Data[i + Count];

            this->m_Length -= Count;
            this->m_Data[m_Length] = '\0';
            strdbg("String erased: \"%s\" (data: %#lx, length: %d, capacity: %d)", this->m_Data, this->m_Data, this->m_Length, this->m_Capacity);
        }

        size_t find_last_not_of(const char *Str, size_t Pos = npos) const
        {
            strdbg("String find_last_not_of: \"%s\", %d", Str, Pos);
            if (Pos == npos)
                Pos = this->m_Length - 1;

            for (int i = (int)Pos; i >= 0; i--)
            {
                bool found = false;
                for (size_t j = 0; Str[j] != '\0'; j++)
                {
                    if (this->m_Data[i] == Str[j])
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return i;
            }
            return npos;
        }

        size_t find_first_not_of(const char *Str, size_t Pos = 0) const
        {
            strdbg("String find_first_not_of: \"%s\", %d", Str, Pos);
            if (Pos >= this->m_Length)
                return npos;

            for (size_t i = Pos; i < this->m_Length; i++)
            {
                bool found = false;
                for (size_t j = 0; Str[j] != '\0'; j++)
                {
                    if (this->m_Data[i] == Str[j])
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return i;
            }
            return npos;
        }

        size_t find_first_of(const char *Str, size_t Pos = 0) const
        {
            strdbg("String find_first_of: \"%s\", %d", Str, Pos);
            if (Pos >= this->m_Length)
                return npos;

            for (size_t i = Pos; i < this->m_Length; i++)
            {
                bool found = false;
                for (size_t j = 0; Str[j] != '\0'; j++)
                {
                    if (this->m_Data[i] == Str[j])
                    {
                        found = true;
                        break;
                    }
                }
                if (found)
                    return i;
            }
            return npos;
        }

        size_t find_last_of(const char *Str, size_t Pos = npos) const
        {
            strdbg("String find_last_of: \"%s\", %d", Str, Pos);
            if (Pos == npos)
                Pos = this->m_Length - 1;

            for (int i = (int)Pos; i >= 0; i--)
            {
                bool found = false;
                for (int j = 0; Str[j] != '\0'; j++)
                {
                    if (this->m_Data[i] == Str[j])
                    {
                        found = true;
                        break;
                    }
                }
                if (found)
                    return i;
            }
            return npos;
        }

        size_t find_first_of(char C, size_t Pos = 0) const
        {
            strdbg("String find_first_of: '%c', %d", C, Pos);
            if (Pos >= this->m_Length)
                return npos;

            for (size_t i = Pos; i < this->m_Length; i++)
            {
                if (this->m_Data[i] == C)
                    return i;
            }
            return npos;
        }

        size_t find_last_of(char C, size_t Pos = npos) const
        {
            strdbg("String find_last_of: '%c', %d", C, Pos);
            if (Pos == npos)
                Pos = this->m_Length - 1;

            for (int i = (int)Pos; i >= 0; i--)
            {
                if (this->m_Data[i] == C)
                    return i;
            }
            return npos;
        }

        size_t substr(const char *Str, size_t Pos = 0) const
        {
            strdbg("String substr: \"%s\", %d", Str, Pos);
            if (Pos >= this->m_Length)
                return npos;

            for (size_t i = Pos; i < this->m_Length; i++)
            {
                bool found = true;
                for (size_t j = 0; Str[j] != '\0'; j++)
                {
                    if (this->m_Data[i + j] != Str[j])
                    {
                        found = false;
                        break;
                    }
                }
                if (found)
                    return i;
            }
            return npos;
        }

        size_t substr(const string &Str, size_t Pos = 0) const
        {
            strdbg("String substr: \"%s\", %d", Str.c_str(), Pos);
            return this->substr(Str.c_str(), Pos);
        }

        string substr(size_t Pos = 0, size_t Count = npos) const
        {
            strdbg("String substr: %d, %d", Pos, Count);
            if (Pos >= this->m_Length)
                return string();

            if (Count == npos)
                Count = this->m_Length - Pos;

            if (Pos + Count > this->m_Length)
                Count = this->m_Length - Pos;

            string ret;
            ret.resize(Count);
            for (size_t i = 0; i < Count; i++)
                ret.m_Data[i] = this->m_Data[Pos + i];
            ret.m_Data[Count] = '\0';
            return ret;
        }

        void replace(size_t Pos, size_t Count, const char *Str)
        {
            strdbg("String replace: %d, %d, \"%s\"", Pos, Count, Str);
            if (Pos >= this->m_Length)
                return;

            if ((int64_t)Count < 0)
                return;

            if (Pos + Count > this->m_Length)
                Count = this->m_Length - Pos;

            int NewLength = this->m_Length - Count + strlen(Str);
            this->resize(NewLength);

            for (size_t i = this->m_Length - 1; i >= Pos + strlen(Str); i--)
                this->m_Data[i] = this->m_Data[i - strlen(Str) + Count];

            for (unsigned long i = 0; i < strlen(Str); i++)
                this->m_Data[Pos + i] = Str[i];

            strdbg("String replaced: \"%s\" (data: %#lx, length: %d, capacity: %d)", this->m_Data, this->m_Data, this->m_Length, this->m_Capacity);
        }

        void replace(size_t Pos, size_t Count, const string &Str)
        {
            strdbg("String replace: %d, %d, \"%s\"", Pos, Count, Str.m_Data);
            if (Pos >= this->m_Length)
                return;

            if ((int64_t)Count < 0)
                return;

            if (Pos + Count > this->m_Length)
                Count = this->m_Length - Pos;

            int NewLength = this->m_Length - Count + Str.m_Length;
            this->resize(NewLength);

            for (size_t i = this->m_Length - 1; i >= Pos + Str.m_Length; i--)
                this->m_Data[i] = this->m_Data[i - Str.m_Length + Count];

            for (size_t i = 0; i < Str.m_Length; i++)
                this->m_Data[Pos + i] = Str.m_Data[i];

            strdbg("String replaced: \"%s\" (data: %#lx, length: %d, capacity: %d)", this->m_Data, this->m_Data, this->m_Length, this->m_Capacity);
        }

        string operator+(const string &Other) const
        {
            string result = *this;
            result.concat(Other);
            strdbg("String added: \"%s\" (data: %#lx, length: %d, capacity: %d)", result.m_Data, result.m_Data, result.m_Length, result.m_Capacity);
            return result;
        }

        string operator+(const char *Other) const
        {
            string result = *this;
            result.concat(Other);
            strdbg("String added: \"%s\" (data: %#lx, length: %d, capacity: %d)", result.m_Data, result.m_Data, result.m_Length, result.m_Capacity);
            return result;
        }

        string &operator+=(const string &Other)
        {
            this->concat(Other);
            strdbg("String appended: \"%s\" (data: %#lx, length: %d, capacity: %d)", this->m_Data, this->m_Data, this->m_Length, this->m_Capacity);
            return *this;
        }

        string &operator+=(const char *Other)
        {
            this->concat(Other);
            strdbg("String appended: \"%s\" (data: %#lx, length: %d, capacity: %d)", this->m_Data, this->m_Data, this->m_Length, this->m_Capacity);
            return *this;
        }

        /* warning: implicitly-declared ‘constexpr String::String(const String&)’ is deprecated [-Wdeprecated-copy] */
        string &operator=(const string &Other) = default;

        // string &operator=(const string &Other)
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

        string &operator=(const char *Other)
        {
            this->m_Length = strlen(Other);
            this->m_Capacity = this->m_Length + 1;
            delete[] this->m_Data;
            this->m_Data = new char[m_Capacity];
            strcpy(m_Data, Other);
            strdbg("String assigned: \"%s\" (data: %#lx, length: %d, capacity: %d)", this->m_Data, this->m_Data, this->m_Length, this->m_Capacity);
            return *this;
        }

        string &operator<<(const string &Other)
        {
            this->concat(Other);
            strdbg("String appended: \"%s\" (data: %#lx, length: %d, capacity: %d)", this->m_Data, this->m_Data, this->m_Length, this->m_Capacity);
            return *this;
        }

        string &operator<<(const char *Other)
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

        bool operator==(const string &Other) const
        {
            strdbg("String compared: \"%s\" == \"%s\"", this->m_Data, Other.m_Data);
            return strcmp(this->m_Data, Other.m_Data) == 0;
        }

        bool operator!=(const char *Other) const
        {
            strdbg("String compared: \"%s\" != \"%s\"", this->m_Data, Other);
            return strcmp(this->m_Data, Other) != 0;
        }

        bool operator!=(const string &Other) const
        {
            strdbg("String compared: \"%s\" != \"%s\"", this->m_Data, Other.m_Data);
            return strcmp(this->m_Data, Other.m_Data) != 0;
        }

        bool operator==(const char *Other) const
        {
            strdbg("String compared: \"%s\" == \"%s\"", this->m_Data, Other);
            return strcmp(this->m_Data, Other) == 0;
        }

        class iterator
        {
        private:
            char *m_Pointer;

        public:
            iterator(char *Pointer) : m_Pointer(Pointer) {}

            iterator &operator++()
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

            bool operator!=(const iterator &Other) const
            {
                strdbg("String iterator compared: %#lx != %#lx", this->m_Pointer, Other.m_Pointer);
                return this->m_Pointer != Other.m_Pointer;
            }

            bool operator==(const iterator &Other) const
            {
                strdbg("String iterator compared: %#lx == %#lx", this->m_Pointer, Other.m_Pointer);
                return this->m_Pointer == Other.m_Pointer;
            }
        };

        iterator begin()
        {
            strdbg("String iterator begin: %#lx", this->m_Data);
            return iterator(this->m_Data);
        }

        iterator end()
        {
            strdbg("String iterator end: %#lx", this->m_Data + this->m_Length);
            return iterator(this->m_Data + this->m_Length);
        }
    };
}

#endif // !__FENNIX_KERNEL_STD_STRING_H__
