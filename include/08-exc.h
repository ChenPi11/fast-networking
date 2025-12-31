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

#ifndef FN_EXC_H_INC
	#define FN_EXC_H_INC
/* @END NO MRG */

/* Initialize EXC module. */
FNAPI extern void EXC_initialize(void);

/* Deinitialize EXC module. */
FNAPI extern void EXC_deinitialize(void);

/* Exception error categories. */
typedef enum EXC_ErrorCategory
{
	/* Libc error category. */
	EXC_CATEGORY_LIBC = 0,
	/* System error category. */
	EXC_CATEGORY_SYS,
	/* Network error category. */
	EXC_CATEGORY_NET,
	/* Error category for this program. */
	EXC_CATEGORY_FN
} EXC_ErrorCategory;

/* Exception error codes for this program. */
typedef enum EXC_Errors
{
	/* Success. */
	EFN_SUCCESS = 0,
	/* Capacity exceeded. */
	EFN_MAX_CAPACITY_EXCEEDED = 1,
	/* Try again. */
	EFN_TRY_AGAIN = 2,
	/* DNS compression pointer unsupported. */
	EFN_COMPRESSION_PTR_UNSUPPORTED = 3,
	/* Invalid hostname. */
	EFN_INVALID_HOSTNAME = 4,
	/* No interfaces joined the multicast group */
	EFN_NO_MULTICAST_JOINED = 5,
	/* Join request refused. */
	EFN_JOIN_REFUSED = 6,

	EFN_MAX
} EXC_Errors;

/* Exception error code. */
typedef struct EXC_ErrorCode
{
	/* Error code. */
	int code;
	/* Error category. */
	enum EXC_ErrorCategory category;
} EXC_ErrorCode;

/* Get last exception code with CATEGORY. */
FNAPI extern EXC_ErrorCode EXC_get_last_error(enum EXC_ErrorCategory category);

/* Clear last exception code with CATEGORY. */
FNAPI extern void EXC_clear_last_error(enum EXC_ErrorCategory category);

/* Get string description of error code. */
FNAPI extern UTIL_ConstString EXC_strerror(struct EXC_ErrorCode error);

/* @NO MRG */
#endif /* FN_EXC_H_INC */
/* @END NO MRG */
