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

#ifndef __FENNIX_KERNEL_STD_LIST_H__
#define __FENNIX_KERNEL_STD_LIST_H__

#include <types.h>
#include <std/stdexcept.hpp>

namespace std
{
    template <typename T>
    class list
    {
    private:
        struct Node
        {
            T value;
            Node *prev;
            Node *next;

            Node(const T &v, Node *p = nullptr, Node *n = nullptr)
                : value(v), prev(p), next(n) {}
        };

        Node *head;
        Node *tail;
        size_t size;

    public:
        list() : head(nullptr), tail(nullptr), size(0) {}
        ~list() { clear(); }
        list(const list &other) : head(nullptr), tail(nullptr), size(0) { *this = other; }

        void push_back(const T &value)
        {
            Node *new_node = new Node(value, tail, nullptr);
            if (empty())
            {
                head = tail = new_node;
            }
            else
            {
                tail->next = new_node;
                tail = new_node;
            }
            ++size;
        }

        void pop_back()
        {
            if (empty())
            {
                throw std::runtime_error("list is empty");
            }
            else if (head == tail)
            {
                delete tail;
                head = tail = nullptr;
                --size;
            }
            else
            {
                Node *old_tail = tail;
                tail = tail->prev;
                tail->next = nullptr;
                delete old_tail;
                --size;
            }
        }

        void push_front(const T &value)
        {
            Node *new_node = new Node(value, nullptr, head);
            if (empty())
            {
                head = tail = new_node;
            }
            else
            {
                head->prev = new_node;
                head = new_node;
            }
            ++size;
        }

        void pop_front()
        {
            if (empty())
            {
                throw std::runtime_error("list is empty");
            }
            else if (head == tail)
            {
                delete head;
                head = tail = nullptr;
                --size;
            }
            else
            {
                Node *old_head = head;
                head = head->next;
                head->prev = nullptr;
                delete old_head;
                --size;
            }
        }

        bool empty() const { return size == 0; }

        void clear()
        {
            while (!empty())
                pop_back();
        }

        list &operator=(const list &other)
        {
            if (this != &other)
            {
                clear();
                for (const T &value : other)
                {
                    push_back(value);
                }
            }
            return *this;
        }

        class iterator
        {
        private:
            Node *node;
            friend class list;

        public:
            iterator(Node *p = nullptr) : node(p) {}
            T &operator*() const { return node->value; }
            T *operator->() const { return &node->value; }

            iterator &operator++()
            {
                node = node->next;
                return *this;
            }

            iterator &operator--()
            {
                node = node->prev;
                return *this;
            }

            iterator operator++(int)
            {
                iterator tmp = *this;
                ++*this;
                return tmp;
            }

            iterator operator--(int)
            {
                iterator tmp = *this;
                --*this;
                return tmp;
            }

            bool operator==(const iterator &rhs) const { return node == rhs.node; }
            bool operator!=(const iterator &rhs) const { return node != rhs.node; }
        };

        iterator begin() { return iterator(head); }
        iterator end() { return iterator(nullptr); }

        iterator insert(iterator pos, const T &value)
        {
            if (pos == end())
            {
                push_back(value);
                return iterator(tail);
            }
            else if (pos == begin())
            {
                push_front(value);
                return iterator(head);
            }
            else
            {
                Node *p = pos.node;
                Node *new_node = new Node(value, p->prev, p);
                p->prev->next = new_node;
                p->prev = new_node;
                ++size;
                return iterator(new_node);
            }
        }

        iterator erase(iterator pos)
        {
            Node *p = pos.node;
            iterator next(p->next);
            p->prev->next = p->next;
            p->next->prev = p->prev;
            delete p;
            --size;
            return next;
        }
    };
}

#endif // !__FENNIX_KERNEL_STD_LIST_H__
