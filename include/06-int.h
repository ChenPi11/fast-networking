/*
 * This file is part of the cppp-reiconv Library.
 *
 * The cppp-reiconv Library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * The cppp-reiconv Library is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the cppp-reiconv Library; see the file LICENSE.
 * If not, see <https://www.gnu.org/licenses/>.
 */

/* @NO MRG */
#include "01-stdafx.h" /* IWYU pragma: keep */

#ifndef FN_INT_H_INC
	#define FN_INT_H_INC
/* @END NO MRG */

	/* Old Windows SDK don't provide some integer types. */
	#if HAS_WINDOWS
typedef SSIZE_T UTIL_SignedSize;
typedef unsigned char UTIL_UInt8; /* VC6.0 Does not define UINT8. */
typedef unsigned short UTIL_UInt16;
typedef UINT32 UTIL_UInt32;
typedef UINT64 UTIL_UInt64;
typedef size_t UTIL_Time;
	#else
typedef ssize_t UTIL_SignedSize;
typedef uint8_t UTIL_UInt8;
typedef uint16_t UTIL_UInt16;
typedef uint32_t UTIL_UInt32;
typedef uint64_t UTIL_UInt64;
typedef size_t UTIL_Time;
	#endif /* HAS_WINDOWS */

/* The integer type must be twos complement. */
typedef char
	_UTIL_MustBeTwosComplementInt[(((int)0 - (int)1) == ~((int)0)) ? 1 : -1];
typedef char _UTIL_MustBeTwosComplementUint
	[(((unsigned)0 - (unsigned)1) == ~((unsigned)0)) ? 1 : -1];

	/* Util for check if integer type is signed. */
	#define UTIL_IS_SIGNED(Type) (((Type) - 1) < (Type)0)

	/* Util for get max value for signed integer type. */
	#define UTIL_SIGNED_MAX(Type) ((Type)(~((Type)1 << (sizeof(Type) * 8 - 1))))

	/* Util for get max value for unsigned integer type. */
	#define UTIL_UNSIGNED_MAX(Type) ((Type)(~(Type)0))

	#define UTIL_INTEGER_MAX(Type)                                             \
		(UTIL_IS_SIGNED(Type) ? UTIL_SIGNED_MAX(Type) : UTIL_UNSIGNED_MAX(Type))

	/* Maximum value for UTIL_SignedSize. */
	#define UTIL_SSIZE_MAX UTIL_INTEGER_MAX(UTIL_SignedSize)

	/* Maximum value for size_t. */
	#define UTIL_SIZE_MAX UTIL_INTEGER_MAX(UTIL_UInt64)
/* @NO MRG */
#endif /* FN_INT_H_INC */
/* @END NO MRG */
