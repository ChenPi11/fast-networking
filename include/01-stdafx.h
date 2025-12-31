
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
#ifndef FN_STDAFX_H_INC
	#define FN_STDAFX_H_INC

	#include "00-config.h" /* IWYU pragma: keep */

/* @END NO MRG */

	#include <ctype.h>  /* IWYU pragma: export */
	#include <errno.h>  /* IWYU pragma: export */
	#include <signal.h> /* IWYU pragma: export */
	#include <stdarg.h>
	#include <stddef.h>
	#include <stdio.h>  /* IWYU pragma: export */
	#include <stdlib.h> /* IWYU pragma: export */
	#include <string.h> /* IWYU pragma: export */
	#include <time.h>   /* IWYU pragma: export */

	#if HAS_WINDOWS
		#include <Windows.h>

		#include <WinSock2.h>

		#include <WS2tcpip.h>

		#include <basetsd.h>
		#include <io.h>

	#elif HAS_UNIX
		#include <arpa/inet.h>
		#include <ifaddrs.h>
		#include <net/if.h>
		#include <netdb.h>
		#include <netinet/in.h>
		#include <stdint.h>
		#include <sys/socket.h>
		#include <sys/types.h>
		#include <unistd.h>
	#else
		#error Unsupported platform. Defined HAS_UNIX to 1 to use POSIX.
	#endif /* HAS_WINDOWS */

	#if defined(_MSC_VER) || (HAS_WINDOWS && defined(__clang__))
		#pragma comment(lib, "ws2_32.lib")
	#endif /* _MSC_VER || (HAS_WINDOWS && defined(__clang__)) */

	#ifndef AF_INET6
		#undef USE_IPv6
		#define USE_IPv6 0
	#endif /* AF_INET6 */

/* @NO MRG */
#endif /* FN_STDAFX_H_INC */
/* @END NO MRG */
