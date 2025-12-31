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
#include "01-stdafx.h"
#include "03-bool.h"
#include "08-exc.h"
#include "10-thread-local.h"

/* @END NO MRG */

#ifndef _WIN64
	/* Windows XP's SDK use DWORD_PTR, but we need support Windows NT 4.0 SDK
	   (VC6.0). Old Windows NT do support DWORD_PTR. So use DWORD (uint32)
	   instead. */
	#define DWORD_PTR unsigned long
#endif

UTIL_THREAD_LOCAL static UTIL_ConstString g_EXC__error_messages[EFN_MAX];
UTIL_THREAD_LOCAL static UTIL_Bool g_EXC__initialized = UTIL_FALSE;

static void EXC__initialize_error_messages()
{
	g_EXC__error_messages[EFN_MAX_CAPACITY_EXCEEDED] =
		(UTIL_ConstString) "Maximum capacity exceeded.";
	g_EXC__error_messages[EFN_TRY_AGAIN] = (UTIL_ConstString) "Try again.";
	g_EXC__error_messages[EFN_COMPRESSION_PTR_UNSUPPORTED] =
		(UTIL_ConstString) "DNS compression pointer unsupported.";
	g_EXC__error_messages[EFN_INVALID_HOSTNAME] =
		(UTIL_ConstString) "Invalid hostname.";
	g_EXC__error_messages[EFN_NO_MULTICAST_JOINED] =
		(UTIL_ConstString) "No interfaces joined the multicast group.";
	g_EXC__error_messages[EFN_JOIN_REFUSED] =
		(UTIL_ConstString) "Join request refused.";
}

#if HAS_SAFE_C_LIB
UTIL_THREAD_LOCAL static char g_annexk_libc_error_msg[1024];

static const char *EXC__my_strerror(int errnum)
{
	if (strerror_s(g_annexk_libc_error_msg, sizeof(g_annexk_libc_error_msg),
				   errnum) == 0)
	{
		return g_annexk_libc_error_msg;
	}
	return "Unknown libc error.";
}

	#define strerror EXC__my_strerror
#endif /* HAS_SAFE_C_LIB */

#if HAS_WINDOWS
UTIL_THREAD_LOCAL static LPSTR g_sys_error_msg = NULL;

static void EXC__clear_sys_error_msg()
{
	if (g_sys_error_msg != NULL)
	{
		LocalFree(g_sys_error_msg);
		g_sys_error_msg = NULL;
	}
}

/* Notice: return is system-allocated. Use LocalFree. */
static LPSTR EXC__not_found_errno_hexstr(LPSTR error_code_str,
										 LPSTR module_path)
{
	LPSTR not_found_msg_buffer;
	DWORD not_found_msg_buffer_length;
	LPSTR final_msg_buffer;
	DWORD_PTR args[2];
	DWORD conv_ret;

	not_found_msg_buffer = NULL;
	not_found_msg_buffer_length = 0;
	final_msg_buffer = NULL;
	conv_ret = 0;
	memset(args, 0, sizeof(args));

	/* Step 1: Get the "Message id %1 not found in %2" message to include in our
	   output. */
	not_found_msg_buffer_length = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, ERROR_MR_MID_NOT_FOUND, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)(&not_found_msg_buffer), 0, NULL);
	if (not_found_msg_buffer_length == 0) goto FAIL;

	/* Step 2: Format our final message including the not found message. */
	args[0] = (DWORD_PTR)error_code_str;
	args[1] = (DWORD_PTR)module_path;
	conv_ret = FormatMessageA(FORMAT_MESSAGE_FROM_STRING |
								  FORMAT_MESSAGE_ARGUMENT_ARRAY |
								  FORMAT_MESSAGE_ALLOCATE_BUFFER,
							  not_found_msg_buffer, 0, 0,
							  (LPSTR)(&final_msg_buffer), 0, (va_list *)(args));
	if (conv_ret == 0) goto FAIL;

	/* Step 3: Clean up and return the final message. */
	LocalFree((HLOCAL)not_found_msg_buffer);
	return final_msg_buffer;
FAIL:
	if (not_found_msg_buffer != NULL) LocalFree((HLOCAL)not_found_msg_buffer);
	if (final_msg_buffer != NULL) LocalFree((HLOCAL)final_msg_buffer);
	return NULL;
}

static void EXC__ev_tohex(unsigned int ev, char *buf)
{
	/* buf is guaranteed non-NULL and large enough */
	static const char hexdig[] = "0123456789ABCDEF";

	char tmp[2 * sizeof(unsigned int)];
	int i;
	int j;
	int len;

	/* Special-case zero. */
	if (ev == 0)
	{
		buf[0] = '0';
		buf[1] = '\0';
		return;
	}

	/* Convert into a temporary buffer from the end, then copy out. */
	i = (int)sizeof(tmp);
	while (ev != 0)
	{
		tmp[--i] = hexdig[ev & 0xF];
		ev >>= 4;
	}

	len = (int)sizeof(tmp) - i;
	for (j = 0; j < len; ++j)
	{
		buf[j] = tmp[i + j];
	}
	buf[len] = '\0';
}

