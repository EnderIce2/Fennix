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

#endif // __cplusplus
