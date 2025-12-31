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

#ifndef UTIL_THREAD_LOCAL
	#if defined(_MSVC_LANG) && defined(__cplusplus)
		#define UTIL_CPP_STD _MSVC_LANG
	#elif defined(__cplusplus)
		#define UTIL_CPP_STD __cplusplus
	#endif /* defined(_MSVC_LANG) && defined(__cplusplus) */

	/* Use thread-local storage. Although this program is only single-thread. */
	#if UTIL_CPP_STD >= 201103L
		#define UTIL_THREAD_LOCAL thread_local
	#elif defined(_ISOC11_SOURCE) ||                                           \
		(defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L)
		#define UTIL_THREAD_LOCAL _Thread_local
	#elif defined(__GNUC__) || defined(__clang__) || defined(__ICCARM__) ||    \
		defined(__TINYC__) || defined(__INTEL_LLVM_COMPILER) ||                \
		defined(__INTEL_COMPILER) || defined(__ICC) || defined(__ICPC)
		#define UTIL_THREAD_LOCAL __thread
	#elif defined(_MSC_VER)
		#define UTIL_THREAD_LOCAL __declspec(thread)
	#elif defined(thread_local)
		#define UTIL_THREAD_LOCAL thread_local
	#else
		#define UTIL_THREAD_LOCAL
	#endif

#endif /* UTIL_THREAD_LOCAL */
