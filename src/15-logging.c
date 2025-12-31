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
#include "15-logging.h"
#include "00-config.h"
#include "03-bool.h"
#include "10-thread-local.h"
#include "11-utils.h"
#include "13-fmt.h"

/* @END NO MRG */

UTIL_THREAD_LOCAL static LOGGING_LoggerHandler g_LOGGING__logger_handler;
UTIL_THREAD_LOCAL static LOGGING_LogLevel g_LOGGING__current_level =
	LOG_LEVEL_DEBUG;

static void LOGGING__set_cur_level(LOGGING_LogLevel level)
{
	g_LOGGING__current_level = level;
}

static UTIL_Bool LOGGING__is_valid_level(LOGGING_LogLevel level)
{
	return (level > LOG_LEVEL_INVALID && level < LOG_LEVEL_MAX) ? UTIL_TRUE
																: UTIL_FALSE;
}

_FNAPI_EXPORT void LOGGING_set_level(LOGGING_LogLevel level)
{
	LOGGING_initialize();

	if (LOGGING__is_valid_level(level) &&
		g_LOGGING__logger_handler.set_level != NULL)
	{
		g_LOGGING__logger_handler.set_level(level);
	}
}

_FNAPI_EXPORT UTIL_ConstString
LOGGING_log_level_tostring(LOGGING_LogLevel level)
{
	switch (level)
	{
	case LOG_LEVEL_DEBUG:
		return "DEBUG";
	case LOG_LEVEL_INFO:
		return "INFO";
	case LOG_LEVEL_WARN:
		return "WARN";
	case LOG_LEVEL_ERROR:
		return "ERROR";
	default:
		return "UNKNOWN";
	}
}

_FNAPI_EXPORT void LOGGING_debug(UTIL_ConstString subject, UTIL_ConstString fmt,
								 ...)
{
	va_list args;

	LOGGING_initialize();

	va_start(args, fmt);
	if (g_LOGGING__logger_handler.debug_handler != NULL)
		g_LOGGING__logger_handler.debug_handler(subject, fmt, args);
	va_end(args);
}

_FNAPI_EXPORT void LOGGING_info(UTIL_ConstString subject, UTIL_ConstString fmt,
								...)
{
	va_list args;

	LOGGING_initialize();

	va_start(args, fmt);
	if (g_LOGGING__logger_handler.info_handler != NULL)
		g_LOGGING__logger_handler.info_handler(subject, fmt, args);
	va_end(args);
}

_FNAPI_EXPORT void LOGGING_warn(UTIL_ConstString subject, UTIL_ConstString fmt,
								...)
{
	va_list args;

	LOGGING_initialize();

	va_start(args, fmt);
	if (g_LOGGING__logger_handler.warn_handler != NULL)
		g_LOGGING__logger_handler.warn_handler(subject, fmt, args);
	va_end(args);
}

_FNAPI_EXPORT void LOGGING_error(UTIL_ConstString subject, UTIL_ConstString fmt,
								 ...)
{
	va_list args;

	LOGGING_initialize();

	va_start(args, fmt);
	if (g_LOGGING__logger_handler.error_handler != NULL)
		g_LOGGING__logger_handler.error_handler(subject, fmt, args);
	va_end(args);
}

#if HAS_WINDOWS
UTIL_THREAD_LOCAL static HANDLE g_LOGGING__fd = INVALID_HANDLE_VALUE;

static UTIL_Bool LOGGING__isatty(void)
{
	DWORD mode;

	if (GetConsoleMode(g_LOGGING__fd, &mode))
	{
		return UTIL_TRUE;
	}

	return UTIL_FALSE;
}

enum LOGGING__Color
{
	LOGGING_COLOR_WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
	LOGGING_COLOR_RED = FOREGROUND_RED | FOREGROUND_INTENSITY,
	LOGGING_COLOR_YELLOW =
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
	LOGGING_COLOR_CYAN =
		FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
};

static void LOGGING__set_text_color(enum LOGGING__Color color)
{
	if (!LOGGING__isatty()) return;

	/* We can use ANSI escape to set text color. But Windows NT 4.0 don't
	   support it. */
	SetConsoleTextAttribute(g_LOGGING__fd, color);
}

static void LOGGING__writelog(UTIL_ConstString msg)
{
	DWORD written;

	if (g_LOGGING__fd == INVALID_HANDLE_VALUE) return;

	(void)WriteFile(g_LOGGING__fd, msg, (DWORD)strlen(msg), &written, NULL);
}
#else
static FILE *g_LOGGING__fd = NULL;

static UTIL_Bool LOGGING__isatty(void)
{
	return isatty(fileno(g_LOGGING__fd));
}

static const char *LOGGING_COLOR_WHITE = "\x1b[0;37m";
static const char *LOGGING_COLOR_RED = "\x1b[0;31m";
static const char *LOGGING_COLOR_YELLOW = "\x1b[0;33m";
static const char *LOGGING_COLOR_CYAN = "\x1b[0;36m";

