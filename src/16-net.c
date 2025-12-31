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
#include "16-net.h"
#include "00-config.h"
#include "01-stdafx.h"
#include "04-minmax.h"
#include "07-string.h"
#include "13-fmt.h"
#include "15-logging.h"

/* @END NO MRG */

/* Result<NET_Socket>. */
_FNAPI_EXPORT RESULT_NET_Socket
RESULT_error_NET_Socket(int code, enum EXC_ErrorCategory category)
{
	RESULT_NET_Socket result;
	result.has_value = UTIL_FALSE;
	result.result.error.code = code;
	result.result.error.category = category;
	return result;
}
_FNAPI_EXPORT RESULT_NET_Socket
RESULT_error_NET_Socket_struct(struct EXC_ErrorCode error)
{
	RESULT_NET_Socket result;
	result.has_value = UTIL_FALSE;
	result.result.error = error;
	return result;
}
_FNAPI_EXPORT RESULT_NET_Socket RESULT_ok_NET_Socket(NET_Socket value)
{
	RESULT_NET_Socket result;
	result.has_value = UTIL_TRUE;
	result.result.value = value;
	return result;
}

_FNAPI_EXPORT void NET_repr_inet_addr(const NET_InetAddr *addr,
									  UTIL_StringBuffer out_buf,
									  size_t out_buf_len)
{
	char *tmp;
	size_t tmp_len;
	RESULT_void fmt_res;

	tmp = NULL;
	memset(out_buf, 0, out_buf_len);

	out_buf[0] = 'R';
	out_buf[1] = 'E';
	out_buf[2] = 'P';
	out_buf[3] = 'R';
	out_buf[4] = ' ';
	out_buf[5] = 'E';
	out_buf[6] = 'R';
	out_buf[7] = 'R';
	out_buf[8] = 'O';
	out_buf[9] = 'R';
	out_buf[10] = 'R';

	if (addr->version == NET_INET_VERSION_4)
	{
		fmt_res = FMT_format(&tmp, "%1.%2.%3.%4",
							 FMT_uint16_autoalign(addr->addr.inet4[0]),
							 FMT_uint16_autoalign(addr->addr.inet4[1]),
							 FMT_uint16_autoalign(addr->addr.inet4[2]),
							 FMT_uint16_autoalign(addr->addr.inet4[3]));
		if (!fmt_res.has_value)
		{
			LOGGING_warn("NET", "NET_repr_inet_addr: FMT_format failed: %s",
						 FMT_str(EXC_strerror(fmt_res.error)));
			goto EXIT;
		}
		tmp_len = strlen(tmp);
		if (tmp_len + 1 > out_buf_len)
		{
			LOGGING_warn("NET", "NET_repr_inet_addr: Output buffer too small.");
			goto EXIT;
		}
		memcpy(out_buf, tmp, tmp_len + 1);
		goto EXIT;
	}
#if USE_IPv6
	else if (addr->version == NET_INET_VERSION_6)
	{
		fmt_res = FMT_format(
			&tmp, "%x:%x:%x:%x:%x:%x:%x:%x", FMT_hex16_4(addr->addr.inet6[0]),
			FMT_hex16_4(addr->addr.inet6[1]), FMT_hex16_4(addr->addr.inet6[2]),
			FMT_hex16_4(addr->addr.inet6[3]), FMT_hex16_4(addr->addr.inet6[4]),
			FMT_hex16_4(addr->addr.inet6[5]), FMT_hex16_4(addr->addr.inet6[6]),
			FMT_hex16_4(addr->addr.inet6[7]));
		if (!fmt_res.has_value)
		{
			LOGGING_warn("NET", "NET_repr_inet_addr: FMT_format failed: %s",
						 FMT_str(EXC_strerror(fmt_res.error)));
			goto EXIT;
		}
		tmp_len = strlen(tmp);
		if (tmp_len + 1 > out_buf_len)
		{
			LOGGING_warn("NET", "NET_repr_inet_addr: Output buffer too small.");
			goto EXIT;
		}
		memcpy(out_buf, tmp, tmp_len + 1);
		goto EXIT;
	}
#endif /* USE_IPv6 */
	else
	{
		LOGGING_warn("NET",
					 "NET_repr_inet_addr: Unsupported address family: %d",
					 FMT_hex16_4(addr->version));
		goto EXIT;
	}
EXIT:
	free(tmp);
}

/* Don't use sockaddr_storage because it's not available on WinNT 4.0. */
typedef UTIL_UInt8 NET_SockAddrStorage[UTIL_max(sizeof(struct sockaddr_in),
												sizeof(struct sockaddr_in6))];

