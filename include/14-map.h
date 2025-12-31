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

#ifndef FN_MAP_H_INC
	#define FN_MAP_H_INC
/* @END NO MRG */

/* A string:any map node. */
typedef struct MAP_Node
{
	UTIL_StringBuffer key; /* Malloc-allocated string key. */
	void *value;           /* Malloc-allocated value. */
	struct MAP_Node
		*next; /* Next node in bucket (linked list for collision resolution). */
	UTIL_UInt16 hash; /* Cached hash value to avoid recomputation. */
} MAP_Node;

/* Pointer to MAP_Node. */
typedef MAP_Node *MAP_NodePtr;

/* Result<MAP_NodePtr>. */
typedef struct RESULT_MAP_NodePtr
{
	UTIL_Bool has_value;
	union {
		struct EXC_ErrorCode error;
		MAP_NodePtr value;
	} result;
} RESULT_MAP_NodePtr;
FNAPI extern RESULT_MAP_NodePtr RESULT_error_MAP_NodePtr(
	int code, enum EXC_ErrorCategory category);
FNAPI extern RESULT_MAP_NodePtr RESULT_error_MAP_NodePtr_struct(
	struct EXC_ErrorCode error);
FNAPI extern RESULT_MAP_NodePtr RESULT_ok_MAP_NodePtr(MAP_NodePtr value);

/* A string:any map. */
typedef struct MAP_Map
{
	struct MAP_Node **buckets;  /* Array of buckets. */
	UTIL_UInt16 capacity;       /* Capacity (2^capacity_power). */
	UTIL_UInt16 size;           /* Current number of elements. */
	UTIL_UInt16 capacity_power; /* Exponent of capacity (capacity = 1 <<
								capacity_power). */
} MAP_Map;

/* Create a new MAP_Map to MAP with initial capacity 2^INIT_CAPACITY_POWER.
   Notice that INIT_CAPACITY_POWER must be in range [4, 14], and you should
   pre-allocate memory for the map struct. */
FNAPI extern RESULT_void MAP_create(MAP_Map *map,
									UTIL_UInt16 init_capacity_power);

/* Set a key-value pair in the MAP. If the KEY already exists, update its value.
   Notice that the KEY and VALUE **MUST** be malloc-allocated and the map will
   take ownership of them. */
FNAPI extern RESULT_void MAP_set(MAP_Map *map, UTIL_StringBuffer key,
								 void *value);

/* Get the value associated with the given KEY in the MAP. Returns NULL if the
   KEY is not found. Don't free the returned value because it is managed by the
   MAP. */
FNAPI extern const void *MAP_get(const MAP_Map *map, UTIL_ConstString key);

/* Check if the MAP contains the given KEY. Returns UTIL_TRUE if found,
   UTIL_FALSE otherwise. */
FNAPI extern UTIL_Bool MAP_contains(const MAP_Map *map, UTIL_ConstString key);

/* Remove the key-value pair with the given KEY from the MAP. If the KEY does
   not exist, nothing will happen. */
FNAPI extern void MAP_remove(MAP_Map *map, UTIL_ConstString key);

/* Clear all key-value pairs in the MAP. But do not free the MAP's bucket array.
 */
FNAPI extern void MAP_clear(MAP_Map *map);

/* Free all resources associated with the MAP. But do not free the MAP structure
   itself. It may be stack-allocated. */
FNAPI extern void MAP_free(MAP_Map *map);

/* @NO MRG */
#endif
/* @END NO MRG */
