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

#ifndef FN_LOGGING_H_INC
	#define FN_LOGGING_H_INC
/* @END NO MRG */

/* Logging level. */
typedef enum LOGGING_LogLevel
{
	LOG_LEVEL_INVALID = -1,

	/* Debug level. */
	LOG_LEVEL_DEBUG = 0,
	/* Info level. */
	LOG_LEVEL_INFO = 1,
	/* Warning level. */
	LOG_LEVEL_WARN = 2,
	/* Error level. */
	LOG_LEVEL_ERROR = 3,

	LOG_LEVEL_MAX
} LOGGING_LogLevel;

/* Logger handler structure. */
typedef struct LOGGING_LoggerHandler
{
	/* Debug level handler. */
	void (*debug_handler)(UTIL_ConstString subject, UTIL_ConstString fmt,
						  va_list args);

	/* Info level handler. */
	void (*info_handler)(UTIL_ConstString subject, UTIL_ConstString fmt,
						 va_list args);

	/* Warning level handler. */
	void (*warn_handler)(UTIL_ConstString subject, UTIL_ConstString fmt,
						 va_list args);

	/* Error level handler. */
	void (*error_handler)(UTIL_ConstString subject, UTIL_ConstString fmt,
						  va_list args);

	/* Set logging level. */
	void (*set_level)(LOGGING_LogLevel level);
} LOGGING_LoggerHandler;

/* Initialize logging module. */
FNAPI extern void LOGGING_initialize(void);

/* Deinitialize logging module. */
FNAPI extern void LOGGING_deinitialize(void);

/* Set logging level. */
FNAPI extern void LOGGING_set_level(LOGGING_LogLevel level);

/* Convert logging level to string. */
FNAPI extern UTIL_ConstString LOGGING_log_level_tostring(
	LOGGING_LogLevel level);

/* Log a debug message with format FMT. */
FNAPI extern void LOGGING_debug(UTIL_ConstString subject, UTIL_ConstString fmt,
								...);

/* Log an info message with format FMT. */
FNAPI extern void LOGGING_info(UTIL_ConstString subject, UTIL_ConstString fmt,
							   ...);

/* Log a warning message with format FMT. */
FNAPI extern void LOGGING_warn(UTIL_ConstString subject, UTIL_ConstString fmt,
							   ...);

/* Log an error message with format FMT. */
FNAPI extern void LOGGING_error(UTIL_ConstString subject, UTIL_ConstString fmt,
								...);

/* @NO MRG */
#endif
/* @END NO MRG */
