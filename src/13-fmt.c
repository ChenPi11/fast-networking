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
#include "13-fmt.h"
#include "00-config.h"
#include "11-utils.h"
/* @END NO MRG */

/* Maximum number of arguments supported. MUST <= 9 */
#define FMT_MAX_ARGS 9

static size_t FMT__count_token_len(FMT_Token token)
{
	switch (token.type)
	{
	case FMT_TOKEN_STRING:
		return strlen(token.value.str);
	case FMT_TOKEN_HEX16_4ALIGN:
		return 6; /* 0x0000-0xFFFF */
	case FMT_TOKEN_UINT16_AUTOALIGN:
		return (token.value.uint16 < 10      ? 1
				: token.value.uint16 < 100   ? 2
				: token.value.uint16 < 1000  ? 3
				: token.value.uint16 < 10000 ? 4
											 : 5);
	case FMT_TOKEN_UINT64_AUTOALIGN: {
		UTIL_UInt64 v; /* Support uint64 on old platform. */
		size_t len;

		len = 1;
		v = token.value.uint64;
		while (v >= 10)
		{
			v /= 10;
			len++;
		}
		return len;
	}
	default:
		return 0;
	}
}

/* (Internal) Count required buffer size (including terminating NUL) using
   pre-cached args. We do NOT consume a va_list here; instead, caller provides
   ARGVALS[] and provided count (number of valid entries). Placeholders
   referencing indices beyond provided args are emitted literally (as "%n").
   Returns -1 on error. */
static UTIL_SignedSize FMT__count_with_args(UTIL_ConstString fmt,
											FMT_Token argvals[], int provided)
{
	UTIL_SignedSize ret;
	UTIL_ConstString p;
	size_t out_len;

	if (!fmt)
	{
		ret = -1;
		return ret;
	}

	out_len = 0;
	p = fmt;

	/* Compute output length. */
	while (*p)
	{
		if (*p == '%')
		{
			UTIL_ConstString q;

			if (p[1] == '%')
			{
				out_len += 1;
				p += 2;
				continue;
			}
			/* Attempt to parse index. */
			q = p + 1;
			if (isdigit((unsigned char)*q))
			{
				int idx;
				UTIL_ConstString r;

				idx = 0;
				r = q;
				while (*r && isdigit((unsigned char)*r))
				{
					idx = idx * 10 + (*r - '0');
					r++;
				}
				if (idx >= 1 && idx <= FMT_MAX_ARGS)
				{
					if (idx <= provided)
					{
						/* Substitute argument. */
						out_len += FMT__count_token_len(argvals[idx - 1]);
					}
					else
					{
						/* Do not substitute: emit literal "%<digits>". */
						out_len += (r - p);
					}
				}
				else
				{
					/* Index out of supported range: emit literal "%<digits>".
					 */
					out_len += (r - p);
				}
				p = r;
				continue;
			}
			else
			{
				/* '%' followed by non-digit: treat '%' literal. */
				out_len += 1;
				p++;
				continue;
			}
		}
		else
		{
			out_len += 1;
			p++;
		}
	}

	/* Include terminating NUL. */
	if (out_len > (size_t)UTIL_SSIZE_MAX - 1)
	{
		ret = -1;
		return ret;
	}

	ret = (UTIL_SignedSize)(out_len + 1);
	return ret;
}

