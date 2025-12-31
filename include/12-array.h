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
#include "00-config.h"
#include "07-string.h"
#include "09-generic-types.h"

#ifndef FN_ARRAY_H_INC
	#define FN_ARRAY_H_INC
/* @END NO MRG */

/* String array structure. */
typedef struct UTIL_StrArray
{
	size_t size;            /* Current number of elements. */
	size_t capacity;        /* Current capacity. */
	UTIL_StringBuffer *arr; /* Array of strings. */
} UTIL_StrArray;

/* Initialize string array ARR with initial capacity of INITIAL_CAPACITY. */
FNAPI extern RESULT_void UTIL_str_array_init(UTIL_StrArray *arr,
											 size_t initial_capacity);

/* Append string DATA to string array ARR.*/
FNAPI extern RESULT_void UTIL_str_array_append(UTIL_StrArray *arr,
											   UTIL_ConstString data);

/* Check if DATA is in string array ARR. Return 1 if found, 0 if not found or on
   error. */
FNAPI extern UTIL_Bool UTIL_str_in_array(UTIL_ConstString data,
										 UTIL_StrArray *arr);

/* Free all resources associated with string array ARR. */
FNAPI extern void UTIL_str_array_free(UTIL_StrArray *arr);

/* @NO MRG */
#endif /* FN_ARRAY_H_INC */

/* @END NO MRG */