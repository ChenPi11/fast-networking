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
#include "14-map.h"
#include "00-config.h"
#include "04-minmax.h"
/* @END NO MRG */

_FNAPI_EXPORT RESULT_void MAP_create(MAP_Map *map,
									 UTIL_UInt16 init_capacity_power)
{
	RESULT_void ret;

	ret = RESULT_ok_void();

	if (map == NULL) return RESULT_error_void(EINVAL, EXC_CATEGORY_LIBC);

	memset(map, 0, sizeof(MAP_Map));

	init_capacity_power = UTIL_min(UTIL_max(init_capacity_power, 4),
								   14); /* 16 <= capacity <= 16384 */

	map->capacity_power = init_capacity_power;
	map->capacity = 1 << init_capacity_power; /* pow(2, capacity_power) */
	map->size = 0;

	/* Allocate memory for bucket array. */
	map->buckets = (MAP_Node **)calloc(map->capacity, sizeof(MAP_Node *));
	if (!map->buckets)
	{
		ret = RESULT_error_void(ENOMEM, EXC_CATEGORY_LIBC);
		goto FAIL;
	}

	return ret;
FAIL:
	free(map->buckets);
	map->buckets = NULL;
	return ret;
}

/* (Internal) Resize the MAP to double its current capacity. */
static RESULT_void MAP__resize(MAP_Map *map)
{
	RESULT_void ret;
	UTIL_UInt16 old_capacity;
	MAP_Node **old_buckets;
	UTIL_UInt16 idx;

	ret = RESULT_ok_void();
	old_buckets = NULL;

	if (map->capacity_power >= 14)
	{
		/* Max capacity reached. */
		ret = RESULT_error_void(EFN_MAX_CAPACITY_EXCEEDED, EXC_CATEGORY_FN);
		goto EXIT;
	}

	if (map->size < map->capacity)
	{
		/* No need to resize. */
		goto EXIT;
	}

	old_capacity = map->capacity;
	old_buckets = map->buckets;

	/* Calculate new capacity (double). */
	map->capacity_power++;
	map->capacity = 1 << map->capacity_power;

	/* Allocate memory for new bucket array. */
	map->buckets = (MAP_Node **)calloc(map->capacity, sizeof(MAP_Node *));
	if (map->buckets == NULL)
	{
		map->buckets = old_buckets;
		map->capacity = old_capacity;
		map->capacity_power--;
		ret = RESULT_error_void(ENOMEM, EXC_CATEGORY_LIBC);
		goto EXIT;
	}

	/* Rehash all elements. */
	for (idx = 0; idx < old_capacity; idx++)
	{
		MAP_Node *node;

		node = old_buckets[idx];
		while (node)
		{
			MAP_Node *next;
			UTIL_UInt16 new_index;

			next = node->next;

			/* Reuse cached hash. */
			new_index = node->hash & (map->capacity - 1);

			/* Insert into new bucket (head insert). */
			node->next = map->buckets[new_index];
			map->buckets[new_index] = node;

			node = next;
		}
	}

	free(old_buckets);
EXIT:
	return ret;
}

/* (Internal) Create a new MAP_Node with the given KEY, VALUE, and HASH. If
   error, set libc error and return NULL. */
static RESULT_MAP_NodePtr MAP__create_node(UTIL_StringBuffer key, void *value,
										   UTIL_UInt16 hash)
{
	MAP_Node *node;

	if (key == NULL || value == NULL)
		return RESULT_error_MAP_NodePtr(EINVAL, EXC_CATEGORY_LIBC);

	node = (MAP_Node *)malloc(sizeof(MAP_Node));
	if (node == NULL)
		return RESULT_error_MAP_NodePtr(ENOMEM, EXC_CATEGORY_LIBC);

	node->key = key;

	node->value = value;
	node->hash = hash;
	node->next = NULL;

	return RESULT_ok_MAP_NodePtr(node);
}

