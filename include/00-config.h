/*
 * This file is part of the Fast Networking.
 *
 * The Fast Networking Library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * The Fast Networking Library is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the Fast Networking Library; see the file LICENSE.
 * If not, see <https://www.gnu.org/licenses/>.
 */

/* @NO MRG */
#ifndef FN_CONFIG_H_INC
	#define FN_CONFIG_H_INC
/* @END NO MRG */

	#if defined(__linux__) || defined(__linux) && !defined(_POSIX_C_SOURCE)
		#define _POSIX_C_SOURCE 200112L
	#endif /* __linux__ */

	#if !defined(_GNU_SOURCE) && !defined(FN_NO_GNU_SOURCE)
		#define _GNU_SOURCE 1
	#endif /* !defined(_GNU_SOURCE) && !defined(FN_NO_GNU_SOURCE) */

	#if !defined(_GNU_SOURCE) && !defined(FN_NO_GNU_SOURCE)
		#define _GNU_SOURCE 1
	#endif /* !defined(_GNU_SOURCE) && !defined(FN_NO_GNU_SOURCE) */

	#if !defined(_BSD_SOURCE) && !defined(FN_NO_BSD_SOURCE)
		#define _BSD_SOURCE 1
	#endif /* !defined(_BSD_SOURCE) && !defined(FN_NO_BSD_SOURCE) */

	#if !defined(_DEFAULT_SOURCE) && !defined(FN_NO_DEFAULT_SOURCE)
		#define _DEFAULT_SOURCE 1
	#endif /* !defined(_DEFAULT_SOURCE) && !defined(FN_NO_DEFAULT_SOURCE) */

	/* Configurable options for multicast. */

	/* Version string. */
	#ifndef FN_VERSION_STRING
		/* Default version string. */
		#define FN_VERSION_STRING "1.0.0"
	#endif /* FN_VERSION_STRING */

	/* Copyright string. */
	#ifndef FN_COPYRIGHT_STRING
		#define FN_COPYRIGHT_STRING "Copyright (C) 2025 ChenPi11."
	#endif /* FN_COPYRIGHT_STRING */

	/* Number of Ctrl-C signals required to force exit. */
	#ifndef FN_FORCE_EXIT_CTRL_C_COUNT
		#define FN_FORCE_EXIT_CTRL_C_COUNT 10
	#endif /* FN_FORCE_EXIT_CTRL_C_COUNT */

	/* Multicast IPv4 address. */
	#ifndef MULTICAST_ADDR
		#define MULTICAST_ADDR "224.0.1.1"
	#endif /* MULTICAST_ADDR */

	/* Multicast IPv6 address. */
	#ifndef MULTICAST_ADDR6
		#define MULTICAST_ADDR6 "ff02::fb"
	#endif /* MULTICAST_ADDR6 */

	/* Multicast port. */
	#ifndef MULTICAST_PORT
		#define MULTICAST_PORT 5354
	#endif /* MULTICAST_PORT */

	/* DNS port. */
	#ifndef DNS_PORT
		#define DNS_PORT 8053
	#endif /* DNS_PORT */

	/* DNS IPv4 address. */
	#ifndef DNS_ADDR4
		#define DNS_ADDR4 "0.0.0.0"
	#endif /* DNS_ADDR4 */

	/* DNS IPv6 address. */
	#ifndef DNS_ADDR6
		#define DNS_ADDR6 "::"
	#endif /* DNS_ADDR6 */

	/* Logging level. */
	#ifndef LOGGING_LEVEL
		#define LOGGING_LEVEL 0
	#endif /* LOGGING_LEVEL */

	/* Logging format for default handler. %1 is SUBJECT, %2 is LEVEL, %3 is
	   TIME in hex, %4 is MESSAGE. */
	#ifndef LOGGING_DEFAULT_FORMAT
		#define LOGGING_DEFAULT_FORMAT "[%1 %2 %3] %4\n"
	#endif /* LOGGING_DEFAULT_FORMAT */

	/* Multicast broadcast interval in milliseconds. */
	#ifndef MULTICAST_BOARDCAST_INTERVAL_MS
		#define MULTICAST_BOARDCAST_INTERVAL_MS 5000
	#endif /* MULTICAST_BOARDCAST_INTERVAL_MS */

	/* Global configuration file path for Unix-like systems. */
	#ifndef CONFIG_GLOBAL_CONFIG_FILE_UNIX
		#define CONFIG_GLOBAL_CONFIG_FILE_UNIX "/etc/fn/fn.conf"
	#endif /* CONFIG_GLOBAL_CONFIG_FILE_UNIX */

	/* User configuration file path for Unix-like systems. */
	#ifndef CONFIG_USER_CONFIG_FILE_UNIX
		#define CONFIG_USER_CONFIG_FILE_UNIX "~/.fn.conf"
	#endif /* CONFIG_USER_CONFIG_FILE_UNIX */

	/* Environment variable name for configuration file path. */
	#ifndef CONFIG_ENV_CONFIG_FILE
		#define CONFIG_ENV_CONFIG_FILE "FN_CONFIG_FILE"
	#endif /* CONFIG_ENV_CONFIG_FILE */

	/* Multicast protocol version. */
	#ifndef MULTICAST_VERSION
		#define MULTICAST_VERSION 1
	#endif /* MULTICAST_VERSION */

	/* Maximum length of multicast domain name. */
	#ifndef MULTICAST_MAX_DOMAIN_LEN
		#define MULTICAST_MAX_DOMAIN_LEN 256
	#endif /* MULTICAST_MAX_DOMAIN_LEN */

	/* Whether to use IPv6. If IPv6 is not supported, it will be disabled. */
	#ifndef USE_IPv6
		#define USE_IPv6 1
	#endif /* USE_IPv6 */

	#ifndef HAS_WINDOWS
		#if defined(_WIN32) || defined(_WIN64)
			#define HAS_WINDOWS 1
		#else
			#define HAS_WINDOWS 0
		#endif /* defined(_WIN32) || defined(_WIN64) */
	#endif     /* HAS_WINDOWS */

	#ifndef HAS_UNIX
		#if defined(__unix) || defined(__unix__) || defined(unix)
			#define HAS_UNIX 1
		#else
			#define HAS_UNIX 0
		#endif /* defined(__unix) || defined(__unix__) || defined(unix) */
	#endif     /* HAS_UNIX */

	#ifndef HAS_SAFE_C_LIB
		#if defined(__STDC_LIB_EXT1__) || (_MSC_VER >= 1600)
			#define HAS_SAFE_C_LIB 1
		#else
			#define HAS_SAFE_C_LIB 0
		#endif /* defined(__STDC_LIB_EXT1__) || (_MSC_VER >= 1600) */
	#endif     /* HAS_SAFE_C_LIB */

	#if (defined(FNAPI) || defined(_FNAPI_EXPORT)) &&                          \
		!(defined(FNAPI) && defined(_FNAPI_EXPORT))
		#error Both FNAPI and _FNAPI_EXPORT must be defined or neither.
	#endif

	#undef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN 1
	#undef NOMINMAX
	#define NOMINMAX 1
	#undef VC_EXTRALEAN
	#define VC_EXTRALEAN 1

	#ifndef FN_UNICODE
		#define FN_UNICODE 1
	#endif /* FN_UNICODE */

	#if FN_UNICODE
		#define UNICODE 1
		#define _UNICODE 1
	#else
		#undef UNICODE
		#undef _UNICODE
	#endif /* FN_NO_UNICODE */

	#if !defined(FNAPI) && !defined(_FNAPI_EXPORT)
		#ifdef _MSC_VER
			#define FNAPI __declspec(dllimport)
			#define _FNAPI_EXPORT __declspec(dllexport)
		#elif defined(__GNUC__) && __GNUC__ >= 4
			#define FNAPI __attribute__((visibility("default")))
			#define _FNAPI_EXPORT __attribute__((visibility("default")))
		#elif defined(__clang__) && __clang__ >= 4
			#define FNAPI __attribute__((visibility("default")))
			#define _FNAPI_EXPORT __attribute__((visibility("default")))
		#else
			#define FNAPI
			#define _FNAPI_EXPORT
		#endif /* _MSC_VER */

		/* Fix FNAPI visibility for building. */
		#if defined(_BUILDING_FN) || defined(MIXED_FILE)
			#undef FNAPI
			#define FNAPI _FNAPI_EXPORT
		#endif /* defined(_BUILDING_FN) || defined(MIXED_FILE) */

	#endif /* !defined(FNAPI) && !defined(_FNAPI_EXPORT) */

/* @NO MRG */
#endif /* FN_CONFIG_H_INC */
/* @END NO MRG */
