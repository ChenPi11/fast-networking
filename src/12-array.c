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
#include "12-array.h"
#include "00-config.h"
#include "11-utils.h"
/* @END NO MRG */

_FNAPI_EXPORT RESULT_void UTIL_str_array_init(UTIL_StrArray *arr,
											  size_t initial_capacity)
{
	if (arr == NULL || initial_capacity == 0)
		return RESULT_error_void(EINVAL, EXC_CATEGORY_LIBC);

	arr->arr = (UTIL_StringBuffer *)malloc(sizeof(UTIL_StringBuffer) *
										   initial_capacity);
	if (arr->arr == NULL) return RESULT_error_void(ENOMEM, EXC_CATEGORY_LIBC);

	arr->size = 0;
	arr->capacity = initial_capacity;

	return RESULT_ok_void();
}

_FNAPI_EXPORT RESULT_void UTIL_str_array_append(UTIL_StrArray *arr,
												UTIL_ConstString data)
{
	UTIL_StringBuffer *new_arr;
	size_t new_capacity;

	if (arr == NULL || data == NULL)
		return RESULT_error_void(EINVAL, EXC_CATEGORY_LIBC);

	if (arr->size >= arr->capacity)
	{
		new_capacity = arr->capacity * 2;
		new_arr = (UTIL_StringBuffer *)realloc(
			arr->arr, sizeof(UTIL_StringBuffer) * new_capacity);
		if (new_arr == NULL)
			return RESULT_error_void(ENOMEM, EXC_CATEGORY_LIBC);

		arr->arr = new_arr;
		arr->capacity = new_capacity;
	}

	arr->arr[arr->size] = UTIL_strdup(data);
	if (arr->arr[arr->size] == NULL)
		return RESULT_error_void(ENOMEM, EXC_CATEGORY_LIBC);

	arr->size++;
	return RESULT_ok_void();
}

_FNAPI_EXPORT UTIL_Bool UTIL_str_in_array(UTIL_ConstString data,
										  UTIL_StrArray *arr)
{
	size_t i;

	if (data == NULL || arr == NULL) return UTIL_FALSE;

	i = 0;

	while (i < arr->size)
	{
		if (strcmp(data, arr->arr[i]) == 0)
		{
			return UTIL_TRUE;
		}
		i++;
	}

	return UTIL_FALSE;
}

_FNAPI_EXPORT void UTIL_str_array_free(UTIL_StrArray *arr)
{
	size_t i;

	if (arr == NULL) return;

	for (i = 0; i < arr->size; i++)
	{
		free(arr->arr[i]);
	}

	free(arr->arr);
}
