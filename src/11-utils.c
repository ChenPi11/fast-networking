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
#include "11-utils.h"
#include "00-config.h"
#include "01-stdafx.h"
#include "05-register.h"
#include "10-thread-local.h"
#include <stddef.h>

/* @END NO MRG */

_FNAPI_EXPORT void *UTIL_memdup(const void *src, size_t n)
{
	void *dst;

	if (src == NULL || n == 0) return NULL;

	dst = malloc(n);
	if (dst == NULL) return NULL;

	memcpy(dst, src, n);

	return dst;
}

_FNAPI_EXPORT UTIL_StringBuffer UTIL_strdup(const char *src)
{
	return (UTIL_StringBuffer)UTIL_memdup((src), strlen(src) + 1);
}

#if HAS_WINDOWS
_FNAPI_EXPORT void UTIL_sleep_ms(UTIL_Time ms)
{
	if (ms == 0)
	{
		return;
	}

	Sleep((DWORD)ms);
}

_FNAPI_EXPORT UTIL_Time UTIL_get_current_time_ms(void)
{
	FILETIME ft;
	ULARGE_INTEGER uli;

	GetSystemTimeAsFileTime(&ft);
	uli.LowPart = ft.dwLowDateTime;
	uli.HighPart = ft.dwHighDateTime;

	/* Convert to milliseconds. */
	return (UTIL_Time)(uli.QuadPart / 10000);
}
#else
_FNAPI_EXPORT void UTIL_sleep_ms(UTIL_Time ms)
{
	struct timespec ts;

	if ((time_t)ms <= 0)
	{
		return;
	}

	ts.tv_sec = (time_t)(ms / 1000);
	ts.tv_nsec = (time_t)(ms % 1000) * 1000000;

	nanosleep(&ts, NULL);
}