static void LOGGING__set_text_color(const char *color)
{
	if (!LOGGING__isatty()) return;

	fputs(color, g_LOGGING__fd);
	fflush(g_LOGGING__fd);
}

static void LOGGING__writelog(UTIL_ConstString msg)
{
	fputs(msg, g_LOGGING__fd);
	fflush(g_LOGGING__fd);
}
#endif /* HAS_WINDOWS */

static void LOGGING__logging_handler_v(LOGGING_LogLevel level,
									   UTIL_ConstString subject,
									   UTIL_ConstString fmt, va_list args)
{
	UTIL_StringBuffer buffer;
	UTIL_StringBuffer buffer2;
	UTIL_Time now;

	buffer = NULL;
	buffer2 = NULL;
	now = UTIL_get_current_time_ms();

	if (!FMT_vformat(&buffer, fmt, args).has_value) goto EXIT;

	if (!FMT_format(&buffer2, LOGGING_DEFAULT_FORMAT, FMT_str(subject),
					FMT_str(LOGGING_log_level_tostring(level)),
					FMT_uint64_autoalign((UTIL_UInt64)now), FMT_str(buffer))
			 .has_value)
		goto EXIT;

	LOGGING__writelog(buffer2);

EXIT:
	free(buffer);
	free(buffer2);
}

static void LOGGING__debug_handler(UTIL_ConstString subject,
								   UTIL_ConstString fmt, va_list args)
{
	if (g_LOGGING__current_level > LOG_LEVEL_DEBUG) return;

	LOGGING__set_text_color(LOGGING_COLOR_CYAN);
	LOGGING__logging_handler_v(LOG_LEVEL_DEBUG, subject, fmt, args);
	LOGGING__set_text_color(LOGGING_COLOR_WHITE);
}

static void LOGGING__info_handler(UTIL_ConstString subject,
								  UTIL_ConstString fmt, va_list args)
{
	if (g_LOGGING__current_level > LOG_LEVEL_INFO) return;

	LOGGING__set_text_color(LOGGING_COLOR_WHITE);
	LOGGING__logging_handler_v(LOG_LEVEL_INFO, subject, fmt, args);
	LOGGING__set_text_color(LOGGING_COLOR_WHITE);
}

static void LOGGING__warn_handler(UTIL_ConstString subject,
								  UTIL_ConstString fmt, va_list args)
{
	if (g_LOGGING__current_level > LOG_LEVEL_WARN) return;

	LOGGING__set_text_color(LOGGING_COLOR_YELLOW);
	LOGGING__logging_handler_v(LOG_LEVEL_WARN, subject, fmt, args);
	LOGGING__set_text_color(LOGGING_COLOR_WHITE);
}

static void LOGGING__error_handler(UTIL_ConstString subject,
								   UTIL_ConstString fmt, va_list args)
{
	if (g_LOGGING__current_level > LOG_LEVEL_ERROR) return;

	LOGGING__set_text_color(LOGGING_COLOR_RED);
	LOGGING__logging_handler_v(LOG_LEVEL_ERROR, subject, fmt, args);
	LOGGING__set_text_color(LOGGING_COLOR_WHITE);
}

UTIL_THREAD_LOCAL static UTIL_Bool g_LOGGING__is_initialized = UTIL_FALSE;

_FNAPI_EXPORT void LOGGING_initialize(void)
{
	if (g_LOGGING__is_initialized) return;

#if HAS_WINDOWS
	g_LOGGING__fd = GetStdHandle(STD_ERROR_HANDLE);
#else
	g_LOGGING__fd = stderr;
#endif /* HAS_WINDOWS */

	g_LOGGING__logger_handler.debug_handler = LOGGING__debug_handler;
	g_LOGGING__logger_handler.info_handler = LOGGING__info_handler;
	g_LOGGING__logger_handler.warn_handler = LOGGING__warn_handler;
	g_LOGGING__logger_handler.error_handler = LOGGING__error_handler;
	g_LOGGING__logger_handler.set_level = LOGGING__set_cur_level;

	g_LOGGING__current_level = LOG_LEVEL_DEBUG;

	g_LOGGING__is_initialized = UTIL_TRUE;
}

_FNAPI_EXPORT void LOGGING_deinitialize(void)
{
	if (!g_LOGGING__is_initialized) return;

#if HAS_WINDOWS
	g_LOGGING__fd = INVALID_HANDLE_VALUE;
#else
	g_LOGGING__fd = NULL;
#endif /* HAS_WINDOWS */

	g_LOGGING__logger_handler.debug_handler = NULL;
	g_LOGGING__logger_handler.info_handler = NULL;
	g_LOGGING__logger_handler.warn_handler = NULL;
	g_LOGGING__logger_handler.error_handler = NULL;
	g_LOGGING__logger_handler.set_level = NULL;

	g_LOGGING__current_level = LOG_LEVEL_DEBUG;

	g_LOGGING__is_initialized = UTIL_FALSE;

	LOGGING__set_text_color(LOGGING_COLOR_WHITE);
}
