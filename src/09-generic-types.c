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
#include "09-generic-types.h"
/* @END NO MRG */

/* We don't use macro because we may minimize the code size later. */

/* Result<UTIL_SignedSize>. */
_FNAPI_EXPORT RESULT_UTIL_SignedSize
RESULT_error_UTIL_SignedSize(int code, enum EXC_ErrorCategory category)
{
	RESULT_UTIL_SignedSize result;
	result.has_value = UTIL_FALSE;
	result.result.error.code = code;
	result.result.error.category = category;
	return result;
}
_FNAPI_EXPORT RESULT_UTIL_SignedSize
RESULT_error_UTIL_SignedSize_struct(struct EXC_ErrorCode error)
{
	RESULT_UTIL_SignedSize result;
	result.has_value = UTIL_FALSE;
	result.result.error = error;
	return result;
}
_FNAPI_EXPORT RESULT_UTIL_SignedSize
RESULT_ok_UTIL_SignedSize(UTIL_SignedSize value)
{
	RESULT_UTIL_SignedSize result;
	result.has_value = UTIL_TRUE;
	result.result.value = value;
	return result;
}

/* Result<UTIL_Time>. */
_FNAPI_EXPORT RESULT_UTIL_Time
RESULT_error_UTIL_Time(int code, enum EXC_ErrorCategory category)
{
	RESULT_UTIL_Time result;
	result.has_value = UTIL_FALSE;
	result.result.error.code = code;
	result.result.error.category = category;
	return result;
}
_FNAPI_EXPORT RESULT_UTIL_Time
RESULT_error_UTIL_Time_struct(struct EXC_ErrorCode error)
{
	RESULT_UTIL_Time result;
	result.has_value = UTIL_FALSE;
	result.result.error = error;
	return result;
}
_FNAPI_EXPORT RESULT_UTIL_Time RESULT_ok_UTIL_Time(UTIL_Time value)
{
	RESULT_UTIL_Time result;
	result.has_value = UTIL_TRUE;
	result.result.value = value;
	return result;
}

/* Result<size_t>. */
_FNAPI_EXPORT RESULT_size_t RESULT_error_size_t(int code,
												enum EXC_ErrorCategory category)
{
	RESULT_size_t result;
	result.has_value = UTIL_FALSE;
	result.result.error.code = code;
	result.result.error.category = category;
	return result;
}
_FNAPI_EXPORT RESULT_size_t
RESULT_error_size_t_struct(struct EXC_ErrorCode error)
{
	RESULT_size_t result;
	result.has_value = UTIL_FALSE;
	result.result.error = error;
	return result;
}
_FNAPI_EXPORT RESULT_size_t RESULT_ok_size_t(size_t value)
{
	RESULT_size_t result;
	result.has_value = UTIL_TRUE;
	result.result.value = value;
	return result;
}

/* Result<UTIL_ConstString>. */
_FNAPI_EXPORT RESULT_UTIL_ConstString
RESULT_error_UTIL_ConstString(int code, enum EXC_ErrorCategory category)
{
	RESULT_UTIL_ConstString result;
	result.has_value = UTIL_FALSE;
	result.result.error.code = code;
	result.result.error.category = category;
	return result;
}
_FNAPI_EXPORT RESULT_UTIL_ConstString
RESULT_error_UTIL_ConstString_struct(struct EXC_ErrorCode error)
{
	RESULT_UTIL_ConstString result;
	result.has_value = UTIL_FALSE;
	result.result.error = error;
	return result;
}
_FNAPI_EXPORT RESULT_UTIL_ConstString
RESULT_ok_UTIL_ConstString(UTIL_ConstString value)
{
	RESULT_UTIL_ConstString result;
	result.has_value = UTIL_TRUE;
	result.result.value = value;
	return result;
}

/* Result<void>. */
_FNAPI_EXPORT RESULT_void RESULT_error_void(int code,
											enum EXC_ErrorCategory category)
{
	RESULT_void result;
	result.has_value = UTIL_FALSE;
	result.error.code = code;
	result.error.category = category;
	return result;
}
_FNAPI_EXPORT RESULT_void RESULT_error_void_struct(struct EXC_ErrorCode error)
{
	RESULT_void result;
	result.has_value = UTIL_FALSE;
	result.error = error;
	return result;
}
_FNAPI_EXPORT RESULT_void RESULT_ok_void(void)
{
	RESULT_void result;
	result.has_value = UTIL_TRUE;
	return result;
}