/* Load port from system ADDR to FN's OUT_ADDR. */
static void NET__load_addr(struct sockaddr *addr, struct NET_InetAddr *out_addr)
{
	if (addr->sa_family == AF_INET)
	{
		struct sockaddr_in *addr_in;
		UTIL_UInt32 ip_host;

		addr_in = (struct sockaddr_in *)addr;
		out_addr->version = NET_INET_VERSION_4;
		ip_host = ntohl(addr_in->sin_addr.s_addr);

		out_addr->addr.inet4[0] = (UTIL_UInt8)((ip_host >> 24) & 0xFF);
		out_addr->addr.inet4[1] = (UTIL_UInt8)((ip_host >> 16) & 0xFF);
		out_addr->addr.inet4[2] = (UTIL_UInt8)((ip_host >> 8) & 0xFF);
		out_addr->addr.inet4[3] = (UTIL_UInt8)(ip_host & 0xFF);
	}
#if USE_IPv6
	else if (addr->sa_family == AF_INET6)
	{
		struct sockaddr_in6 *addr_in6;
		size_t i;
		UTIL_UInt16 segment;

		addr_in6 = (struct sockaddr_in6 *)addr;
		out_addr->version = NET_INET_VERSION_6;
		for (i = 0; i < 8; i++)
		{
			memcpy(&segment, &addr_in6->sin6_addr.s6_addr[i * 2],
				   sizeof(UTIL_UInt16));
			segment = ntohs(segment);
			out_addr->addr.inet6[i] = segment;
		}
	}
#endif /* USE_IPv6 */
	else
		LOGGING_error("NET", "NET_load_addr: Unsupported address family: %d",
					  FMT_hex16_4(addr->sa_family));
}

/* Load the port of a ADDR into OUT_PORT. */
static void NET__load_port(struct sockaddr *addr, UTIL_UInt16 *out_port)
{
	if (addr == NULL || out_port == NULL) return;

	if (addr->sa_family == AF_INET)
	{
		struct sockaddr_in *addr_in;

		addr_in = (struct sockaddr_in *)addr;
		*out_port = ntohs(addr_in->sin_port);
	}
#if USE_IPv6
	else if (addr->sa_family == AF_INET6)
	{
		struct sockaddr_in6 *addr_in6;

		addr_in6 = (struct sockaddr_in6 *)addr;
		*out_port = ntohs(addr_in6->sin6_port);
	}
#endif
	else
		LOGGING_error("NET", "NET_load_port: Unsupported address family: %d",
					  FMT_hex16_4(addr->sa_family));
}

_FNAPI_EXPORT UTIL_Bool NET_need_try_again(struct EXC_ErrorCode err)
{
	return (err.category == EXC_CATEGORY_FN) && (err.code == EFN_TRY_AGAIN);
}

#if HAS_WINDOWS
	#define NET__need_retry(err) (err == WSAEWOULDBLOCK)

static UTIL_Bool g_NET__wsa_initialized = UTIL_FALSE;

_FNAPI_EXPORT RESULT_void NET_initialize(void)
{
	WSADATA wsa_data;
	int result;

	if (g_NET__wsa_initialized) return RESULT_ok_void();

	result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != 0) return RESULT_error_void(result, EXC_CATEGORY_NET);

	g_NET__wsa_initialized = UTIL_TRUE;

	return RESULT_ok_void();
}

_FNAPI_EXPORT RESULT_void NET_deinitialize(void)
{
	int result;

	if (!g_NET__wsa_initialized) return RESULT_ok_void();

	result = WSACleanup();
	if (result != 0) return RESULT_error_void(result, EXC_CATEGORY_NET);

	g_NET__wsa_initialized = UTIL_FALSE;

	return RESULT_ok_void();
}

_FNAPI_EXPORT void NET_closesocket(NET_Socket sock)
{
	EXC_clear_last_error(EXC_CATEGORY_NET);
	if (closesocket(sock) == SOCKET_ERROR)
		LOGGING_warn(
			"NET", "Failed to close socket: %s",
			FMT_str(EXC_strerror(EXC_get_last_error(EXC_CATEGORY_NET))));
}

