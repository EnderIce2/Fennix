#ifndef __FENNIX_KERNEL_STD_UNORDERED_MAP_H__
#define __FENNIX_KERNEL_STD_UNORDERED_MAP_H__

#include <types.h>
#include <std/functional.hpp>
#include <std/utility.hpp>
#include <std/list.hpp>

namespace std
{
    template <typename key_type, typename value_type>
    class unordered_map
    {
    public:
        typedef std::pair<key_type, value_type> key_value_pair;
        typedef std::list<key_value_pair> bucket;
        typedef typename std::vector<bucket>::iterator iterator;
        typedef typename std::vector<bucket>::const_iterator const_iterator;

    private:
        static const size_t DEFAULT_NUM_BUCKETS = 10;
        std::vector<bucket> m_buckets;

        size_t hash(const key_type &key) const
        {
            std::hash<key_type> hash_function;
            return hash_function(key) % m_buckets.size();
        }

    public:
        unordered_map() : m_buckets(DEFAULT_NUM_BUCKETS) {}
        unordered_map(size_t num_buckets) : m_buckets(num_buckets) {}

        void insert(const key_value_pair &pair)
        {
            size_t bucket_index = hash(pair.first);
            bucket &bucket = m_buckets[bucket_index];
            for (auto it = bucket.begin(); it != bucket.end(); ++it)
            {
                if (it->first == pair.first)
                {
                    it->second = pair.second;
                    return;
                }
            }
            bucket.push_back(pair);
        }

        bool contains(const key_type &key) const
        {
            size_t bucket_index = hash(key);
            const bucket &bucket = m_buckets[bucket_index];
            for (auto it = bucket.begin(); it != bucket.end(); ++it)
            {
                if (it->first == key)
                {
                    return true;
                }
            }
            return false;
        }

        iterator find(const key_type &k)
        {
            size_t bucket_index = hash(k);
            bucket &bucket = m_buckets[bucket_index];
            for (auto it = bucket.begin(); it != bucket.end(); ++it)
            {
                if (it->first == k)
                    return it;
            }
            return bucket.end();
        }

        const_iterator find(const key_type &k) const
        {
            size_t bucket_index = hash(k);
            const bucket &bucket = m_buckets[bucket_index];
            for (auto it = bucket.begin(); it != bucket.end(); ++it)
            {
                if (it->first == k)
                    return it;
            }
            return bucket.end();
        }

        iterator end() noexcept { return m_buckets.end(); }

        size_t size() const
        {
            size_t count = 0;
            for (const auto &bucket : m_buckets)
                count += bucket.size();

            return count;
        }

        value_type &operator[](const key_type &key)
        {
            size_t bucket_index = hash(key);
            bucket &bucket = m_buckets[bucket_index];
            for (auto it = bucket.begin(); it != bucket.end(); ++it)
            {
                if (it->first == key)
                    return it->second;
            }
            bucket.emplace_back(key, value_type());
            return bucket.back().second;
        }
    };
}

#endif // !__FENNIX_KERNEL_STD_UNORDERED_MAP_H__
