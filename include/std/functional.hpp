#ifndef __FENNIX_KERNEL_STD_FUNCTIONAL_H__
#define __FENNIX_KERNEL_STD_FUNCTIONAL_H__

#include <types.h>
#include <assert.h>

namespace std
{
    template <typename Key>
    struct hash
    {
        size_t operator()(const Key &key) const
        {
            static_assert(sizeof(size_t) == sizeof(uint64_t)); // size_t and uint64_t must have the same size
            const uint64_t fnv_offset_basis = 14695981039346656037ull;
            const uint64_t fnv_prime = 1099511628211ull;

            const uint8_t *data = reinterpret_cast<const uint8_t *>(&key);
            const size_t size = sizeof(Key);
            uint64_t ret = fnv_offset_basis;

            for (size_t i = 0; i < size; ++i)
            {
                ret ^= static_cast<uint64_t>(data[i]);
                ret *= fnv_prime;
            }

            return static_cast<size_t>(ret);
        }
    };
}

#endif // !__FENNIX_KERNEL_STD_FUNCTIONAL_H__