_FNAPI_EXPORT UTIL_Time UTIL_get_current_time_ms(void)
{
	struct timespec ts;

	if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
	{
		perror("WHY clock_gettime FAIL? ABORTING!");
		abort();
	}

	return (UTIL_Time)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
#endif /* HAS_WINDOWS */

_FNAPI_EXPORT void UTIL_timer_start(struct UTIL_Timer *timer)
{
	if (timer == NULL) return;

	timer->start_time = UTIL_get_current_time_ms();
	timer->duration_ms = 0;
}

_FNAPI_EXPORT void UTIL_timer_reset(struct UTIL_Timer *timer)
{
	if (timer == NULL) return;

	timer->start_time = UTIL_get_current_time_ms();
	timer->duration_ms = 0;
}

_FNAPI_EXPORT UTIL_Time UTIL_timer_elapsed_ms(struct UTIL_Timer *timer)
{
	UTIL_Time now;

	if (timer == NULL) return 0;

	now = UTIL_get_current_time_ms();
	return now - timer->start_time;
}

_FNAPI_EXPORT UTIL_Bool UTIL_timer_ontime(struct UTIL_Timer *timer,
										  UTIL_Time duration_ms)
{
	if (timer == NULL) return UTIL_FALSE;

	return UTIL_timer_elapsed_ms(timer) >= duration_ms ? UTIL_TRUE : UTIL_FALSE;
}

/* CRC16-CCITT polynomial. */
static const UTIL_UInt16 UTIL_CRC16_CCITT_POLY = 0x1021;
UTIL_THREAD_LOCAL static UTIL_UInt16 g_UTIL_crc16_table[256];

_FNAPI_EXPORT void UTIL_initialize(void)
{
	int i;
	int j;
	UTIL_UInt16 crc;
	UTIL_THREAD_LOCAL static UTIL_Bool g_UTIL_crc16_table_initialized =
		UTIL_FALSE;

	if (g_UTIL_crc16_table_initialized) return;

	for (i = 0; i < 256; ++i)
	{
		crc = (UTIL_UInt16)i << 8;
		for (j = 0; j < 8; ++j)
		{
			if (crc & 0x8000)
				crc = (UTIL_UInt16)((crc << 1) ^ UTIL_CRC16_CCITT_POLY);
			else
				crc = (UTIL_UInt16)(crc << 1);
		}
		g_UTIL_crc16_table[i] = crc;
	}

	g_UTIL_crc16_table_initialized = UTIL_TRUE;
}

_FNAPI_EXPORT UTIL_UInt16 UTIL_str_hash(UTIL_ConstString key,
										UTIL_UInt16 table_size_2power)
{
	UTIL_register UTIL_UInt16 crc;

	UTIL_initialize();

	crc = 0xFFFF;

	while (*key)
	{
		crc = (UTIL_UInt16)((crc << 8) ^
							g_UTIL_crc16_table[((crc >> 8) ^ (*key++)) &
											   0xFF]); /* Can't understand. */
	}

	/* Only available when table_size_2power is a power of 2. */
	return (UTIL_UInt16)(crc & (table_size_2power - 1));
}

_FNAPI_EXPORT UTIL_Bool UTIL_is_file(UTIL_ConstString path)
{
	FILE *file;
#if HAS_SAFE_C_LIB
	errno_t sc_err;
#endif

	if (path == NULL) return UTIL_FALSE;

	file = NULL;

#if HAS_SAFE_C_LIB
	sc_err = fopen_s(&file, path, "rb");
	if (sc_err != 0) return UTIL_FALSE;
#else
	file = fopen(path, "r");
	if (file == NULL) return UTIL_FALSE;
#endif

	fclose(file);
	return UTIL_TRUE;
}

/* Maximum value for size_t divided by 10. */
static const size_t SIZE_T_MAX_DIV10 = UTIL_INTEGER_MAX(size_t) / 10;

_FNAPI_EXPORT RESULT_size_t UTIL_str_to_unsigned_base10(UTIL_ConstString str)
{
	size_t result;
	int digit;

	if (str == NULL) return RESULT_error_size_t(EINVAL, EXC_CATEGORY_LIBC);

	result = 0;

	/* Skip leading whitespace. */
	while (isspace((int)(unsigned char)*str))
	{
		str++;
	}

	if (*str == '\0')
	{
		/* Empty string or only whitespace. */
		return RESULT_error_size_t(EINVAL, EXC_CATEGORY_LIBC);
	}

	while (*str)
	{
		if (!isdigit((unsigned char)*str))
		{
			/* Encountered non-digit character, return failure. */
			return RESULT_error_size_t(EINVAL, EXC_CATEGORY_LIBC);
		}

		digit = *str - '0'; /* Convert character to digit. */

		/* Check for overflow. */
		if (result > SIZE_T_MAX_DIV10 ||
			(result == SIZE_T_MAX_DIV10 && digit > 7))
		{
			/* Overflow case. */
			return RESULT_error_size_t(ERANGE, EXC_CATEGORY_LIBC);
		}

		/* Update result. */
		result = result * 10 + digit;

		str++;
	}

	return RESULT_ok_size_t(result);
}

_FNAPI_EXPORT size_t UTIL_uint16_to_base10(UTIL_UInt16 value, char *buffer)
{
	char temp[UTIL_UINT16_TO_BASE10_MAX_BUFSIZE];
	int i;
	int j;

	i = 0;

	/* Special case for zero. */
	if (value == 0)
	{
		buffer[0] = '0';
		return 1;
	}

	/* Extract digits in reverse order. */
	while (value > 0)
	{
		temp[i++] = (char)('0' + (value % 10));
		value /= 10;
	}

	/* Reverse digits to get correct order. */
	for (j = 0; j < i; j++)
	{
		buffer[j] = temp[i - 1 - j];
	}

	return (size_t)i;
}

_FNAPI_EXPORT size_t UTIL_uint16_to_base16(UTIL_UInt16 value, char *buffer)
{
	static const char *hex_digits = "0123456789ABCDEF";

	buffer[0] = '0';
	buffer[1] = 'x';
	buffer[2] = hex_digits[(value >> 12) & 0x0F];
	buffer[3] = hex_digits[(value >> 8) & 0x0F];
	buffer[4] = hex_digits[(value >> 4) & 0x0F];
	buffer[5] = hex_digits[value & 0x0F];

	return 6; /* 0x0000-0xFFFF */
}

_FNAPI_EXPORT size_t UTIL_uint64_to_base10(UTIL_UInt64 value, char *buffer)
{
	char temp[UTIL_UINT64_TO_BASE10_MAX_BUFSIZE];
	int i;
	int j;

	i = 0;

	/* Special case for zero. */
	if (value == 0)
	{
		buffer[0] = '0';
		return 1;
	}

	/* Extract digits in reverse order. */
	while (value > 0)
	{
		temp[i++] = (char)('0' + (value % 10));
		value /= 10;
	}

	/* Reverse digits to get correct order. */
	for (j = 0; j < i; j++)
	{
		buffer[j] = temp[i - 1 - j];
	}

	return (size_t)i;
}
