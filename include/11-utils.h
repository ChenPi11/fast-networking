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
#include "01-stdafx.h" /* IWYU pragma: keep */
#include "07-string.h"
#include "09-generic-types.h"

#ifndef FN_UTILS_H_INC
	#define FN_UTILS_H_INC
/* @END NO MRG */

/* Initialize UTIL module. */
FNAPI extern void UTIL_initialize(void);

/* Duplicate memory block SRC of size N. Return NULL on error. */
FNAPI extern void *UTIL_memdup(const void *src, size_t n);

/* Duplicate string STR. Return NULL on error. */
FNAPI extern UTIL_StringBuffer UTIL_strdup(const char *src);

/* Get current time in milliseconds. */
FNAPI extern UTIL_Time UTIL_get_current_time_ms(void);

/* Sleep for MS milliseconds. */
FNAPI extern void UTIL_sleep_ms(UTIL_Time ms);

/* Timer structure. */
typedef struct UTIL_Timer
{
	UTIL_Time start_time;  /* Start time in milliseconds. */
	UTIL_Time duration_ms; /* Duration in milliseconds. */
} UTIL_Timer;

/* Start timer TIMER. */
FNAPI extern void UTIL_timer_start(struct UTIL_Timer *timer);

/* Reset timer TIMER. */
FNAPI extern void UTIL_timer_reset(struct UTIL_Timer *timer);

/* Get elapsed time in milliseconds since TIMER was started. */
FNAPI extern UTIL_Time UTIL_timer_elapsed_ms(struct UTIL_Timer *timer);

/* Check if TIMER has elapsed DURATION_MS milliseconds. Return UTIL_TRUE if yes,
   UTIL_FALSE if not. */
FNAPI extern UTIL_Bool UTIL_timer_ontime(struct UTIL_Timer *timer,
										 UTIL_Time duration_ms);

/* Compute CRC16-CCITT hash of string KEY. TABLE_SIZE_2POWER is the size of hash
   table (must be a power of 2). It MUST be a power of 2, or undefined behavior
   will occur. */
FNAPI extern UTIL_UInt16 UTIL_str_hash(UTIL_ConstString key,
									   UTIL_UInt16 table_size_2power);

/* Check if PATH is a file. Return UTIL_TRUE if yes, UTIL_FALSE if not. */
FNAPI extern UTIL_Bool UTIL_is_file(UTIL_ConstString path);

/* Convert string STR to unsigned base 10 integer. */
FNAPI extern RESULT_size_t UTIL_str_to_unsigned_base10(UTIL_ConstString str);

/* Convert a unsigned int16 to base-10 string. This function will not apply \0
   and check buffer size. Return the length of string. Maximum 5 digits. */
FNAPI extern size_t UTIL_uint16_to_base10(UTIL_UInt16 value, char *buffer);

	#define UTIL_UINT16_TO_BASE10_MAX_BUFSIZE 5

/* Convert a unsigned int16 to base-16 string. This function will not apply \0
   and check buffer size. But use "0x" prefix. Return the length of string.
   (include "0x" prefix, 6 always.) */
FNAPI extern size_t UTIL_uint16_to_base16(UTIL_UInt16 value, char *buffer);

	#define UTIL_UINT16_TO_BASE16_BUFSIZE 6

/* Convert a unsigned int64 to base-10 string. This function will not apply \0
   and check buffer size. Return the length of string. Maximum 20 digits. */
FNAPI extern size_t UTIL_uint64_to_base10(UTIL_UInt64 value, char *buffer);

	#define UTIL_UINT64_TO_BASE10_MAX_BUFSIZE 20

/* @NO MRG */
#endif /* FN_UTILS_H_INC */
/* @END NO MRG */
