/*
   This file is part of Fennix Kernel.

   Fennix Kernel is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of
   the License, or (at your option) any later version.

   Fennix Kernel is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

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
        std::vector<bucket> bkts;

        size_t hash(const key_type &key) const
        {
            std::hash<key_type> hash_function;
            return hash_function(key) % this->bkts.size();
        }

    public:
        unordered_map() : bkts(DEFAULT_NUM_BUCKETS) {}
        unordered_map(size_t num_buckets) : bkts(num_buckets) {}

        void insert(const key_value_pair &pair)
        {
            size_t bucket_index = hash(pair.first);
            bucket &bkt = this->bkts[bucket_index];
            for (auto it = bkt.begin(); it != bkt.end(); ++it)
            {
                if (it->first == pair.first)
                {
                    it->second = pair.second;
                    return;
                }
            }
            bkt.push_back(pair);
        }

        bool contains(const key_type &key) const
        {
            size_t bucket_index = hash(key);
            const bucket &bkt = this->bkts[bucket_index];
            for (auto it = bkt.begin(); it != bkt.end(); ++it)
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
            bucket &bkt = this->bkts[bucket_index];
            for (auto it = bkt.begin(); it != bkt.end(); ++it)
            {
                if (it->first == k)
                    return it;
            }
            return bkt.end();
        }

        const_iterator find(const key_type &k) const
        {
            size_t bucket_index = hash(k);
            const bucket &bkt = this->bkts[bucket_index];
            for (auto it = bkt.begin(); it != bkt.end(); ++it)
            {
                if (it->first == k)
                    return it;
            }
            return bkt.end();
        }

        iterator end() noexcept { return this->bkts.end(); }

        size_t size() const
        {
            size_t count = 0;
            foreach (const auto &bkt in this->bkts)
                count += bkt.size();

            return count;
        }

        value_type &operator[](const key_type &key)
        {
            size_t bucket_index = hash(key);
            bucket &bkt = this->bkts[bucket_index];
            for (auto it = bkt.begin(); it != bkt.end(); ++it)
            {
                if (it->first == key)
                    return it->second;
            }
            bkt.emplace_back(key, value_type());
            return bkt.back().second;
        }
    };
}

#endif // !__FENNIX_KERNEL_STD_UNORDERED_MAP_H__
