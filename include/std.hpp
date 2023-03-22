#ifndef __FENNIX_KERNEL_STD_H__
#define __FENNIX_KERNEL_STD_H__

#include <types.h>
#include <smartptr.hpp>
#include <vector.hpp>
#include <string.hpp>

/**
 * @brief // stub namespace for std::align_val_t and new operator
 * @note // Found on https://gcc.gnu.org/legacy-ml/gcc-patches/2016-09/msg00628.html for "_ZnwmSt11align_val_t" compiler error
 */
namespace std
{
    typedef __SIZE_TYPE__ size_t;
    enum class align_val_t : std::size_t
    {
    };

    template <class T>
    class smart_ptr
    {
    public:
        using SmartPointer<T>::SmartPointer;
        using SmartPointer<T>::operator*;
        using SmartPointer<T>::operator->;
    };

    template <class T>
    class auto_ptr
    {
    public:
        using AutoPointer<T>::AutoPointer;
    };

    template <class T>
    class unique_ptr
    {
    public:
        using UniquePointer<T>::UniquePointer;
    };

    template <class T>
    class weak_ptr
    {
    public:
        using WeakPointer<T>::WeakPointer;
    };

    template <typename T>
    class shared_ptr
    {
        using SharedPointer<T>::SharedPointer;
        using SharedPointer<T>::operator*;
        using SharedPointer<T>::operator->;
    };

    template <typename T>
    struct remove_reference
    {
        typedef T type;
    };

    template <typename T>
    struct remove_reference<T &>
    {
        typedef T type;
    };

    template <typename T>
    struct remove_reference<T &&>
    {
        typedef T type;
    };

    template <typename T>
    using remove_reference_t = typename remove_reference<T>::type;

    template <typename T>
    T &&forward(remove_reference_t<T> &t) { return static_cast<T &&>(t); };

    template <typename T>
    T &&forward(remove_reference_t<T> &&t) { return static_cast<T &&>(t); };

    template <typename T, typename... Args>
    shared_ptr<T> make_shared(Args &&...args)
    {
        return SharedPointer<T>(new T(forward<Args>(args)...));
    };

    template <class T>
    class vector : public Vector<T>
    {
    public:
        using Vector<T>::Vector;
        using Vector<T>::operator=;
        using Vector<T>::operator[];
        using typename Vector<T>::iterator;
    };

    class string : public String
    {
    public:
        using String::String;
        using String::operator=;
        using String::operator+;
        using String::operator<<;
        using String::operator[];
        using String::operator==;
        using String::operator!=;
        using typename String::iterator;
    };
}

#endif // !__FENNIX_KERNEL_STD_H__
