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
#include "03-bool.h"
#include "06-int.h"
#include "08-exc.h"

#ifndef FN_GENERIC_TYPES_H_INC
	#define FN_GENERIC_TYPES_H_INC
/* @END NO MRG */

	#if 0 /* Generic template. */
/* Result<$TYPE>. */
typedef struct RESULT_$TYPE
{
	UTIL_Bool has_value;
	union {
		EXC_ErrorCode error;
		$TYPE value;
	} result;
} RESULT_$TYPE;
FNAPI extern RESULT_$TYPE RESULT_error_$TYPE(int code, EXC_ErrorCategory category);
FNAPI extern RESULT_$TYPE RESULT_error_$TYPE_struct(EXC_ErrorCode error);
FNAPI extern RESULT_$TYPE RESULT_ok_$TYPE($TYPE value);
	#endif

/* Result<UTIL_SignedSize>. */
typedef struct RESULT_UTIL_SignedSize
{
	UTIL_Bool has_value;
	union {
		EXC_ErrorCode error;
		UTIL_SignedSize value;
	} result;
} RESULT_UTIL_SignedSize;
FNAPI extern RESULT_UTIL_SignedSize RESULT_error_UTIL_SignedSize(
	int code, EXC_ErrorCategory category);
FNAPI extern RESULT_UTIL_SignedSize RESULT_error_UTIL_SignedSize_struct(
	EXC_ErrorCode error);
FNAPI extern RESULT_UTIL_SignedSize RESULT_ok_UTIL_SignedSize(
	UTIL_SignedSize value);

/* Result<UTIL_Time>. */
typedef struct RESULT_UTIL_Time
{
	UTIL_Bool has_value;
	union {
		EXC_ErrorCode error;
		UTIL_Time value;
	} result;
} RESULT_UTIL_Time;
FNAPI extern RESULT_UTIL_Time RESULT_error_UTIL_Time(
	int code, EXC_ErrorCategory category);
FNAPI extern RESULT_UTIL_Time RESULT_error_UTIL_Time_struct(
	EXC_ErrorCode error);
FNAPI extern RESULT_UTIL_Time RESULT_ok_UTIL_Time(UTIL_Time value);

/* Result<size_t>. */
typedef struct RESULT_size_t
{
	UTIL_Bool has_value;
	union {
		EXC_ErrorCode error;
		size_t value;
	} result;
} RESULT_size_t;
FNAPI extern RESULT_size_t RESULT_error_size_t(int code,
											   EXC_ErrorCategory category);
FNAPI extern RESULT_size_t RESULT_error_size_t_struct(
	EXC_ErrorCode error);
FNAPI extern RESULT_size_t RESULT_ok_size_t(size_t value);

/* Result<UTIL_ConstString>. */
typedef struct RESULT_UTIL_ConstString
{
	UTIL_Bool has_value;
	union {
		EXC_ErrorCode error;
		UTIL_ConstString value;
	} result;
} RESULT_UTIL_ConstString;
FNAPI extern RESULT_UTIL_ConstString RESULT_error_UTIL_ConstString(
	int code, EXC_ErrorCategory category);
FNAPI extern RESULT_UTIL_ConstString RESULT_error_UTIL_ConstString_struct(
	EXC_ErrorCode error);
FNAPI extern RESULT_UTIL_ConstString RESULT_ok_UTIL_ConstString(
	UTIL_ConstString value);

/* Result<void>. */
typedef struct RESULT_Tag_void
{
	UTIL_Bool has_value;
	EXC_ErrorCode error;
} RESULT_void;
FNAPI extern RESULT_void RESULT_error_void(int code,
										   EXC_ErrorCategory category);
FNAPI extern RESULT_void RESULT_error_void_struct(EXC_ErrorCode error);
FNAPI extern RESULT_void RESULT_ok_void(void);

	#define RESULT_ERRNO(ResultType, category)                                 \
		RESULT_error_##ResultType##_struct(EXC_get_last_error(category))

/* @NO MRG */
#endif /* FN_GENERIC_TYPES_H_INC */
/* @END NO MRG */
