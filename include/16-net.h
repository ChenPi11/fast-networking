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
#include "03-bool.h"
#include "08-exc.h"
#include "09-generic-types.h"

#ifndef FN_NET_H_INC
	#define FN_NET_H_INC
/* @END NO MRG */

/* Network socket type. */
typedef int NET_Socket;

/* Result<NET_Socket>. */
typedef struct RESULT_NET_Socket
{
	UTIL_Bool has_value;
	union {
		struct EXC_ErrorCode error;
		NET_Socket value;
	} result;
} RESULT_NET_Socket;
FNAPI extern RESULT_NET_Socket RESULT_error_NET_Socket(
	int code, enum EXC_ErrorCategory category);
FNAPI extern RESULT_NET_Socket RESULT_error_NET_Socket_struct(
	struct EXC_ErrorCode error);
FNAPI extern RESULT_NET_Socket RESULT_ok_NET_Socket(NET_Socket value);

	/* Invalid socket constant. */
	#define NET_INVALID_SOCKET (NET_Socket)(-1)

/* Internet protocol version. */
typedef enum NET_InetVersion
{
	/* IPv4 */
	NET_INET_VERSION_4 = AF_INET,
	#if USE_IPv6
	/* IPv6 */
	NET_INET_VERSION_6 = AF_INET6
	#endif /* USE_IPv6 */
} NET_InetVersion;

/* Internet address structures in local byte order. */
typedef UTIL_UInt8 NET_Inet4Addr[4];

	#if USE_IPv6
/* IPv6 address as an array of 8 16-bit segments in local byte order. */
typedef UTIL_UInt16 NET_Inet6Addr[8];
	#endif /* USE_IPv6 */

/* Generic internet address structure. */
typedef struct NET_InetAddr
{
	/* Internet protocol version. */
	NET_InetVersion version;
	union {
		/* IPv4 address. */
		NET_Inet4Addr inet4;
	#if USE_IPv6
		/* IPv6 address. */
		NET_Inet6Addr inet6;
	#endif /* USE_IPv6 */
	} addr;
} NET_InetAddr;

/* Initialize the networking module. */
FNAPI extern RESULT_void NET_initialize(void);

/* Deinitialize the networking subsystem. */
FNAPI extern RESULT_void NET_deinitialize(void);

/* Close a network socket. */
FNAPI extern void NET_closesocket(NET_Socket sock);

/* Create an asynchronous socket for the given internet version INET_VERSION. */
FNAPI extern RESULT_NET_Socket NET_async_socket(NET_InetVersion inet_version);

/* Convert the internet address ADDR to a string representation in OUT_BUF. */
FNAPI extern void NET_repr_inet_addr(const NET_InetAddr *addr,
									 UTIL_StringBuffer out_buf,
									 size_t out_buf_len);

	/* Maximum length of a string representation of an internet address. */
	#define NET_ADDR_REPR_MAX_LENGTH 46

/* Receive data from a socket SK, storing the source address and port in
   SRC_ADDR and SRC_PORT. */
FNAPI extern RESULT_UTIL_SignedSize NET_recvfrom(NET_Socket sock, UTIL_StringBuffer buf,
												 size_t len, int flags,
												 NET_InetAddr *src_addr,
												 UTIL_UInt16 *src_port);

/* Send data to the specified destination address and port. */
FNAPI extern RESULT_void NET_sendto(NET_Socket sock, UTIL_ConstString buf,
									size_t len, int flags,
									const NET_InetAddr *dest_addr,
									UTIL_UInt16 dest_port);

/* Check if the error code ERR indicates a temporary failure (EAGAIN). */
FNAPI extern UTIL_Bool NET_need_try_again(EXC_ErrorCode err);

/* @NO MRG */
#endif
/* @END NO MRG */
