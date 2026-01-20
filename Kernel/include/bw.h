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

/**
 * This file has names based on
 * Table 5-3: Register Attributes
 * in the eXtensible Host Controller Interface Specification
 *
 * I intend to make this as generic as possible, so that
 *  it can be used for other purposes.
 */

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

/**
 * Read-only
 *
 * Register bits are read-only and may not be altered by software.
 *
 * Example:
 *   BF_RO(uint8_t, SomeFlag, 0, 1);
 */
#define BF_RO(type, name, shift, width)                    \
	inline type name() const                               \
	{                                                      \
		return (type)((raw >> (shift)) & _BF_MASK(width)); \
	}

/**
 * Sticky - Read-only
 *
 * Register bits are read-only and may not be altered by software.
 * Value is preserved across resets.
 *
 * Example:
 *   BF_ROS(uint8_t, SomeFlag, 0, 1);
 */
#define BF_ROS(type, name, shift, width) BF_RO(type, name, shift, width)

/**
 * Read-Write
 *
 * Register bits are read-write and may be either
 *  set or cleared by software to the desired state.
 *
 * Example:
 *   BF_RW(uint8_t, SomeFlag, 2, 1);
 */
#define BF_RW(type, name, shift, width)                                                          \
	BF_RO(type, name, shift, width)                                                              \
	inline void name(type v)                                                                     \
	{                                                                                            \
		using __raw_t = std::remove_cv_t<decltype(raw)>;                                         \
		const __raw_t __mask = (__raw_t)_BF_MASK_SHIFT(width, shift);                            \
		raw = (__raw_t)((raw & ~__mask) | (((__raw_t)v & (__raw_t)_BF_MASK(width)) << (shift))); \
	}

/**
 * Sticky - Read-Write
 *
 * Register bits are read-write and may be either
 *  set or cleared by software to the desired state.
 * Value is preserved across resets.
 *
 * Example:
 *   BF_RWS(uint8_t, SomeFlag, 4, 1);
 */
#define BF_RWS(type, name, shift, width) BF_RW(type, name, shift, width)

/**
 * Write-1-to-clear
 *
 * Register bits indicate status when read,
 *  a set bit indicating a status event may be
 *  cleared by writing a '1'.
 * Writing a '0' has no effect.
 *
 * Example:
 *   BF_RW1C(uint8_t, SomeFlag, 4, 1);
 */
#define BF_RW1C(type, name, shift, width) BF_RW(type, name, shift, width)

/**
 * Write-1-to-set
 *
 * Register bits indicate status when read,
 *  a clear bit may be set by writing a '1'.
 * Writing a '0' has no effect.
 *
 * Example:
 *   BF_RW1S(uint8_t, SomeFlag, 4, 1);
 */
#define BF_RW1S(type, name, shift, width) BF_RW1C(type, name, shift, width)

/**
 * Sticky - Write-1-to-clear
 *
 * Register bits indicate status when read,
 *  a set bit indicating a status event may
 *  be cleared by writing a '1'.
 * Writing a '0' to RW1CS bits has no effect.
 * Value is preserved across resets.
 *
 * Example:
 *   BF_RW1CS(uint8_t, SomeFlag, 4, 1);
 */
#define BF_RW1CS(type, name, shift, width) BF_RW1C(type, name, shift, width)

/**
 * Read-Write Operation for Physical Address
 *
 * For fields where the value is written raw (no >> shift in getter).
 * Useful for fields representing physical addresses or pre-shifted values.
 * Setter writes v masked into the bitfield (v already at bit position).
 *
 * Example:
 *   BF_PHYS_RW(uint64_t, SomePhysField, 6, 58);
 */
#define BF_PHYS_RW(type, name, shift, width)                          \
	inline type name() const                                          \
	{                                                                 \
		return (type)(raw & _BF_MASK_SHIFT(width, shift));            \
	}                                                                 \
	inline void name(type v)                                          \
	{                                                                 \
		using __raw_t = std::remove_cv_t<decltype(raw)>;              \
		const __raw_t __mask = (__raw_t)_BF_MASK_SHIFT(width, shift); \
		raw = (__raw_t)((raw & ~__mask) | (v & __mask));              \
	}

#endif // __cplusplus
