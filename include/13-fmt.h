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
#include "06-int.h"
#include "07-string.h"
#include "09-generic-types.h"

#ifndef FN_FMT_H_INC
	#define FN_FMT_H_INC
/* @END NO MRG */

/* Token type. */
typedef enum FMT_TokenType
{
	/* End-of-arguments sentinel. */
	FMT_TOKEN_END = 0,
	/* String token. */
	FMT_TOKEN_STRING,
	/* 0x0000-0xFFFF 4-digit aligned hexadecimal token. For uint16. */
	FMT_TOKEN_HEX16_4ALIGN,
	/* 0-65535 auto-aligned base-10 integer token. For uint16. */
	FMT_TOKEN_UINT16_AUTOALIGN,
	/* 0-18446744073709551615 auto-aligned base-10 integer token. */
	FMT_TOKEN_UINT64_AUTOALIGN,
} FMT_TokenType;

/* Format token, used to provide FMT_format() arguments. */
typedef struct FMT_Token
{
	/* Token type. */
	FMT_TokenType type;
	union {
		/* Integer token. */
		UTIL_UInt16 uint16;
		/* UINT64 token. */
		UTIL_UInt64 uint64;
		/* String token. */
		UTIL_ConstString str;
	} value;
} FMT_Token;

/* End guard token. You MUST pass this as the last argument to FMT_format(). */
static const FMT_Token FMT_END = {FMT_TOKEN_END, {0}};

/* String token. */
FNAPI FMT_Token FMT_str(UTIL_ConstString str);

/* 0x0000-0xFFFF 4-digit aligned hexadecimal token. For uint16. */
FNAPI FMT_Token FMT_hex16_4(UTIL_UInt16 value);

/* 0-65535 auto-aligned base-10 integer token. For uint16. */
FNAPI FMT_Token FMT_uint16_autoalign(UTIL_UInt16 value);

/* 0-18446744073709551615 auto-aligned base-10 integer token. */
FNAPI FMT_Token FMT_uint64_autoalign(UTIL_UInt64 value);

/*
  Simple formatter supporting placeholders %1 .. %9 and escaped percent %%
  Behavior and constraints (design choices explained in comments below):
   - Only supports placeholders %1 .. %9 (single digit).
   - "%%" emits a single '%' and consumes no argument.
   - For safety (to avoid reading past provided varargs) this implementation
	 treats a FMT_END vararg pointer as the end-of-arguments sentinel:
	   e.g. FMT_format("a=%1 b=%2", FMT_str("x"), FMT_str("y"), FMT_END)
	   if the caller provides fewer args and does NOT provide a END sentinel,
	   reading further via va_arg would be undefined. To avoid that undefined
	   behavior, we stop reading args when we see a FMT_END.
	 This means callers who want to pass an actual FMT_END as an argument cannot
	 do so with this implementation (trade-off for safety per request).
   - If a format contains a placeholder %n (1..9) but the caller did not
	 provide that many arguments (we determined provided args by stopping at the
	 first FMT_END or after the highest digit referenced up to 9), then we do
  NOT substitute it; instead we emit the literal characters "%n". This avoids
	 consuming/reading non-existent arguments.

  Note: Because standard C varargs have no built-in count, the safest practical
  approach is to require a FMT_END sentinel if the caller may provide fewer
  arguments than the highest placeholder referenced. Example usage:
	FMT_format("%1-%2-%3", FMT_str("a"), FMT_str("b"), FMT_str("c")); // ok
	FMT_format("%1-%2-%3-%4", FMT_str("a"), FMT_str("b"), FMT_str("c"),
			   FMT_END);   // ok, but %4 won't be formatted.

  The function FMT_vformat formats a string using the given format specifier and
  a va_list of arguments. The formatted string is stored in the buffer pointed
  to by BUFFER. If BUFFER is NULL, the function returns the required buffer size
  (including terminating NUL) without formatting. */
FNAPI extern RESULT_void FMT_vformat(UTIL_StringBuffer *buffer,
									 UTIL_ConstString fmt, va_list ap);

/* Formats a string using the given format specifier and arguments. Stores the
   result in BUFFER. */
FNAPI extern RESULT_void FMT_format(UTIL_StringBuffer *buffer,
									UTIL_ConstString fmt, ...);

/* @NO MRG */
#endif /* FN_FMT_H_INC */
/* @END NO MRG */