_FNAPI_EXPORT RESULT_void FMT_vformat(UTIL_StringBuffer *buffer,
									  UTIL_ConstString fmt, va_list ap)
{
	UTIL_StringBuffer buf;
	UTIL_SignedSize need;
	size_t bufsize;
	int max_idx;
	FMT_Token argvals[FMT_MAX_ARGS];
	int provided;
	UTIL_ConstString p;
	UTIL_StringBuffer w;
	size_t remaining;
	int i;

	if (fmt == NULL || buffer == NULL)
		return RESULT_error_void(EINVAL, EXC_CATEGORY_LIBC);

	/* Determine highest referenced index (1..9). */
	p = fmt;
	max_idx = 0;
	while (*p)
	{
		if (*p == '%')
		{
			UTIL_ConstString q;
			if (p[1] == '%')
			{
				p += 2;
				continue;
			}
			q = p + 1;
			if (isdigit((unsigned char)*q))
			{
				int idx;
				idx = 0;
				while (*q && isdigit((unsigned char)*q))
				{
					idx = idx * 10 + (*q - '0');
					q++;
				}
				if (idx >= 1 && idx <= FMT_MAX_ARGS)
				{
					if (idx > max_idx) max_idx = idx;
				}
				p = q;
				continue;
			}
			else
			{
				p++;
				continue;
			}
		}
		else
		{
			p++;
		}
	}

	/* Initialize argvals. */
	for (i = 0; i < FMT_MAX_ARGS; i++)
		argvals[i] = FMT_END;
	provided = 0;

	if (max_idx > 0)
	{
		/* Read arguments up to max_idx, stopping at first FMT_END sentinel. */
		for (i = 0; i < max_idx; i++)
		{
			FMT_Token t;
			t = va_arg(ap, FMT_Token);
			if (t.type == FMT_TOKEN_END)
			{
				/* Treat END as end-of-args sentinel. */
				break;
			}
			argvals[i] = t;
			provided++;
		}
	}

	/* Determine required size using cached args. */
	need = FMT__count_with_args(fmt, argvals, provided);
	if (need <= 0) return RESULT_error_void(EINVAL, EXC_CATEGORY_LIBC);

	bufsize = (size_t)need;
	buf = (UTIL_StringBuffer)malloc(bufsize);
	if (buf == NULL) return RESULT_error_void(ENOMEM, EXC_CATEGORY_LIBC);

	/* Build the string into buf. */
	p = fmt;
	w = buf;
	remaining = bufsize; /* Includes room for terminating NUL. */

	while (*p)
	{
		if (*p == '%')
		{
			UTIL_ConstString q;

			if (p[1] == '%')
			{
				if (remaining < 2) goto ERR_NOMEM;
				*w++ = '%';
				remaining -= 1;
				p += 2;
				continue;
			}
			q = p + 1;
			if (isdigit((unsigned char)*q))
			{
				int idx;
				UTIL_ConstString r;

				idx = 0;
				r = q;
				while (*r && isdigit((unsigned char)*r))
				{
					idx = idx * 10 + (*r - '0');
					r++;
				}
				if (idx >= 1 && idx <= FMT_MAX_ARGS)
				{
					if (idx <= provided)
					{
						FMT_Token token;
						size_t tlen;

						token = argvals[idx - 1];
						switch (token.type)
						{
						case FMT_TOKEN_STRING: {
							UTIL_ConstString s;

							s = token.value.str;
							tlen = strlen(s);
							if (tlen > remaining) goto ERR_NOMEM;
							memcpy(w, s, tlen);
							w += tlen;
							remaining -= tlen;
							break;
						}
						case FMT_TOKEN_HEX16_4ALIGN: {
							tlen = UTIL_uint16_to_base16(token.value.uint16, w);
							w += tlen;
							remaining -= tlen;
							break;
						}
						case FMT_TOKEN_UINT16_AUTOALIGN: {
							tlen = UTIL_uint16_to_base10(token.value.uint16, w);
							w += tlen;
							remaining -= tlen;
							break;
						}
						case FMT_TOKEN_UINT64_AUTOALIGN: {
							tlen = UTIL_uint64_to_base10(token.value.uint64, w);
							w += tlen;
							remaining -= tlen;
							break;
						}
						case FMT_TOKEN_END:
						default:
							/* Unknown token type: emit nothing. */
							break;
						}
					}
					else
					{
						size_t litlen;

						litlen = (size_t)(r - p);
						if (litlen >= remaining) goto ERR_NOMEM;
						memcpy(w, p, litlen);
						w += litlen;
						remaining -= litlen;
					}
				}
				else
				{
					size_t litlen;

					litlen = (size_t)(r - p);
					if (litlen >= remaining) goto ERR_NOMEM;
					memcpy(w, p, litlen);
					w += litlen;
					remaining -= litlen;
				}
				p = r;
				continue;
			}
			else
			{
				if (remaining < 2) goto ERR_NOMEM;
				*w++ = '%';
				remaining -= 1;
				p++;
				continue;
			}
		}
		else
		{
			if (remaining < 2) goto ERR_NOMEM;
			*w++ = *p++;
			remaining -= 1;
		}
	}

	if (remaining < 1) goto ERR_NOMEM;
	*w = '\0';
	*buffer = buf;
	return RESULT_ok_void();

ERR_NOMEM:
	free(buf);
	return RESULT_error_void(ENOMEM, EXC_CATEGORY_LIBC);
}

_FNAPI_EXPORT RESULT_void FMT_format(UTIL_StringBuffer *buffer,
									 UTIL_ConstString fmt, ...)
{
	va_list ap;
	RESULT_void ret;

	va_start(ap, fmt);
	ret = FMT_vformat(buffer, fmt, ap);
	va_end(ap);

	return ret;
}

_FNAPI_EXPORT FMT_Token FMT_str(UTIL_ConstString str)
{
	FMT_Token token;
	token.type = FMT_TOKEN_STRING;
	token.value.str = str;
	return token;
}

_FNAPI_EXPORT FMT_Token FMT_hex16_4(UTIL_UInt16 value)
{
	FMT_Token token;
	token.type = FMT_TOKEN_HEX16_4ALIGN;
	token.value.uint16 = value;
	return token;
}

_FNAPI_EXPORT FMT_Token FMT_uint16_autoalign(UTIL_UInt16 value)
{
	FMT_Token token;
	token.type = FMT_TOKEN_UINT16_AUTOALIGN;
	token.value.uint16 = value;
	return token;
}

_FNAPI_EXPORT FMT_Token FMT_uint64_autoalign(UTIL_UInt64 value)
{
	FMT_Token token;
	token.type = FMT_TOKEN_UINT64_AUTOALIGN;
	token.value.uint64 = value;
	return token;
}
