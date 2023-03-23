#ifndef __FENNIX_KERNEL_STD_STDEXCEPT_H__
#define __FENNIX_KERNEL_STD_STDEXCEPT_H__

#include <types.h>

namespace std
{
    class runtime_error
    {
    private:
        const char *m_what;

    public:
        runtime_error(const char *what_arg) : m_what(what_arg) {}
        const char *what() const { return m_what; }
    };
}

#endif // !__FENNIX_KERNEL_STD_STDEXCEPT_H__
