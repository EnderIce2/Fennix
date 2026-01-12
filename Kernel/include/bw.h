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

#pragma once

#ifdef __cplusplus

#include <type_traits>

#define DEFINE_BITWISE_TYPE(OP_TYPE, STRUCT_NAME) \
	volatile OP_TYPE raw;                         \
                                                  \
	operator OP_TYPE()                            \
	{                                             \
		return raw;                               \
	}                                             \
                                                  \
	OP_TYPE operator=(OP_TYPE value)              \
	{                                             \
		raw = value;                              \
		return raw;                               \
	}                                             \
                                                  \
	bool operator==(auto value)                   \
	{                                             \
		return raw == (OP_TYPE)value;             \
	}                                             \
                                                  \
	STRUCT_NAME &operator|=(auto other)           \
	{                                             \
		raw |= (OP_TYPE)other;                    \
		return *this;                             \
	}                                             \
                                                  \
	STRUCT_NAME &operator&=(auto other)           \
	{                                             \
		raw &= (OP_TYPE)other;                    \
		return *this;                             \
	}                                             \
                                                  \
	STRUCT_NAME &operator^=(auto other)           \
	{                                             \
		raw ^= (OP_TYPE)other;                    \
		return *this;                             \
	}                                             \
                                                  \
	STRUCT_NAME operator|(auto other)             \
	{                                             \
		return STRUCT_NAME(raw | (OP_TYPE)other); \
	}                                             \
                                                  \
	STRUCT_NAME operator&(auto other)             \
	{                                             \
		return STRUCT_NAME(raw & (OP_TYPE)other); \
	}                                             \
                                                  \
	STRUCT_NAME operator^(auto other)             \
	{                                             \
		return STRUCT_NAME(raw ^ (OP_TYPE)other); \
	}                                             \
                                                  \
	STRUCT_NAME(OP_TYPE value)                    \
	{                                             \
		raw = value;                              \
	}                                             \
                                                  \
	STRUCT_NAME()                                 \
	{                                             \
		raw = 0;                                  \
	}

#define _BF_MASK(width) ((1ULL << (width)) - 1ULL)
#define _BF_MASK_SHIFT(width, shift) (_BF_MASK(width) << (shift))

/* Read-Only: type name() const */
#define BF_RO(type, name, shift, width)                    \
	inline type name() const                               \
	{                                                      \
		return (type)((raw >> (shift)) & _BF_MASK(width)); \
	}

/* Read-Write: type name() const; void name(type) */
#define BF_RW(type, name, shift, width)                                                          \
	inline type name() const                                                                     \
	{                                                                                            \
		return (type)((raw >> (shift)) & _BF_MASK(width));                                       \
	}                                                                                            \
	inline void name(type v)                                                                     \
	{                                                                                            \
		using __raw_t = std::remove_cv_t<decltype(raw)>;                                         \
		const __raw_t __mask = (__raw_t)_BF_MASK_SHIFT(width, shift);                            \
		raw = (__raw_t)((raw & ~__mask) | (((__raw_t)v & (__raw_t)_BF_MASK(width)) << (shift))); \
	}

#define BF_RO_EX(rawmember, type, name, shift, width)            \
	inline type name() const                                     \
	{                                                            \
		return (type)((rawmember >> (shift)) & _BF_MASK(width)); \
	}

#define BF_RW_EX(rawmember, type, name, shift, width)                                                        \
	inline type name() const                                                                                 \
	{                                                                                                        \
		return (type)((rawmember >> (shift)) & _BF_MASK(width));                                             \
	}                                                                                                        \
	inline void name(type v)                                                                                 \
	{                                                                                                        \
		using __raw_t = std::remove_cv_t<decltype(rawmember)>;                                               \
		const __raw_t __mask = (__raw_t)_BF_MASK_SHIFT(width, shift);                                        \
		rawmember = (__raw_t)((rawmember & ~__mask) | (((__raw_t)v & (__raw_t)_BF_MASK(width)) << (shift))); \
	}

#endif // __cplusplus