static LPSTR EXC__not_found_errno(int ev)
{
	char hexev[2 * sizeof(unsigned int)];
	char module_path[MAX_PATH];

	memset(module_path, 0, sizeof(module_path));
	EXC__ev_tohex(ev, hexev);

	(void)GetModuleFileNameA(NULL, module_path, MAX_PATH);

	return EXC__not_found_errno_hexstr(hexev, module_path);
}

static LPSTR EXC__get_sys_error_msg(int error_code)
{
	DWORD format_result;

	EXC__clear_sys_error_msg();

	format_result = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
		LoadLibraryA("WS2_32.dll"), (DWORD)error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)(&g_sys_error_msg), 0, NULL);
	if (format_result == 0)
	{
		/* Could not find the message, try to get a better one. */
		g_sys_error_msg = EXC__not_found_errno(error_code);
	}

	return g_sys_error_msg;
}

static EXC_ErrorCode EXC__libc_get_last_error_code()
{
	EXC_ErrorCode error;

	#ifdef _doserrno
	error.code = _doserrno;
	error.category = EXC_CATEGORY_SYS;
	if (error.code != 0) return error;
	if (errno == 0) return error;
	#endif /* _doserrno */

	error.code = errno;
	error.category = EXC_CATEGORY_LIBC;
	return error;
}

_FNAPI_EXPORT EXC_ErrorCode EXC_get_last_error(enum EXC_ErrorCategory category)
{
	EXC_ErrorCode error;

	EXC_initialize();
	memset(&error, 0, sizeof(EXC_ErrorCode));

	switch (category)
	{
	case EXC_CATEGORY_SYS: {
		error.code = (int)GetLastError();
		error.category = EXC_CATEGORY_SYS;
		return error;
	}
	case EXC_CATEGORY_LIBC: {
		error = EXC__libc_get_last_error_code();
		return error;
	}
	case EXC_CATEGORY_NET: {
		error.code = WSAGetLastError();
		error.category = EXC_CATEGORY_NET;
		return error;
	}
	case EXC_CATEGORY_FN: {
		error.code = 0;
		error.category = EXC_CATEGORY_FN;
		return error;
	}
	}

	return error; /* Should not reach here. */
}

_FNAPI_EXPORT void EXC_clear_last_error(enum EXC_ErrorCategory category)
{
	EXC_initialize();

	switch (category)
	{
	case EXC_CATEGORY_SYS: {
		SetLastError(ERROR_SUCCESS);
		EXC__clear_sys_error_msg();
		return;
	}
	case EXC_CATEGORY_LIBC: {
		errno = 0;
	#ifdef _doserrno
		_doserrno = ERROR_SUCCESS;
	#endif
		return;
	}
	case EXC_CATEGORY_NET: {
		WSASetLastError(ERROR_SUCCESS);
		return;
	}
	case EXC_CATEGORY_FN: {
		return;
	}
	}
}

_FNAPI_EXPORT UTIL_ConstString EXC_strerror(struct EXC_ErrorCode error)
{
	switch (error.category)
	{
	case EXC_CATEGORY_LIBC: {
		return (UTIL_ConstString)strerror(error.code);
	}
	case EXC_CATEGORY_SYS:
	case EXC_CATEGORY_NET: {
		return (UTIL_ConstString)EXC__get_sys_error_msg(error.code);
	}
	case EXC_CATEGORY_FN: {
		if (error.code >= 0 && error.code < EFN_MAX)
		{
			UTIL_ConstString msg = g_EXC__error_messages[error.code];
			if (msg != NULL) return msg;
		}
		break;
	}
	}

	return (UTIL_ConstString) "Unknown error.";
}
#else
_FNAPI_EXPORT EXC_ErrorCode EXC_get_last_error(enum EXC_ErrorCategory category)
{
	EXC_ErrorCode error;

	error.code = errno;
	error.category = category;

	return error;
}

_FNAPI_EXPORT void EXC_clear_last_error(enum EXC_ErrorCategory category)
{
	errno = 0;
}

_FNAPI_EXPORT UTIL_ConstString EXC_strerror(struct EXC_ErrorCode error)
{
	return (UTIL_ConstString)strerror(error.code);
}
#endif /* HAS_WINDOWS */

_FNAPI_EXPORT void EXC_initialize(void)
{
	if (g_EXC__initialized) return;

#if HAS_WINDOWS
	EXC__clear_sys_error_msg();
#endif /* HAS_WINDOWS */
	EXC__initialize_error_messages();
	g_EXC__initialized = UTIL_TRUE;
}

_FNAPI_EXPORT void EXC_deinitialize(void)
{
	if (!g_EXC__initialized) return;

#if HAS_WINDOWS
	EXC__clear_sys_error_msg();
#endif /* HAS_WINDOWS */
	g_EXC__initialized = UTIL_FALSE;
}
