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
#include "01-stdafx.h" /* IWYU pragma: keep */
/* @END NO MRG */

/* va_copy implementation for various platforms. */
#ifndef va_copy
	/* Prefer compiler provided forms first. */
	#ifdef __va_copy
		#define va_copy(dest, src) __va_copy((dest), (src))
	#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
		#define va_copy(dest, src) __builtin_va_copy((dest), (src))
	#elif defined(_MSC_VER)
		/* MSVC's va_list is assignable on known targets. */
		#define va_copy(dest, src) ((dest) = (src))
	#else
		/* Last resort: memcpy the underlying object. Works on many real-world
		   impls, but strictly speaking C89 doesn't guarantee it's portable for
		   all va_list representations (e.g. if va_list is an array type). */
		#ifndef FN_ALLOW_UNSAFE_VA_COPY
			#define va_copy(dest, src) memcpy(&(dest), &(src), sizeof(va_list))
		#else
			#error No va_copy implementation available on this platform. \
			You should update to C99 or later, \
			or define FN_ALLOW_UNSAFE_VA_COPY to force an unsafe \
			memcpy-based implementation.
		#endif /* FN_ALLOW_UNSAFE_VA_COPY */
	#endif     /* __va_copy */
#endif         /* va_copy */