_FNAPI_EXPORT RESULT_NET_Socket NET_async_socket(NET_InetVersion inet_version)
{
	NET_Socket sock;
	u_long nonblock;

	EXC_clear_last_error(EXC_CATEGORY_NET);
	sock = (NET_Socket)WSASocket((int)inet_version, SOCK_DGRAM, IPPROTO_UDP,
								 NULL, 0, WSA_FLAG_OVERLAPPED);
	if (sock == INVALID_SOCKET)
		return RESULT_ERRNO(NET_Socket, EXC_CATEGORY_NET);

	/* Set the socket to non-blocking mode. */
	nonblock = 1;

	EXC_clear_last_error(EXC_CATEGORY_NET);
	if (ioctlsocket(sock, FIONBIO, &nonblock) == SOCKET_ERROR)
	{
		RESULT_NET_Socket err;
		err = RESULT_ERRNO(NET_Socket, EXC_CATEGORY_NET);
		NET_closesocket(sock);
		return err;
	}

	return RESULT_ok_NET_Socket(sock);
}

_FNAPI_EXPORT RESULT_UTIL_SignedSize NET_recvfrom(NET_Socket sock,
												  UTIL_StringBuffer buf,
												  size_t len, int flags,
												  NET_InetAddr *src_addr,
												  UTIL_UInt16 *src_port)
{
	UTIL_SignedSize received;
	NET_SockAddrStorage addr;
	int addr_len;

	if (src_addr == NULL || src_port == NULL || buf == NULL || len == 0)
		return RESULT_error_UTIL_SignedSize(EINVAL, EXC_CATEGORY_LIBC);

	addr_len = (int)sizeof(addr);
	memset(&addr, 0, sizeof(addr));

	EXC_clear_last_error(EXC_CATEGORY_NET);
	received = (UTIL_SignedSize)recvfrom(sock, buf, len, flags,
										 (struct sockaddr *)&addr, &addr_len);
	if (received < 0)
	{
		EXC_ErrorCode err;
		err = EXC_get_last_error(EXC_CATEGORY_NET);
		if (NET__need_retry(err.code)) /* Convert EAGAIN. */
		{
			return RESULT_error_UTIL_SignedSize(EFN_TRY_AGAIN, EXC_CATEGORY_FN);
		}
		return RESULT_error_UTIL_SignedSize_struct(err);
	}

	NET__load_addr((struct sockaddr *)&addr, src_addr);
	NET__load_port((struct sockaddr *)&addr, src_port);

	return RESULT_ok_UTIL_SignedSize(received);
}

_FNAPI_EXPORT RESULT_void NET_sendto(NET_Socket sock, UTIL_ConstString buf,
									 size_t len, int flags,
									 const NET_InetAddr *dest_addr,
									 UTIL_UInt16 dest_port)
{
	NET_SockAddrStorage addr;
	int addr_len;

	if (dest_addr == NULL || buf == NULL || len == 0)
		return RESULT_error_void(EINVAL, EXC_CATEGORY_LIBC);

	memset(&addr, 0, sizeof(addr));

	/* Fill in address structure. */
	if (dest_addr->version == NET_INET_VERSION_4)
	{
		struct sockaddr_in *addr_in;

		addr_in = (struct sockaddr_in *)&addr;
		addr_in->sin_family = AF_INET;
		addr_in->sin_port = htons(dest_port);
		addr_in->sin_addr.s_addr =
			htonl((UTIL_UInt32)dest_addr->addr.inet4[0] << 24 |
				  (UTIL_UInt32)dest_addr->addr.inet4[1] << 16 |
				  (UTIL_UInt32)dest_addr->addr.inet4[2] << 8 |
				  (UTIL_UInt32)dest_addr->addr.inet4[3]);
		addr_len = (int)sizeof(struct sockaddr_in);
	}
	else if (dest_addr->version == NET_INET_VERSION_6)
	{
		struct sockaddr_in6 *addr_in6;
		size_t i;

		addr_in6 = (struct sockaddr_in6 *)&addr;
		addr_in6->sin6_family = AF_INET6;
		addr_in6->sin6_port = htons(dest_port);
		for (i = 0; i < 8; i++)
		{
			UTIL_UInt16 segment;

			segment = htons(dest_addr->addr.inet6[i]);
			memcpy(&addr_in6->sin6_addr.s6_addr[i * 2], &segment,
				   sizeof(UTIL_UInt16));
		}
		addr_len = (int)sizeof(struct sockaddr_in6);
	}
	else
		return RESULT_error_void(EINVAL, EXC_CATEGORY_LIBC); /* Never happen. */

	if (sendto(sock, buf, len, flags, (struct sockaddr *)&addr, addr_len) < 0)
	{
		EXC_ErrorCode err;
		err = EXC_get_last_error(EXC_CATEGORY_NET);
		if (NET__need_retry(err.code)) /* Convert EAGAIN. */
		{
			return RESULT_error_void(EFN_TRY_AGAIN, EXC_CATEGORY_FN);
		}
		return RESULT_error_void_struct(err);
	}

	return RESULT_ok_void();
}
#else

#endif /* HAS_WINDOWS */