_FNAPI_EXPORT RESULT_void MAP_set(MAP_Map *map, UTIL_StringBuffer key,
								  void *value)
{
	RESULT_void ret;
	UTIL_UInt16 hash;
	UTIL_UInt16 index;
	MAP_Node *node;
	MAP_Node *new_node;
	RESULT_MAP_NodePtr new_node_res;

	UTIL_initialize();
	ret = RESULT_ok_void();
	new_node = NULL;

	if (map == NULL || key == NULL || value == NULL)
		return RESULT_error_void(EINVAL, EXC_CATEGORY_LIBC);

	/* Check if resizing is needed. */
	ret = MAP__resize(map);
	if (!ret.has_value) return ret;

	/* Calculate hash and index. */
	hash = UTIL_str_hash(key, map->capacity);
	index = hash; /* Already masked. */

	/* Check if key already exists. */
	node = map->buckets[index];
	while (node)
	{
		if (node->hash == hash && strcmp(node->key, key) == 0)
		{
			if (node->value != value)
			{
				free(node->value);
			}
			if (node->key != key)
			{
				free(node->key);
			}
			node->key = key;
			node->value = value;
			return ret; /* Updated existing key. */
		}
		node = node->next;
	}

	/* Create new node. */
	new_node_res = MAP__create_node(key, value, hash);
	if (!new_node_res.has_value)
		return RESULT_error_void_struct(new_node_res.result.error);

	new_node = new_node_res.result.value;

	/* Insert new node at the head of the bucket. */
	new_node->next = map->buckets[index];
	map->buckets[index] = new_node;
	map->size++;

	return ret;
}

_FNAPI_EXPORT const void *MAP_get(const MAP_Map *map, UTIL_ConstString key)
{
	UTIL_UInt16 hash;
	UTIL_UInt16 index;
	MAP_Node *node;

	UTIL_initialize();
	if (map == NULL || key == NULL) return NULL;

	hash = UTIL_str_hash(key, map->capacity);
	index = hash;

	node = map->buckets[index];
	while (node)
	{
		if (node->hash == hash && strcmp(node->key, key) == 0)
		{
			return node->value;
		}
		node = node->next;
	}

	return NULL; /* Not found. */
}

_FNAPI_EXPORT UTIL_Bool MAP_contains(const MAP_Map *map, UTIL_ConstString key)
{
	UTIL_UInt16 hash;
	UTIL_UInt16 index;
	MAP_Node *node;

	UTIL_initialize();
	if (map == NULL || key == NULL) return UTIL_FALSE;

	hash = UTIL_str_hash(key, map->capacity);
	index = hash;

	node = map->buckets[index];
	while (node)
	{
		if (node->hash == hash && strcmp(node->key, key) == 0)
		{
			return UTIL_TRUE;
		}
		node = node->next;
	}

	return UTIL_FALSE; /* Not found. */
}

_FNAPI_EXPORT void MAP_remove(MAP_Map *map, UTIL_ConstString key)
{
	UTIL_UInt16 hash;
	UTIL_UInt16 index;
	MAP_Node *prev;
	MAP_Node *node;

	UTIL_initialize();
	if (map == NULL || key == NULL) return;

	hash = UTIL_str_hash(key, map->capacity);
	index = hash;

	prev = NULL;
	node = map->buckets[index];

	while (node)
	{
		if (node->hash == hash && strcmp(node->key, key) == 0)
		{
			/* Found node to remove. */
			if (prev)
			{
				prev->next = node->next;
			}
			else
			{
				map->buckets[index] = node->next;
			}

			/* Free node resources. */
			free(node->key);
			free(node->value);
			free(node);
			map->size--;

			return;
		}

		prev = node;
		node = node->next;
	}
}

_FNAPI_EXPORT void MAP_clear(MAP_Map *map)
{
	UTIL_UInt16 i;

	if (map == NULL) return;

	for (i = 0; i < map->capacity; i++)
	{
		MAP_Node *node;

		if (map->buckets == NULL) continue;

		node = map->buckets[i];
		while (node)
		{
			MAP_Node *next;

			if (node == NULL) continue;

			next = node->next;

			free(node->key);
			free(node->value);
			free(node);

			node = next;
		}
		map->buckets[i] = NULL;
	}

	map->size = 0;
}

_FNAPI_EXPORT void MAP_free(MAP_Map *map)
{
	if (map == NULL) return;

	MAP_clear(map);
	if (map->buckets) free(map->buckets);
}

/* Result<MAP_NodePtr>. */
_FNAPI_EXPORT RESULT_MAP_NodePtr
RESULT_error_MAP_NodePtr(int code, enum EXC_ErrorCategory category)
{
	RESULT_MAP_NodePtr result;
	result.has_value = UTIL_FALSE;
	result.result.error.code = code;
	result.result.error.category = category;
	return result;
}
_FNAPI_EXPORT RESULT_MAP_NodePtr
RESULT_error_MAP_NodePtr_struct(struct EXC_ErrorCode error)
{
	RESULT_MAP_NodePtr result;
	result.has_value = UTIL_FALSE;
	result.result.error = error;
	return result;
}
_FNAPI_EXPORT RESULT_MAP_NodePtr RESULT_ok_MAP_NodePtr(MAP_NodePtr value)
{
	RESULT_MAP_NodePtr result;
	result.has_value = UTIL_TRUE;
	result.result.value = value;
	return result;
}
