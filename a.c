/*
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#if defined(__linux__) /* Only on Linux. */
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif
#undef _BSD_SOURCE
#define _BSD_SOURCE 1
#undef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1

/* Configurable options for multicast. */
#define FN_VERSION_STRING "1.0.0"
#define FN_COPYRIGHT_STRING "Copyright (C) 2025 ChenPi11."
#define FN_FORCEEXIT_COUNT 10
#define MULTICAST_ADDR "224.0.1.1"
#define MULTICAST_ADDR6 "ff02::fb"
#define MULTICAST_PORT 5354
#define DNS_PORT 8053
#define DNS_ADDR4 "0.0.0.0"
#define DNS_ADDR6 "::"
#define LOGGING_LEVEL 0
#define MULTICAST_BOARDCAST_INTERVAL_MS 5000

#define CONFIG_GLOBAL_CONFIG_FILE_UNIX "/etc/fn/fn.conf"
#define CONFIG_USER_CONFIG_FILE_UNIX "~/.fn.conf"
#define CONFIG_ENV_CONFIG_FILE "FN_CONFIG_FILE"

/* Multicast protocol version. */
#define MULTICAST_VERSION 1
/* Maximum length of multicast domain name. */
#define MULTICAST_MAX_DOMAIN_LEN 256

#if defined(_WIN32) || defined(_WIN64)
#define HAS_WINDOWS 1
#endif

#if defined(__unix) || defined(__unix__) || defined(unix)
#define HAS_UNIX 1
#endif

#if defined(__STDC_LIB_EXT1__) || (_MSC_VER >= 1600)
#define HAS_SAFE_C_LIB 1
#endif

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if HAS_WINDOWS
#include <Windows.h>
#include <winsock.h>
#else
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#ifndef va_copy
/* Prefer compiler provided forms first. */
#if defined(__va_copy)
#define va_copy(dest, src) __va_copy((dest), (src))
#elif defined(__GNUC__) && defined(__GNUC_MINOR__) /* GCC/Clang provide builtin. */
#define va_copy(dest, src) __builtin_va_copy((dest), (src))
#elif defined(_MSC_VER)
/* MSVC's va_list is assignable on known targets. */
#define va_copy(dest, src) ((dest) = (src))
#else
/* Last resort: memcpy the underlying object. Works on many real-world impls,
   but strictly speaking C89 doesn't guarantee it's portable for all va_list
   representations (e.g. if va_list is an array type). */

#if defined(FN_ALLOW_UNSAFE_VA_COPY)
#define va_copy(dest, src) memcpy(&(dest), &(src), sizeof(va_list))
#else
#error                                                                                                                 \
    "No va_copy implementation available on this platform. You should update to C99 or later, or define FN_ALLOW_UNSAFE_VA_COPY to force an unsafe memcpy-based implementation."
#endif
#endif
#endif

#if 0
#pragma region util types
#endif

/* Boolean type. */
typedef unsigned int UTIL_Bool;

/* Boolean values. */
#define UTIL_TRUE 1
#define UTIL_FALSE 0

/* Maximum of A and B. */
#define UTIL_max(a, b) ((a) > (b) ? (a) : (b))

/* Minimum of A and B. */
#define UTIL_min(a, b) ((a) < (b) ? (a) : (b))

#if defined(__cplusplus)
#define UTIL_register
#else
#define UTIL_register register
#endif

/* Maximum signed value for type of size N (e.g. UTIL_MAX_SIGNED_VALUE(4) is
   2147483647 for int32_t). */
#define UTIL_MAX_SIGNED_VALUE(x) ((int64_t)(((uint64_t)1 << ((x) * 8 - 1)) - 1))

#if HAS_WINDOWS
/* Signed size type. */
typedef ptrdiff_t UTIL_SignedSize;
#else
/* Signed size type. */
typedef ssize_t UTIL_SignedSize;
#endif

/* Constant string type. */
typedef const char *UTIL_ConstString;

/* Mutable string buffer type. */
typedef char *UTIL_StringBuffer;

/* Maximum value for UTIL_SignedSize. */
#define UTIL_SSIZE_MAX UTIL_MAX_SIGNED_VALUE(sizeof(UTIL_SignedSize))

#if 0
#pragma endregion

#pragma region exception
#endif

/* Exception error categories. */
enum EXC_ErrrorCategory
{
    /* Libc error category. */
    EXC_CATEGORY_LIBC = 0,
    /* System error category. */
    EXC_CATEGORY_SYS,
    /* Network error category. */
    EXC_CATEGORY_NET,
    /* Error category for this program. */
    EXC_CATEGORY_FN
};

#define EFN_MAX_CAPACITY_EXCEEDED 1       /* Capacity exceeded. */
#define EFN_TRY_AGAIN 2                   /* Try again. */
#define EFN_COMPRESSION_PTR_UNSUPPORTED 3 /* DNS compression pointer unsupported. */
#define EFN_INVALID_HOSTNAME 4            /* Invalid hostname. */
#define EFN_NO_MULTICAST_JOINED 5         /* No interfaces joined the multicast group */
#define EFN_JOIN_REFUSED 6                /* Join request refused. */

/* Exception error code. */
struct EXC_ErrorCode
{
    /* Error code. */
    int code;
    /* Error category. */
    enum EXC_ErrrorCategory category;
};

#if HAS_WINDOWS
/* Get last exception code with CATEGORY. */
struct EXC_ErrorCode EXC_get_last_error(enum EXC_ErrrorCategory category)
{
    struct EXC_ErrorCode error;

    error.code = 0;
    error.category = category;

    switch (category)
    {
    case EXC_CATEGORY_LIBC:
        error.code = errno;
        break;
    case EXC_CATEGORY_SYS:
        error.code = (int)GetLastError();
        break;
    case EXC_CATEGORY_NET:
        error.code = (int)WSAGetLastError();
        break;
    default:
        break;
    }

    return error;
}
#else
/* Get last exception code with CATEGORY. */
struct EXC_ErrorCode EXC_get_last_error(enum EXC_ErrrorCategory category)
{
    struct EXC_ErrorCode error;

    error.code = errno;
    error.category = category;

    return error;
}
#endif

#if HAS_WINDOWS
/* Clear last exception code with CATEGORY. */
void EXC_clear_last_error(enum EXC_ErrrorCategory category)
{
    switch (category)
    {
    case EXC_CATEGORY_LIBC:
        errno = 0;
        break;
    case EXC_CATEGORY_SYS:
        SetLastError(0);
        break;
    case EXC_CATEGORY_NET:
        WSASetLastError(0);
        break;
    default:
        break;
    }
}
#else
/* Clear last exception code with CATEGORY. */
#define EXC_clear_last_error(category)                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        errno = 0;                                                                                                     \
    } while (0)
#endif

/* Get error message for ERROR. */
UTIL_ConstString EXC_strerror(struct EXC_ErrorCode error)
{
    switch (error.category)
    {
    case EXC_CATEGORY_LIBC:
        return strerror(error.code);
    case EXC_CATEGORY_SYS:
        return strerror(error.code);
    case EXC_CATEGORY_NET:
        return strerror(error.code);
    case EXC_CATEGORY_FN:
        switch (error.code)
        {
        case EFN_MAX_CAPACITY_EXCEEDED:
            return "Max capacity exceeded";
        case EFN_TRY_AGAIN:
            return "Try again";
        case EFN_COMPRESSION_PTR_UNSUPPORTED:
            return "DNS compression pointer unsupported";
        case EFN_INVALID_HOSTNAME:
            return "Invalid hostname";
        case EFN_NO_MULTICAST_JOINED:
            return "No interfaces joined the multicast group";
        case EFN_JOIN_REFUSED:
            return "Join request refused";
        default:
            return "Unknown error";
        }
    default:
        return "Unknown error category";
    }
}

#if 0
#pragma endregion

#pragma region result
#endif

/* A generic result type definition utility. */
#define RESULT_GENERIC(ResultType)                                                                                     \
    typedef struct RESULT_Tag_##ResultType                                                                             \
    {                                                                                                                  \
        UTIL_Bool has_value;                                                                                           \
        union {                                                                                                        \
            struct EXC_ErrorCode error;                                                                                \
            ResultType value;                                                                                          \
        } result;                                                                                                      \
    } RESULT_##ResultType;                                                                                             \
    RESULT_##ResultType RESULT_error_##ResultType(int code, enum EXC_ErrrorCategory category)                          \
    {                                                                                                                  \
        RESULT_##ResultType res;                                                                                       \
        res.has_value = UTIL_FALSE;                                                                                    \
        res.result.error.category = category;                                                                          \
        res.result.error.code = code;                                                                                  \
        return res;                                                                                                    \
    }                                                                                                                  \
    RESULT_##ResultType RESULT_error_##ResultType##_struct(struct EXC_ErrorCode error)                                 \
    {                                                                                                                  \
        RESULT_##ResultType res;                                                                                       \
        res.has_value = UTIL_FALSE;                                                                                    \
        res.result.error = error;                                                                                      \
        return res;                                                                                                    \
    }                                                                                                                  \
    RESULT_##ResultType RESULT_ok_##ResultType(ResultType value)                                                       \
    {                                                                                                                  \
        RESULT_##ResultType res;                                                                                       \
        res.has_value = UTIL_TRUE;                                                                                     \
        res.result.value = value;                                                                                      \
        return res;                                                                                                    \
    }

/* Result type for void. */
typedef struct RESULT_Tag_void
{
    UTIL_Bool has_value;
    struct EXC_ErrorCode error;
} RESULT_void;

/* Create an error result of type void with code and category. */
RESULT_void RESULT_error_void(int code, enum EXC_ErrrorCategory category)
{
    RESULT_void res;
    res.has_value = UTIL_FALSE;
    res.error.category = category;
    res.error.code = code;
    return res;
}

/* Create an error result of type void with ERROR_CODE. */
RESULT_void RESULT_error_void_struct(struct EXC_ErrorCode error)
{
    RESULT_void res;
    res.has_value = UTIL_FALSE;
    res.error = error;
    return res;
}

/* Create a success result of type void. */
RESULT_void RESULT_ok_void(void)
{
    RESULT_void res;
    res.has_value = UTIL_TRUE;
    return res;
}

/* Return a void result with error code from last exception. */
RESULT_void RESULT_ERRNO_void(enum EXC_ErrrorCategory category)
{
    return RESULT_error_void_struct(EXC_get_last_error(category));
}

/* Return a result with error code from last exception. */
#define RESULT_ERRNO(ResultType, category) RESULT_error_##ResultType##_struct(EXC_get_last_error(category))

#if 0
#pragma endregion

#pragma region types
#endif

RESULT_GENERIC(UTIL_SignedSize)
RESULT_GENERIC(uint64_t)
RESULT_GENERIC(UTIL_ConstString)

#if 0
#pragma endregion

#pragma region utils
#endif

/* Duplicate memory block SRC of size N. Return NULL on error. */
void *UTIL_memdup(const void *src, size_t n)
{
    void *dst;

    if (src == NULL || n == 0)
        return NULL;

    dst = malloc(n);
    if (dst == NULL)
        return NULL;

    memcpy(dst, src, n);

    return dst;
}

/* Duplicate string STR. Return NULL on error. */
UTIL_StringBuffer UTIL_strdup(const char *src)
{
    return (UTIL_StringBuffer)UTIL_memdup((src), strlen(src) + 1);
}

/* Print formatted string to BUFFER. Return number of characters printed or negative error code. */
RESULT_UTIL_SignedSize UTIL_vsprintf(UTIL_StringBuffer *buffer, UTIL_ConstString fmt, va_list args)
{
    RESULT_UTIL_SignedSize ret;
    UTIL_SignedSize res;
    UTIL_StringBuffer tmp_buffer;
    va_list ap_len;
    va_list ap_write;

    ret = RESULT_ok_UTIL_SignedSize(0);
    tmp_buffer = NULL;

    if (buffer == NULL || fmt == NULL)
    {
        abort();
    }

    *buffer = NULL;

    va_copy(ap_len, args);
    res = (UTIL_SignedSize)vsnprintf(NULL, 0, fmt, ap_len);
    va_end(ap_len);
    if (res < 0)
    {
        ret = RESULT_ERRNO(UTIL_SignedSize, EXC_CATEGORY_LIBC);
        goto FAIL;
    }

    tmp_buffer = (UTIL_StringBuffer)malloc((size_t)res + 1);
    if (!tmp_buffer)
    {
        ret = RESULT_ERRNO(UTIL_SignedSize, EXC_CATEGORY_LIBC);
        goto FAIL;
    }

    va_copy(ap_write, args);
    res = vsnprintf(tmp_buffer, res + 1, fmt, ap_write);
    va_end(ap_write);
    if (res < 0)
    {
        ret = RESULT_ERRNO(UTIL_SignedSize, EXC_CATEGORY_LIBC);
        goto FAIL;
    }

    *buffer = tmp_buffer;

    goto EXIT;

FAIL:
    free(tmp_buffer);
EXIT:
    return ret;
}

#if HAS_WINDOWS
/* Get current time in milliseconds. */
uint64_t UTIL_get_current_time_ms()
{
    FILETIME ft;
    ULARGE_INTEGER uli;

    GetSystemTimeAsFileTime(&ft);
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    /* Convert to milliseconds. */
    return (uint64_t)(uli.QuadPart / 10000);
}
#else
/* Get current time in milliseconds. */
uint64_t UTIL_get_current_time_ms(void)
{
    struct timespec ts;

WHY_THIS_FAILS:
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
    {
        goto WHY_THIS_FAILS;
    }

    return (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
#endif

#if HAS_WINDOWS
void UTIL_sleep_ms(uint64_t ms)
{
    if (ms == 0)
    {
        return;
    }

    Sleep((DWORD)ms);
}
#else
/* Sleep for MS milliseconds. */
void UTIL_sleep_ms(uint64_t ms)
{
    struct timespec ts;

    if ((time_t)ms <= 0)
    {
        return;
    }

    ts.tv_sec = (time_t)(ms / 1000);
    ts.tv_nsec = (time_t)(ms % 1000) * 1000000;

    nanosleep(&ts, NULL);
}
#endif

/* Timer structure. */
struct UTIL_Timer
{
    uint64_t start_time;  /* Start time in milliseconds. */
    uint64_t duration_ms; /* Duration in milliseconds. */
};

/* Start timer TIMER. */
void UTIL_timer_start(struct UTIL_Timer *timer)
{
    if (timer == NULL)
        abort();

    timer->start_time = UTIL_get_current_time_ms();
    timer->duration_ms = 0;
}

/* Reset timer TIMER. */
#define UTIL_reset_timer UTIL_timer_start

/* Get elapsed time in milliseconds since TIMER was started. */
uint64_t UTIL_timer_elapsed_ms(struct UTIL_Timer *timer)
{
    uint64_t now;

    if (timer == NULL)
        abort();

    now = UTIL_get_current_time_ms();
    return now - timer->start_time;
}

/* Check if TIMER has elapsed DURATION_MS milliseconds. Return UTIL_TRUE if yes, UTIL_FALSE if not. */
UTIL_Bool UTIL_timer_ontime(struct UTIL_Timer *timer, uint64_t duration_ms)
{
    if (timer == NULL)
        abort();

    return UTIL_timer_elapsed_ms(timer) >= duration_ms ? UTIL_TRUE : UTIL_FALSE;
}

/* String array structure. */
struct UTIL_StrArray
{
    size_t size;            /* Current number of elements. */
    size_t capacity;        /* Current capacity. */
    UTIL_StringBuffer *arr; /* Array of strings. */
};

/* Initialize string array ARR with initial capacity of INITIAL_CAPACITY. */
RESULT_void UTIL_str_array_init(struct UTIL_StrArray *arr, size_t initial_capacity)
{
    if (arr == NULL || initial_capacity == 0)
        abort();

    arr->arr = (UTIL_StringBuffer *)malloc(sizeof(UTIL_StringBuffer) * initial_capacity);
    if (arr->arr == NULL)
        return RESULT_ERRNO_void(EXC_CATEGORY_LIBC);

    arr->size = 0;
    arr->capacity = initial_capacity;

    return RESULT_ok_void();
}

/* Append string DATA to string array ARR.*/
RESULT_void UTIL_str_array_append(struct UTIL_StrArray *arr, UTIL_ConstString data)
{
    UTIL_StringBuffer *new_arr;
    size_t new_capacity;

    if (arr == NULL || data == NULL)
        abort();

    if (arr->size >= arr->capacity)
    {
        new_capacity = arr->capacity * 2;
        new_arr = (UTIL_StringBuffer *)realloc(arr->arr, sizeof(UTIL_StringBuffer) * new_capacity);
        if (new_arr == NULL)
            return RESULT_ERRNO_void(EXC_CATEGORY_LIBC);

        arr->arr = new_arr;
        arr->capacity = new_capacity;
    }

    arr->arr[arr->size] = UTIL_strdup(data);
    if (arr->arr[arr->size] == NULL)
        return RESULT_ERRNO_void(EXC_CATEGORY_LIBC);

    arr->size++;
    return RESULT_ok_void();
}

/* Check if DATA is in string array ARR. Return 1 if found, 0 if not found or on error. */
UTIL_Bool UTIL_str_in_array(UTIL_ConstString data, struct UTIL_StrArray *arr)
{
    size_t i;

    if (data == NULL || arr == NULL)
        return UTIL_FALSE;

    i = 0;

    while (i < arr->size)
    {
        if (strcmp(data, arr->arr[i]) == 0)
        {
            return UTIL_TRUE;
        }
        i++;
    }

    return UTIL_FALSE;
}

/* Free all resources associated with string array ARR. */
void UTIL_str_array_free(struct UTIL_StrArray *arr)
{
    size_t i;

    if (arr == NULL)
        return;

    for (i = 0; i < arr->size; i++)
    {
        free(arr->arr[i]);
    }

    free(arr->arr);
}

/* CRC16-CCITT polynomial. */
#define UTIL_CRC16_CCITT_POLY 0x1021

/* (Internal) CRC16-CCITT table. */
static uint16_t g_UTIL_crc16_table[256];

/* Initialize CRC16-CCITT table. */
void UTIL_init_crc16_table(void)
{
    int i;
    int j;
    uint16_t crc;

    for (i = 0; i < 256; ++i)
    {
        crc = (uint16_t)i << 8;
        for (j = 0; j < 8; ++j)
        {
            if (crc & 0x8000)
                crc = (uint16_t)((crc << 1) ^ UTIL_CRC16_CCITT_POLY);
            else
                crc = (uint16_t)(crc << 1);
        }
        g_UTIL_crc16_table[i] = crc;
    }
}

/* Initialize UTIL system. */
#define UTIL_initialize UTIL_init_crc16_table

/* Compute CRC16-CCITT hash of string KEY. TABLE_SIZE_2POWER is the size of hash table (must be a power of 2).
   It MUST be a power of 2, or undefined behavior will occur. */
uint16_t UTIL_str_hash(UTIL_ConstString key, uint16_t table_size_2power)
{
    UTIL_register uint16_t crc;

    crc = 0xFFFF;

    while (*key)
    {
        crc = (uint16_t)((crc << 8) ^ g_UTIL_crc16_table[((crc >> 8) ^ (*key++)) & 0xFF]); /* Can't understand. */
    }

    /* Only available when table_size_2power is a power of 2. */
    return (uint16_t)(crc & (table_size_2power - 1));
}

UTIL_Bool UTIL_is_file(UTIL_ConstString path)
{
    FILE *file;
#if HAS_SAFE_C_LIB
    errno_t sc_err;
#endif

    if (path == NULL)
        return UTIL_FALSE;

#if HAS_SAFE_C_LIB
    sc_err = fopen_s(&file, path, "r");
    if (sc_err != 0)
        return UTIL_FALSE;
#else
    file = fopen(path, "r");
    if (file == NULL)
        return UTIL_FALSE;
#endif

    fclose(file);
    return UTIL_TRUE;
}

/* Maximum value for uint64_t divided by 10. */
#define UINT64_MAX_DIV10 UINT64_MAX / 10

/* Convert string STR to unsigned base 10 integer. */
RESULT_uint64_t UTIL_str_to_unsigned_base10(UTIL_ConstString str)
{
    uint64_t result;
    int digit;

    if (str == NULL)
        abort();

    result = 0;

    /* Skip leading whitespace. */
    while (isspace((unsigned char)*str))
    {
        str++;
    }

    if (*str == '\0')
    {
        /* Empty string or only whitespace. */
        return RESULT_error_uint64_t(EINVAL, EXC_CATEGORY_LIBC);
    }

    while (*str)
    {
        if (!isdigit((unsigned char)*str))
        {
            /* Encountered non-digit character, return failure. */
            return RESULT_error_uint64_t(EINVAL, EXC_CATEGORY_LIBC);
        }

        digit = *str - '0'; /* Convert character to digit. */

        /* Check for overflow. */
        if (result > UINT64_MAX_DIV10 || (result == UINT64_MAX_DIV10 && digit > 7))
        {
            /* Overflow case. */
            return RESULT_error_uint64_t(ERANGE, EXC_CATEGORY_LIBC);
        }

        /* Update result. */
        result = result * 10 + digit;

        str++;
    }

    return RESULT_ok_uint64_t(result);
}

#if 0
#pragma endregion

#pragma region fmt
#endif

/* Maximum number of arguments supported. MUST <= 9 */
#define FMT_MAX_ARGS 9

/* (Internal) Count required buffer size (including terminating NUL) using pre-cached args.
   We do NOT consume a va_list here; instead, caller provides ARGVALS[] and
   provided count (number of valid entries). Placeholders referencing indices
   beyond provided args are emitted literally (as "%n"). Returns -1 on error. */
static UTIL_SignedSize FMT__count_with_args(UTIL_ConstString fmt, UTIL_ConstString argvals[], int provided)
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
                        out_len += strlen(argvals[idx - 1]);
                    }
                    else
                    {
                        /* Do not substitute: emit literal "%<digits>". */
                        out_len += (r - p);
                    }
                }
                else
                {
                    /* Index out of supported range: emit literal "%<digits>". */
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
    if (out_len >= (size_t)UTIL_SSIZE_MAX - 1)
    {
        ret = -1;
        return ret;
    }

    ret = (UTIL_SignedSize)(out_len + 1);
    return ret;
}

/*
Simple formatter supporting placeholders %1 .. %9 and escaped percent %%
Behavior and constraints (design choices explained in comments below):
 - Only supports placeholders %1 .. %9 (single digit).
 - "%%" emits a single '%' and consumes no argument.
 - For safety (to avoid reading past provided varargs) this implementation
   treats a NULL vararg pointer as the end-of-arguments sentinel:
     e.g. FMT_format("a=%1 b=%2", "x", "y", NULL) -> OK
     if the caller provides fewer args and does NOT provide a NULL sentinel,
     reading further via va_arg would be undefined. To avoid that undefined
     behavior, we stop reading args when we see a NULL.
   This means callers who want to pass an actual NULL as an argument cannot
   do so with this implementation (trade-off for safety per request).
 - If a format contains a placeholder %n (1..9) but the caller did not
   provide that many arguments (we determined provided args by stopping at the
   first NULL or after the highest digit referenced up to 9), then we do NOT
   substitute it; instead we emit the literal characters "%n". This avoids
   consuming/reading non-existent arguments.

Note: Because standard C varargs have no built-in count, the safest practical
approach is to require a NULL sentinel if the caller may provide fewer
arguments than the highest placeholder referenced. Example usage:
  FMT_format("%1-%2-%3", "a", "b", "c");               // ok
  FMT_format("%1-%2-%3-%4", "a", "b", "c", NULL);     // ok, %4 won't be formatted

The function FMT_vformat formats a string using the given format specifier and
a va_list of arguments. The formatted string is stored in the buffer pointed to by
BUFFER. If BUFFER is NULL, the function returns the required buffer size (including
terminating NUL) without formatting.
*/
RESULT_void FMT_vformat(UTIL_StringBuffer *buffer, UTIL_ConstString fmt, va_list ap)
{
    UTIL_StringBuffer buf;
    UTIL_SignedSize need;
    size_t bufsize;
    int max_idx;
    UTIL_ConstString argvals[FMT_MAX_ARGS];
    int provided;
    UTIL_ConstString p;
    UTIL_StringBuffer w;
    size_t remaining;
    int i;

    if (fmt == NULL || buffer == NULL)
        abort();

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
                    if (idx > max_idx)
                        max_idx = idx;
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
        argvals[i] = NULL;
    provided = 0;

    if (max_idx > 0)
    {
        /* Read arguments up to max_idx, stopping at first NULL sentinel. */
        for (i = 0; i < max_idx; i++)
        {
            UTIL_ConstString s;
            s = va_arg(ap, UTIL_ConstString);
            if (s == NULL)
            {
                /* Treat NULL as end-of-args sentinel. */
                break;
            }
            argvals[i] = s;
            provided++;
        }
    }

    /* Determine required size using cached args. */
    need = FMT__count_with_args(fmt, argvals, provided);
    if (need <= 0)
        return RESULT_error_void(EINVAL, EXC_CATEGORY_LIBC);

    bufsize = (size_t)need;
    buf = (UTIL_StringBuffer)malloc(bufsize);
    if (!buf)
        return RESULT_ERRNO_void(EXC_CATEGORY_LIBC);

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
                if (remaining < 2)
                    goto ERR_FREE_BUF;
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
                        UTIL_ConstString s;
                        size_t slen;

                        s = argvals[idx - 1];
                        slen = strlen(s);
                        if (slen >= remaining)
                            goto ERR_FREE_BUF;
                        memcpy(w, s, slen);
                        w += slen;
                        remaining -= slen;
                    }
                    else
                    {
                        size_t litlen;

                        litlen = (size_t)(r - p);
                        if (litlen >= remaining)
                            goto ERR_FREE_BUF;
                        memcpy(w, p, litlen);
                        w += litlen;
                        remaining -= litlen;
                    }
                }
                else
                {
                    size_t litlen;

                    litlen = (size_t)(r - p);
                    if (litlen >= remaining)
                        goto ERR_FREE_BUF;
                    memcpy(w, p, litlen);
                    w += litlen;
                    remaining -= litlen;
                }
                p = r;
                continue;
            }
            else
            {
                if (remaining < 2)
                    goto ERR_FREE_BUF;
                *w++ = '%';
                remaining -= 1;
                p++;
                continue;
            }
        }
        else
        {
            if (remaining < 2)
                goto ERR_FREE_BUF;
            *w++ = *p++;
            remaining -= 1;
        }
    }

    if (remaining < 1)
        goto ERR_FREE_BUF;
    *w = '\0';
    *buffer = buf;
    return RESULT_ok_void();

ERR_FREE_BUF:
    free(buf);
    return RESULT_error_void(EINVAL, EXC_CATEGORY_LIBC);
}

/* Formats a string using the given format specifier and arguments. Stores the result in BUFFER. */
RESULT_void FMT_format(UTIL_StringBuffer *buffer, UTIL_ConstString fmt, ...)
{
    va_list ap;
    RESULT_void ret;

    va_start(ap, fmt);
    ret = FMT_vformat(buffer, fmt, ap);
    va_end(ap);

    return ret;
}

#if 0
#pragma endregion

#pragma region map
#endif

/* A string:any map node. */
struct MAP_Node
{
    UTIL_StringBuffer key; /* Malloc-allocated string key. */
    void *value;           /* Malloc-allocated value. */
    struct MAP_Node *next; /* Next node in bucket (linked list for collision resolution). */
    uint16_t hash;         /* Cached hash value to avoid recomputation. */
};

/* Pointer to MAP_Node. */
typedef struct MAP_Node *MAP_NodePtr;

/* Result type for MAP_NodePtr. */
RESULT_GENERIC(MAP_NodePtr)

/* A string:any map. */
struct MAP_Map
{
    struct MAP_Node **buckets; /* Array of buckets. */
    uint16_t capacity;         /* Capacity (2^capacity_power). */
    uint16_t size;             /* Current number of elements. */
    uint16_t capacity_power;   /* Exponent of capacity (capacity = 1 << capacity_power). */
};

/* Create a new MAP_Map to MAP with initial capacity 2^INIT_CAPACITY_POWER. Notice that INIT_CAPACITY_POWER must be in
   range [4, 14], and you should pre-allocate memory for the map struct. */
RESULT_void MAP_create(struct MAP_Map *map, uint16_t init_capacity_power)
{
    RESULT_void ret;

    ret = RESULT_ok_void();

    if (map == NULL)
        abort();

    memset(map, 0, sizeof(struct MAP_Map));

    init_capacity_power = UTIL_min(UTIL_max(init_capacity_power, 4), 14); /* 16 <= capacity <= 16384 */

    map->capacity_power = init_capacity_power;
    map->capacity = 1 << init_capacity_power; /* pow(2, capacity_power) */
    map->size = 0;

    /* Allocate memory for bucket array. */
    map->buckets = (struct MAP_Node **)calloc(map->capacity, sizeof(struct MAP_Node *));
    if (!map->buckets)
    {
        ret = RESULT_ERRNO_void(EXC_CATEGORY_LIBC);
        goto FAIL;
    }

    return ret;
FAIL:
    free(map->buckets);
    map->buckets = NULL;
    return ret;
}

/* (Internal) Resize the MAP to double its current capacity. */
static RESULT_void MAP__resize(struct MAP_Map *map)
{
    RESULT_void ret;
    uint16_t old_capacity;
    struct MAP_Node **old_buckets;
    uint16_t idx;

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
    map->buckets = (struct MAP_Node **)calloc(map->capacity, sizeof(struct MAP_Node *));
    if (map->buckets == NULL)
    {
        map->buckets = old_buckets;
        map->capacity = old_capacity;
        map->capacity_power--;
        ret = RESULT_ERRNO_void(EXC_CATEGORY_LIBC);
        goto EXIT;
    }

    /* Rehash all elements. */
    for (idx = 0; idx < old_capacity; idx++)
    {
        struct MAP_Node *node;

        node = old_buckets[idx];
        while (node)
        {
            struct MAP_Node *next;
            uint16_t new_index;

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

/* (Internal) Create a new MAP_Node with the given KEY, VALUE, and HASH. If error, set libc error and return NULL. */
static RESULT_MAP_NodePtr MAP__create_node(UTIL_StringBuffer key, void *value, uint16_t hash)
{
    struct MAP_Node *node;

    if (key == NULL || value == NULL)
        abort();

    node = (struct MAP_Node *)malloc(sizeof(struct MAP_Node));
    if (node == NULL)
    {
        return RESULT_ERRNO(MAP_NodePtr, EXC_CATEGORY_LIBC);
    }

    node->key = key;

    node->value = value;
    node->hash = hash;
    node->next = NULL;

    return RESULT_ok_MAP_NodePtr(node);
}

/* Set a key-value pair in the MAP. If the KEY already exists, update its value. Notice that the KEY and VALUE **MUST**
   be malloc-allocated and the map will take ownership of them. */
RESULT_void MAP_set(struct MAP_Map *map, UTIL_StringBuffer key, void *value)
{
    RESULT_void ret;
    uint16_t hash;
    uint16_t index;
    struct MAP_Node *node;
    struct MAP_Node *new_node;
    RESULT_MAP_NodePtr new_node_res;

    ret = RESULT_ok_void();
    new_node = NULL;

    if (map == NULL || key == NULL || value == NULL)
        abort();

    /* Check if resizing is needed. */
    ret = MAP__resize(map);
    if (!ret.has_value)
    {
        return ret;
    }

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

/* Get the value associated with the given KEY in the MAP. Returns NULL if the KEY is not found. Don't free the returned
   value because it is managed by the MAP. */
const void *MAP_get(const struct MAP_Map *map, UTIL_ConstString key)
{
    uint16_t hash;
    uint16_t index;
    struct MAP_Node *node;

    if (!map || !key)
        abort();

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

/* Check if the MAP contains the given KEY. */
#define MAP_contains(map, key) (MAP_get(map, key) != NULL)

/* Remove the key-value pair with the given KEY from the MAP. If the KEY does not exist, nothing will happen. */
void MAP_remove(struct MAP_Map *map, UTIL_ConstString key)
{
    uint16_t hash;
    uint16_t index;
    struct MAP_Node *prev;
    struct MAP_Node *node;

    if (map == NULL || key == NULL)
        return;

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

/* Clear all key-value pairs in the MAP. But do not free the MAP's bucket array. */
void MAP_clear(struct MAP_Map *map)
{
    uint16_t i;

    if (map == NULL)
        return;

    for (i = 0; i < map->capacity; i++)
    {
        struct MAP_Node *node;

        node = map->buckets[i];
        while (node)
        {
            struct MAP_Node *next;

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

/* Free all resources associated with the MAP. But do not free the MAP structure itself. It may be stack-allocated. */
void MAP_free(struct MAP_Map *map)
{
    if (map == NULL)
        return;

    MAP_clear(map);
    free(map->buckets);
}

#if 0
#pragma endregion

#pragma region logging
#endif

/* Logging level. */
enum LOGGING_LogLevel
{
    LOG_LEVEL_INVALID = -1,

    /* Debug level. */
    LOG_LEVEL_DEBUG = 0,
    /* Info level. */
    LOG_LEVEL_INFO = 1,
    /* Warning level. */
    LOG_LEVEL_WARN = 2,
    /* Error level. */
    LOG_LEVEL_ERROR = 3,

    LOG_LEVEL_MAX
};

/* Validate logging level. */
#define LOGGING_is_valid_level(level) ((level) >= LOG_LEVEL_DEBUG && (level) <= LOG_LEVEL_ERROR)

/* Logging formats. */
typedef UTIL_ConstString LOGGING_LogFormats[4];

/* (Internal) Current logging level. */
static enum LOGGING_LogLevel g_LOGGING_current_level;

/* (Internal) Logging formats. */
static LOGGING_LogFormats g_LOGGING_format;

/* Set logging level to LEVEL. */
#define LOGGING_set_level(level)                                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        g_LOGGING_current_level = (level);                                                                             \
    } while (0)

/* Get current logging level. */
#define LOGGING_get_level() (g_LOGGING_current_level)

/* Set logging format for LEVEL to FMT. */
#define LOGGING_set_format(level, fmt)                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        g_LOGGING_format[level] = (fmt);                                                                               \
    } while (0)

/* Initialize logging. */
void LOGGING_initialize(void)
{
    LOGGING_set_level(LOG_LEVEL_DEBUG);
    LOGGING_set_format(LOG_LEVEL_DEBUG, "%2");
    LOGGING_set_format(LOG_LEVEL_INFO, "%2");
    LOGGING_set_format(LOG_LEVEL_WARN, "%2");
    LOGGING_set_format(LOG_LEVEL_ERROR, "%2");
}

/* Convert logging level to string. */
UTIL_ConstString LOGGING_log_level_tostring(enum LOGGING_LogLevel level)
{
    switch (level)
    {
    case LOG_LEVEL_DEBUG:
        return "DEBUG";
    case LOG_LEVEL_INFO:
        return "INFO";
    case LOG_LEVEL_WARN:
        return "WARN";
    case LOG_LEVEL_ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

/* Log a message with level LEVEL and format FMT using variable argument list ARGS. */
void LOGGING_vlog(enum LOGGING_LogLevel level, UTIL_ConstString fmt, va_list args)
{
    RESULT_UTIL_SignedSize ret;
    RESULT_void ret2;
    unsigned long ret3;
    UTIL_StringBuffer buffer1;
    UTIL_StringBuffer buffer2;
    size_t buffer_nmemb;

    buffer1 = NULL;
    buffer2 = NULL;

    ret = UTIL_vsprintf(&buffer1, fmt, args);
    if (!ret.has_value)
    {
        perror("(logging)UTIL_vsprintf");
        goto EXIT;
    }

    ret2 = FMT_format(&buffer2, g_LOGGING_format[level], LOGGING_log_level_tostring(level), buffer1, NULL);
    if (!ret2.has_value)
    {
        perror("FMT_format");
        goto EXIT;
    }

    buffer_nmemb = strlen(buffer2);
    ret3 = fwrite(buffer2, 1, buffer_nmemb, stderr);
    if (ret3 < buffer_nmemb)
    {
        perror("(logging)fwrite");
        goto EXIT;
    }
    ret3 = fwrite("\n", 1, 1, stderr);
    if (ret3 < 1)
    {
        perror("(logging)fwrite");
        goto EXIT;
    }
    if (fflush(stderr) != 0)
    {
        perror("(logging)fflush");
        goto EXIT;
    }

EXIT:
    free(buffer1);
    free(buffer2);
}

/* Check if the logfile supports color output. */
UTIL_Bool LOGGING_logfile_supports_color(void)
{
    return isatty(fileno(stderr)) ? UTIL_TRUE : UTIL_FALSE;
}

/* Log a message with level LEVEL and format FMT. */
void LOGGING_log(enum LOGGING_LogLevel level, UTIL_ConstString fmt, ...)
{
    va_list args;

    if (level < LOGGING_get_level())
    {
        return;
    }

    va_start(args, fmt);

    LOGGING_vlog(level, fmt, args);

    va_end(args);
}

#if LOGGING_LEVEL >= LOG_LEVEL_DEBUG
/* Log a debug message with format FMT. */
void LOGGING_debug(UTIL_ConstString fmt, ...)
{
    va_list args;

    if (LOG_LEVEL_DEBUG < LOGGING_get_level())
    {
        return;
    }

    va_start(args, fmt);

    LOGGING_vlog(LOG_LEVEL_DEBUG, fmt, args);

    va_end(args);
}
#else
/* Log a debug message with format FMT (disabled). */
#define LOGGING_debug(fmt, ...)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
    } while (0)
#endif

#if LOGGING_LEVEL >= LOG_LEVEL_INFO
/* Log an info message with format FMT. */
void LOGGING_info(UTIL_ConstString fmt, ...)
{
    va_list args;

    if (LOG_LEVEL_INFO < LOGGING_get_level())
    {
        return;
    }

    va_start(args, fmt);

    LOGGING_vlog(LOG_LEVEL_INFO, fmt, args);

    va_end(args);
}
#else
/* Log an info message with format FMT (disabled). */
#define LOGGING_info(fmt, ...)                                                                                         \
    do                                                                                                                 \
    {                                                                                                                  \
    } while (0)
#endif

#if LOGGING_LEVEL >= LOG_LEVEL_WARN
/* Log a warning message with format FMT. */
void LOGGING_warn(UTIL_ConstString fmt, ...)
{
    va_list args;

    if (LOG_LEVEL_WARN < LOGGING_get_level())
    {
        return;
    }

    va_start(args, fmt);

    LOGGING_vlog(LOG_LEVEL_WARN, fmt, args);

    va_end(args);
}
#else
/* Log a warning message with format FMT (disabled). */
#define LOGGING_warn(fmt, ...)                                                                                         \
    do                                                                                                                 \
    {                                                                                                                  \
    } while (0)
#endif

#if LOGGING_LEVEL >= LOG_LEVEL_ERROR
/* Log an error message with format FMT. */
void LOGGING_error(UTIL_ConstString fmt, ...)
{
    va_list args;

    if (LOG_LEVEL_ERROR < LOGGING_get_level())
    {
        return;
    }

    va_start(args, fmt);

    LOGGING_vlog(LOG_LEVEL_ERROR, fmt, args);

    va_end(args);
}
#else
/* Log an error message with format FMT (disabled). */
#define LOGGING_error(fmt, ...)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
    } while (0)
#endif

/* Log an error message with message MSG and error code EC. */
void LOGGING_perror(UTIL_ConstString msg, struct EXC_ErrorCode ec)
{
    LOGGING_error("%s: %s", msg, EXC_strerror(ec));
}

/* Log an warning message with message MSG and error code EC. */
void LOGGING_pwarn(UTIL_ConstString msg, struct EXC_ErrorCode rc)
{
    LOGGING_warn("%s: %s", msg, EXC_strerror(rc));
}

#if 0
#pragma endregion

#pragma region net
#endif

/* Network socket type. */
typedef int NET_Socket;

RESULT_GENERIC(NET_Socket)

/* Invalid socket constant. */
#define NET_INVALID_SOCKET (NET_Socket)(-1)

/* Internet protocol version. */
enum NET_InetVersion
{
    /* IPv4 */
    NET_INET_VERSION_4 = AF_INET,
    /* IPv6 */
    NET_INET_VERSION_6 = AF_INET6
};

/* Check if the given internet version is supported. */
#define NET_inet_version_is_supported(ver) ((ver) == NET_INET_VERSION_4 || (ver) == NET_INET_VERSION_6)

/* Internet address structures in local byte order. */
typedef uint8_t NET_Inet4Addr[4];

/* IPv6 address as an array of 8 16-bit segments in local byte order. */
typedef uint16_t NET_Inet6Addr[8];

/* Generic internet address structure. */
struct NET_InetAddr
{
    /* Internet protocol version. */
    enum NET_InetVersion version;
    union {
        /* IPv4 address. */
        NET_Inet4Addr inet4;
        /* IPv6 address. */
        NET_Inet6Addr inet6;
    } addr;
};

/* Initialize the networking subsystem. */
RESULT_void NET_initialize(void)
{
    return RESULT_ok_void();
}

/* Deinitialize the networking subsystem. */
RESULT_void NET_deinitialize(void)
{
    return RESULT_ok_void();
}

/* Close a socket. */
#define NET_closesocket(s)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        if ((s) >= 0)                                                                                                  \
            close(s);                                                                                                  \
    } while (0)

/* Check if the error code ERR indicates that the network buffer is full. */
#define NET_kern_buffer_is_full(err) (-(err.code) == ENOBUFS)

/* Create an asynchronous socket for the given internet version INET_VERSION. */
RESULT_NET_Socket NET_async_socket(enum NET_InetVersion inet_version)
{
    NET_Socket sock;
    int optval;

    sock = (NET_Socket)socket(inet_version, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    if (sock < 0)
    {
        return RESULT_ERRNO(NET_Socket, EXC_CATEGORY_NET);
    }

    /* Enable address reuse. */
    optval = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        goto FREE_SOCK;
    }

    return RESULT_ok_NET_Socket(sock);
FREE_SOCK:
    NET_closesocket(sock);
    return RESULT_ERRNO(NET_Socket, EXC_CATEGORY_NET);
}

/* Load the address of a sockaddr_storage ADDR into OUT_ADDR. */
void NET_load_addr(struct sockaddr_storage *addr, struct NET_InetAddr *out_addr)
{
    if (addr == NULL || out_addr == NULL)
        abort();

    if (addr->ss_family == AF_INET)
    {
        struct sockaddr_in *addr_in;
        uint32_t ip_host;

        addr_in = (struct sockaddr_in *)addr;
        out_addr->version = NET_INET_VERSION_4;
        ip_host = ntohl(addr_in->sin_addr.s_addr);

        out_addr->addr.inet4[0] = (uint8_t)((ip_host >> 24) & 0xFF);
        out_addr->addr.inet4[1] = (uint8_t)((ip_host >> 16) & 0xFF);
        out_addr->addr.inet4[2] = (uint8_t)((ip_host >> 8) & 0xFF);
        out_addr->addr.inet4[3] = (uint8_t)(ip_host & 0xFF);
    }
    else if (addr->ss_family == AF_INET6)
    {
        struct sockaddr_in6 *addr_in6;
        size_t i;
        uint16_t segment;

        addr_in6 = (struct sockaddr_in6 *)addr;
        out_addr->version = NET_INET_VERSION_6;
        for (i = 0; i < 8; i++)
        {
            memcpy(&segment, &addr_in6->sin6_addr.s6_addr[i * 2], sizeof(uint16_t));
            segment = ntohs(segment);
            out_addr->addr.inet6[i] = segment;
        }
    }
    else
        abort(); /* Should never happen. */
}

/* Load the port of a ADDR into OUT_PORT. */
void NET_load_port(struct sockaddr_storage *addr, uint16_t *out_port)
{
    if (addr == NULL || out_port == NULL)
        abort();

    if (addr->ss_family == AF_INET)
    {
        struct sockaddr_in *addr_in;

        addr_in = (struct sockaddr_in *)addr;
        *out_port = ntohs(addr_in->sin_port);
    }
    else if (addr->ss_family == AF_INET6)
    {
        struct sockaddr_in6 *addr_in6;

        addr_in6 = (struct sockaddr_in6 *)addr;
        *out_port = ntohs(addr_in6->sin6_port);
    }
    else
        abort(); /* Should never happen. */
}

/* Convert the internet address ADDR to a string representation in OUT_BUF. */
void NET_repr_inet_addr(const struct NET_InetAddr *addr, UTIL_StringBuffer out_buf, size_t out_buf_len)
{
    if (addr->version == NET_INET_VERSION_4)
    {
        snprintf(out_buf, out_buf_len, "%u.%u.%u.%u", addr->addr.inet4[0], addr->addr.inet4[1], addr->addr.inet4[2],
                 addr->addr.inet4[3]);
    }
    else if (addr->version == NET_INET_VERSION_6)
    {
        snprintf(out_buf, out_buf_len, "%x:%x:%x:%x:%x:%x:%x:%x", addr->addr.inet6[0], addr->addr.inet6[1],
                 addr->addr.inet6[2], addr->addr.inet6[3], addr->addr.inet6[4], addr->addr.inet6[5],
                 addr->addr.inet6[6], addr->addr.inet6[7]);
    }
    else
        abort();
}

/* Maximum length of a string representation of an internet address. */
#define NET_ADDR_REPR_MAX_LENGTH 46

/* Receive data from a socket SK, storing the source address and port in SRC_ADDR and SRC_PORT. */
RESULT_UTIL_SignedSize NET_recvfrom(NET_Socket sock, void *buf, size_t len, int flags, struct NET_InetAddr *src_addr,
                                    uint16_t *src_port)
{
    UTIL_SignedSize received;
    struct sockaddr_storage addr;
    socklen_t addr_len;

    if (src_addr == NULL || src_port == NULL || buf == NULL || len == 0)
        abort();

    addr_len = sizeof(addr);

    received = (UTIL_SignedSize)recvfrom(sock, buf, len, flags, (struct sockaddr *)&addr, &addr_len);
    if (received < 0)
    {
        /* Convert EAGAIN. */
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return RESULT_error_UTIL_SignedSize(EFN_TRY_AGAIN, EXC_CATEGORY_FN);
        }
        return RESULT_ERRNO(UTIL_SignedSize, EXC_CATEGORY_NET);
    }

    NET_load_addr(&addr, src_addr);
    NET_load_port(&addr, src_port);

    return RESULT_ok_UTIL_SignedSize(received);
}

/* Send data to the specified destination address and port. */
RESULT_void NET_sendto(NET_Socket sock, const void *buf, size_t len, int flags, const struct NET_InetAddr *dest_addr,
                       uint16_t dest_port)
{
    struct sockaddr_storage addr;
    socklen_t addr_len;

    if (dest_addr == NULL || buf == NULL || len == 0)
        abort();

    memset(&addr, 0, sizeof(addr));

    /* Fill in address structure. */
    if (dest_addr->version == NET_INET_VERSION_4)
    {
        struct sockaddr_in *addr_in;

        addr_in = (struct sockaddr_in *)&addr;
        addr_in->sin_family = AF_INET;
        addr_in->sin_port = htons(dest_port);
        addr_in->sin_addr.s_addr =
            htonl((uint32_t)dest_addr->addr.inet4[0] << 24 | (uint32_t)dest_addr->addr.inet4[1] << 16 |
                  (uint32_t)dest_addr->addr.inet4[2] << 8 | (uint32_t)dest_addr->addr.inet4[3]);
        addr_len = sizeof(struct sockaddr_in);
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
            uint16_t segment;

            segment = htons(dest_addr->addr.inet6[i]);
            memcpy(&addr_in6->sin6_addr.s6_addr[i * 2], &segment, sizeof(uint16_t));
        }
        addr_len = sizeof(struct sockaddr_in6);
    }
    else
        abort(); /* Never happen. */

    if (sendto(sock, buf, len, flags, (struct sockaddr *)&addr, addr_len) < 0)
    {
        /* Convert EAGAIN. */
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return RESULT_error_void(EFN_TRY_AGAIN, EXC_CATEGORY_FN);
        }
        return RESULT_ERRNO_void(EXC_CATEGORY_NET);
    }

    return RESULT_ok_void();
}

/* Check if the error code ERR indicates a temporary failure (EAGAIN). */
UTIL_Bool NET_need_try_again(struct EXC_ErrorCode err)
{
    return (err.category == EXC_CATEGORY_FN) && (err.code == EFN_TRY_AGAIN);
}

#if 0
#pragma endregion

#pragma region dns
#endif

/* DNS header: standard sections and fields. */
enum DNS_Opcode
{
    /* Only opcode 0 (standard query) is supported. */
    DNS_OPCODE_QUERY = 0
};

/* DNS header: standard sections and fields. */
struct DNS_Header
{
    uint16_t id;         /* DNS message ID. */
    uint16_t flags;      /* DNS flags. */
    uint16_t questions;  /* Number of questions. */
    uint16_t answer;     /* Number of answer RRs. */
    uint16_t authority;  /* Number of authority RRs. */
    uint16_t additional; /* Number of additional RRs. */
};

/* Fill DNS header HEADER from a byte buffer BUF with length LEN in network byte order. */
void DNS_fill_header(struct DNS_Header *header, uint8_t *buf, size_t len)
{
    if (header == NULL || buf == NULL || len < sizeof(struct DNS_Header))
        abort();

    memcpy(header, buf, sizeof(struct DNS_Header));
    header->id = ntohs(header->id);
    header->flags = ntohs(header->flags);
    header->questions = ntohs(header->questions);
    header->answer = ntohs(header->answer);
    header->authority = ntohs(header->authority);
    header->additional = ntohs(header->additional);
}

/* Header parsing and creation above; length helpers are defined after structs. */
struct DNS_Question
{
    size_t length;   /* Total length of this question. */
    uint16_t qtype;  /* Query type. */
    uint16_t qclass; /* Query class. */
    uint8_t *name;   /* Query name. (caller frees) */
};

/* Free a DNS_Question structure Q. */
#define DNS_free_question(q) free((q)->name)

/* Parse a DNS name (label format) and return a dot-separated string.
   Basic labels supported; compression pointers (0xC0) are not handled.
   BUF: packet buffer.
   LEN: total packet length.
   OFFSET: starting offset of name.
   OUT_NAME: output domain string (malloc'ed; caller frees).
   CONSUMED: bytes consumed including terminal 0. */
RESULT_void DNS_parse_name(const uint8_t *buf, size_t len, size_t offset, uint8_t **out_name, size_t *consumed)
{
    RESULT_void ret;
    UTIL_StringBuffer tmp;
    size_t tmp_pos;
    size_t pos;
    int first_label;

    if (buf == NULL || out_name == NULL || consumed == NULL || offset >= len)
        abort();

    ret = RESULT_ok_void();
    tmp = NULL;
    tmp_pos = 0;
    pos = offset;
    first_label = 1;

    /* Use a temporary buffer up to remaining packet length. */
    tmp = (UTIL_StringBuffer)malloc(len - offset + 1);
    if (tmp == NULL)
    {
        return RESULT_ERRNO_void(EXC_CATEGORY_LIBC);
    }

    while (pos < len)
    {
        uint8_t lab_len;

        lab_len = buf[pos++];
        if (lab_len == 0)
        {
            /* End of name. */
            break;
        }
        /* Compression pointers (top two bits 11) are not supported here.
           In this context we only expect an uncompressed single record. */
        if ((lab_len & 0xC0) == 0xC0)
        {
            /* Name compression requires pointer jumps; not implemented. */
            ret = RESULT_error_void(EFN_COMPRESSION_PTR_UNSUPPORTED, EXC_CATEGORY_FN);
            goto FAIL;
        }
        if (pos + lab_len > len)
        {
            /* Label length exceeds remaining packet length. */
            ret = RESULT_error_void(EFN_MAX_CAPACITY_EXCEEDED, EXC_CATEGORY_FN);
            goto FAIL;
        }
        if (!first_label)
        {
            tmp[tmp_pos++] = '.';
        }
        first_label = 0;
        /* Copy label content into the temporary buffer. */
        memcpy(tmp + tmp_pos, buf + pos, lab_len);
        tmp_pos += lab_len;
        pos += lab_len;
    }

    if (pos > len)
    {
        ret = RESULT_error_void(EFN_MAX_CAPACITY_EXCEEDED, EXC_CATEGORY_FN);
        goto FAIL;
    }

    tmp[tmp_pos] = '\0';
    *out_name = (uint8_t *)tmp;
    *consumed = (pos - offset); /* Includes terminal 0 byte. */

    return ret;
FAIL:
    free(tmp);
    return ret;
}

/* Parse a DNS_Question: name + qtype + qclass.
   Returns 0 on success; fields are host byte order (qtype/qclass via ntohs). Name is malloc'ed; caller frees. */
RESULT_void DNS_parse_question(const uint8_t *buf, size_t len, size_t offset, struct DNS_Question *q)
{
    RESULT_void ret;
    uint8_t *name;
    size_t name_consumed;
    size_t pos;
    uint16_t net_qtype;
    uint16_t net_qclass;

    if (buf == NULL || q == NULL || offset >= len)
        abort();

    ret = RESULT_ok_void();
    memset(q, 0, sizeof(*q));
    name = NULL;
    name_consumed = 0;

    /* Parse domain name. */
    ret = DNS_parse_name(buf, len, offset, &name, &name_consumed);
    if (!ret.has_value)
    {
        goto EXIT;
    }

    pos = offset + name_consumed;
    /* Need at least 4 bytes after name for qtype + qclass. */
    if (pos + 4 > len)
    {
        ret = RESULT_error_void(EFN_MAX_CAPACITY_EXCEEDED, EXC_CATEGORY_FN);
        goto EXIT;
    }

    net_qtype = 0;
    net_qclass = 0;
    memcpy(&net_qtype, buf + pos, sizeof(uint16_t));
    memcpy(&net_qclass, buf + pos + 2, sizeof(uint16_t));

    q->name = name;
    q->qtype = ntohs(net_qtype);
    q->qclass = ntohs(net_qclass);
    q->length = (uint32_t)(name_consumed + 4); /* Total bytes consumed for question. */

    name = NULL; /* Transferred ownership. */

EXIT:
    free(name);
    return ret;
}

/* DNS Resource Record structure. */
struct DNS_ResourceRecord
{
    UTIL_StringBuffer name; /* Domain name. (must be freed) */
    uint16_t type;          /* Record type. */
    uint16_t rclass;        /* Record class. */
    uint32_t ttl;           /* Time to live. */
    uint16_t rdlength;      /* RDATA length. */
    void *rdata;            /* Record data. (depends on type) */
};

/* Check if a DNS name is in compressed format (starts with 0xC0). */
#define DNS_record_name_is_compressed(name_ptr) ((((uint8_t *)(name_ptr))[0] & 0xC0) == 0xC0)

/* Compute required response length (no content generation).
    Layout: Header(12) + Question(name+4) + each Answer(name+10+rdlength). */
size_t DNS_dns_name_encoded_len(UTIL_ConstString name)
{
    size_t total;
    UTIL_ConstString p;

    /* Calculate label-encoded length: 1 byte per label length + terminal \0 */
    if (!name)
        return 1; /* Only terminal \0. */

    total = 1; /* Includes terminal \0. */
    p = name;

    while (*p)
    {
        UTIL_ConstString dot;
        size_t lablen;

        dot = strchr(p, '.');
        lablen = dot ? (size_t)(dot - p) : strlen(p);
        total += 1 + lablen;
        if (!dot)
            break;
        p = dot + 1;
    }

    return total;
}

/* Compute required response length (no content generation).
    Layout: Header(12) + Question(name+4) + each Answer(name+10+rdlength). */
size_t DNS_get_response_len(const struct DNS_Question *q, const struct DNS_ResourceRecord *rr, size_t rr_count)
{
    size_t len;
    size_t answers_i;

    if (q == NULL || rr == NULL || rr_count == 0)
        return 0;

    len = sizeof(struct DNS_Header);
    /* Question. */
    len += DNS_dns_name_encoded_len((UTIL_ConstString)q->name);
    len += 4; /* qtype + qclass */

    /* Answers. */
    for (answers_i = 0; answers_i < rr_count; ++answers_i)
    {
        const struct DNS_ResourceRecord *r;

        r = &rr[answers_i];
        len += DNS_record_name_is_compressed(r->name) ? 2 : DNS_dns_name_encoded_len(r->name);
        len += 10; /* type(2) + class(2) + ttl(4) + rdlength(2) */
        len += r->rdlength;
    }

    return len;
}

/* Free a DNS_ResourceRecord structure RR. */
#define DNS_free_resource_record(rr)                                                                                   \
    do                                                                                                                 \
    {                                                                                                                  \
        free((rr)->name);                                                                                              \
        free((rr)->rdata);                                                                                             \
    } while (0)

/* Encode a dot-separated domain name into DNS label format.
   wlen is set to the number of bytes written. */
RESULT_void DNS_encode_name(UTIL_ConstString name, uint8_t *buf, size_t buf_len, size_t *wlen)
{
    UTIL_ConstString p;

    if (name == NULL || buf == NULL || buf_len == 0 || wlen == NULL)
        abort();

    *wlen = 0;
    p = name;
    while (*p)
    {
        UTIL_ConstString dot;
        size_t lablen;

        dot = strchr(p, '.');
        lablen = dot ? (size_t)(dot - p) : strlen(p);
        if (lablen > 63)
        {
            /* RFC limit: label length must not exceed 63 bytes. */
            return RESULT_error_void(EFN_MAX_CAPACITY_EXCEEDED, EXC_CATEGORY_FN);
        }
        if (*wlen + 1 + lablen + 1 > buf_len)
        {
            return RESULT_error_void(EFN_MAX_CAPACITY_EXCEEDED, EXC_CATEGORY_FN);
        }
        buf[(*wlen)++] = (uint8_t)lablen;
        memcpy(buf + *wlen, p, lablen);
        *wlen += lablen;
        if (!dot)
            break;
        p = dot + 1;
    }
    if (*wlen + 1 > buf_len)
    {
        return RESULT_error_void(EFN_MAX_CAPACITY_EXCEEDED, EXC_CATEGORY_FN);
    }
    buf[(*wlen)++] = 0; /* Terminal zero byte. */

    return RESULT_ok_void();
}

/* DNS record type. */
enum DNS_RecordType
{
    DNS_TYPE_A = 1,    /* A record. */
    DNS_TYPE_AAAA = 28 /* AAAA record. */
};

/* DNS record class. */
enum DNS_RecordClass
{
    DNS_CLASS_IN = 1 /* Internet class. */
};

/* Internet IPv4 address. */
typedef uint8_t DNS_Inet4Addr[4];
/* Internet IPv6 address. */
typedef uint8_t DNS_Inet6Addr[16];

/* Build an A record resource with NAME, IP_ADDR. Don't forget to free mallocated memory in RR. */
RESULT_void DNS_make_A_record(UTIL_ConstString name, DNS_Inet4Addr ip_addr, struct DNS_ResourceRecord *rr)
{
    RESULT_void ret;

    if (name == NULL || rr == NULL)
        abort();

    ret = RESULT_ok_void();
    rr->rdata = NULL;

    rr->name = UTIL_strdup(name);
    if (!rr->name)
    {
        ret = RESULT_ERRNO_void(EXC_CATEGORY_LIBC);
        goto EXIT;
    }

    rr->type = DNS_TYPE_A;
    rr->rclass = DNS_CLASS_IN;
    rr->ttl = 0;
    rr->rdlength = sizeof(DNS_Inet4Addr);
    rr->rdata = malloc(rr->rdlength);
    if (!rr->rdata)
    {
        ret = RESULT_ERRNO_void(EXC_CATEGORY_LIBC);
        goto EXIT;
    }
    memcpy(rr->rdata, ip_addr, rr->rdlength);

EXIT:
    if (!ret.has_value)
        DNS_free_resource_record(rr);
    return ret;
}

/* Build an AAAA record resource with NAME, IP_ADDR. Don't forget to free mallocated memory in RR. */
RESULT_void DNS_make_AAAA_record(UTIL_ConstString name, DNS_Inet6Addr ip_addr, struct DNS_ResourceRecord *rr)
{
    RESULT_void ret;

    if (name == NULL || rr == NULL)
        abort();

    ret = RESULT_ok_void();
    rr->rdata = NULL;

    rr->name = UTIL_strdup(name);
    if (!rr->name)
    {
        ret = RESULT_ERRNO_void(EXC_CATEGORY_LIBC);
        goto EXIT;
    }

    rr->type = DNS_TYPE_AAAA;
    rr->rclass = DNS_CLASS_IN;
    rr->ttl = 0;
    rr->rdlength = sizeof(DNS_Inet6Addr);
    rr->rdata = malloc(rr->rdlength);
    if (!rr->rdata)
    {
        ret = RESULT_ERRNO_void(EXC_CATEGORY_LIBC);
        goto EXIT;
    }
    memcpy(rr->rdata, ip_addr, rr->rdlength);

EXIT:
    if (!ret.has_value)
        DNS_free_resource_record(rr);

    return ret;
}

/* Build a DNS response from QUERY_HEADER, question Q, and answer records RR. */
RESULT_void DNS_make_response(const struct DNS_Header *query_header, const struct DNS_Question *q,
                              const struct DNS_ResourceRecord *rr, size_t rr_count, uint8_t *out_buf,
                              size_t out_buf_len, size_t *out_len)
{
    RESULT_void ret;
    size_t pos;
    struct DNS_Header hdr;
    uint16_t net_id;
    uint16_t net_flags;
    uint16_t net_questions;
    uint16_t net_answer;
    uint16_t net_authority;
    uint16_t net_additional;
    size_t name_encoded_len;
    uint16_t net_qtype;
    uint16_t net_qclass;
    size_t i;

    if (query_header == NULL || q == NULL || rr == NULL || rr_count == 0 || out_buf == NULL || out_len == NULL)
        abort();

    if (sizeof(struct DNS_Header) > out_buf_len)
    {
        return RESULT_error_void(EFN_MAX_CAPACITY_EXCEEDED, EXC_CATEGORY_FN);
    }

    ret = RESULT_ok_void();
    pos = 0;
    memset(&hdr, 0, sizeof(hdr));

    /* Use the incoming query ID for consistency. */
    hdr.id = query_header->id;
    /* Manually construct response flags:
       QR=1 (response), OPCODE=0 (standard query), AA=1 (authoritative),
       TC=0, RD=0, RA=0, Z=0, RCODE=0.
       RFC 1035 bit layout: QR(15) | OPCODE(14-11) | AA(10) | TC(9) | RD(8) | RA(7) | Z(6-4) | RCODE(3-0).
       0x8500 = 1000 0101 0000 0000 (host byte order). */
    hdr.flags = 0x8500;    /* Host-order response flags. */
    hdr.questions = 1;     /* Question count. */
    hdr.answer = rr_count; /* Answer count. */
    hdr.authority = 0;     /* Authority count. */
    hdr.additional = 0;    /* Additional count. */

    /* Write header in network byte order. */
    net_id = htons(hdr.id);
    net_flags = htons(hdr.flags);
    net_questions = htons(hdr.questions);
    net_answer = htons(hdr.answer);
    net_authority = htons(hdr.authority);
    net_additional = htons(hdr.additional);

    memcpy(out_buf + pos, &net_id, 2);
    memcpy(out_buf + pos + 2, &net_flags, 2);
    memcpy(out_buf + pos + 4, &net_questions, 2);
    memcpy(out_buf + pos + 6, &net_answer, 2);
    memcpy(out_buf + pos + 8, &net_authority, 2);
    memcpy(out_buf + pos + 10, &net_additional, 2);
    pos += sizeof(struct DNS_Header);

    /* Write the Question section (copied from query).
        Note: q->name is dot-separated and must be label-encoded. */
    name_encoded_len = 0;
    ret = DNS_encode_name((UTIL_ConstString)q->name, out_buf + pos, out_buf_len - pos, &name_encoded_len);
    if (!ret.has_value)
        goto EXIT;
    pos += name_encoded_len;

    /* Write qtype and qclass. (network byte order) */
    if (pos + 4 > out_buf_len)
    {
        ret = RESULT_error_void(EFN_MAX_CAPACITY_EXCEEDED, EXC_CATEGORY_FN);
        goto EXIT;
    }
    net_qtype = htons(q->qtype);
    net_qclass = htons(q->qclass);
    memcpy(out_buf + pos, &net_qtype, 2);
    memcpy(out_buf + pos + 2, &net_qclass, 2);
    pos += 4;

    /* Write Answer section. */
    for (i = 0; i < rr_count; i++)
    {
        uint16_t net_type;
        uint16_t net_class;
        uint32_t net_ttl;
        uint16_t net_rdlength;
        const struct DNS_ResourceRecord *r;

        r = &rr[i];

        /* 3.1 Write name using compression pointer to the Question name.
           Compression format: top two bits 11 (0xC0), remaining 14 bits are offset.
           Question name offset = DNS header size (12) = 0x0C. */
        if (pos + 2 > out_buf_len)
        {
            ret = RESULT_error_void(EFN_MAX_CAPACITY_EXCEEDED, EXC_CATEGORY_FN);
            goto EXIT;
        }
        out_buf[pos++] = 0xC0; /* Compression pointer marker. */
        out_buf[pos++] = 0x0C; /* Offset pointing to Question name. */

        /* 3.2 Write type, class, TTL, and RDLENGTH. (network byte order) */
        if (pos + 10 > out_buf_len)
        {
            ret = RESULT_error_void(EFN_MAX_CAPACITY_EXCEEDED, EXC_CATEGORY_FN);
            goto EXIT;
        }

        net_type = htons(r->type);
        net_class = htons(r->rclass);
        net_ttl = htonl(r->ttl);
        net_rdlength = htons(r->rdlength);

        memcpy(out_buf + pos, &net_type, 2);
        memcpy(out_buf + pos + 2, &net_class, 2);
        memcpy(out_buf + pos + 4, &net_ttl, 4);
        memcpy(out_buf + pos + 8, &net_rdlength, 2);
        pos += 10;

        /* 3.3 Write RDATA. */
        if (pos + r->rdlength > out_buf_len)
        {
            ret = RESULT_error_void(EFN_MAX_CAPACITY_EXCEEDED, EXC_CATEGORY_FN);
            goto EXIT;
        }
        memcpy(out_buf + pos, r->rdata, r->rdlength);
        pos += r->rdlength;
    }

EXIT:
    *out_len = pos;
    return ret;
}

/* Bind the socket SK to ADDR:PORT for the given INET_VERSION.
   ADDR must be a string representation of the IP address. */
RESULT_void DNS_bind_socket(NET_Socket sk, UTIL_ConstString addr, uint16_t port, enum NET_InetVersion inet_version)
{
    if (addr == NULL)
        abort();

    if (inet_version == NET_INET_VERSION_4)
    {
        struct sockaddr_in sa_in;

        memset(&sa_in, 0, sizeof(sa_in));
        sa_in.sin_family = AF_INET;
        sa_in.sin_port = htons(port);
        if (inet_pton(AF_INET, addr, &sa_in.sin_addr) != 1)
        {
            return RESULT_ERRNO_void(EXC_CATEGORY_NET);
        }
        if (bind(sk, (struct sockaddr *)&sa_in, sizeof(sa_in)) < 0)
        {
            return RESULT_ERRNO_void(EXC_CATEGORY_NET);
        }
        return RESULT_ok_void();
    }
    else if (inet_version == NET_INET_VERSION_6)
    {
        struct sockaddr_in6 sa_in6;

        memset(&sa_in6, 0, sizeof(sa_in6));
        sa_in6.sin6_family = AF_INET6;
        sa_in6.sin6_port = htons(port);
        if (inet_pton(AF_INET6, addr, &sa_in6.sin6_addr) != 1)
        {
            return RESULT_ERRNO_void(EXC_CATEGORY_NET);
        }
        if (bind(sk, (struct sockaddr *)&sa_in6, sizeof(sa_in6)) < 0)
        {
            return RESULT_ERRNO_void(EXC_CATEGORY_NET);
        }
        return RESULT_ok_void();
    }
    else
        abort();

    return RESULT_ok_void(); /* Never reached. */
}

/* Receive a DNS query from socket SK.
   Fills OUT_HEADER and OUT_QUESTION with the parsed data. */
RESULT_void DNS_recv_query(NET_Socket sk, struct DNS_Header *out_header, struct DNS_Question *out_question,
                           struct NET_InetAddr *out_src_addr, uint16_t *out_src_port)
{
    RESULT_void ret;
    uint8_t buf[512];
    UTIL_SignedSize recv_len;
    RESULT_UTIL_SignedSize recv_len_res;
    size_t offset;

    if (out_header == NULL || out_question == NULL || out_src_addr == NULL || out_src_port == NULL)
        abort();

    ret = RESULT_ok_void();
    recv_len = 0;

    recv_len_res = NET_recvfrom(sk, buf, sizeof(buf), 0, out_src_addr, out_src_port);
    if (!recv_len_res.has_value)
    {
        return RESULT_error_void_struct(recv_len_res.result.error);
    }
    recv_len = recv_len_res.result.value;
    if ((size_t)recv_len < sizeof(struct DNS_Header))
        return RESULT_error_void(EFN_TRY_AGAIN, EXC_CATEGORY_FN);

    /* Parse DNS header. */
    DNS_fill_header(out_header, buf, (size_t)recv_len);
    offset = sizeof(struct DNS_Header);

    if (out_header->questions < 1)
    {
        /* No questions present, ignore. */
        return RESULT_error_void(EFN_TRY_AGAIN, EXC_CATEGORY_FN);
    }

    /* Parse first question only. */
    ret = DNS_parse_question(buf, (size_t)recv_len, offset, out_question);
    if (!ret.has_value)
        return ret;

    return ret;
}

/* (Ineternal) Reply to a DNS query for an IPv4 address. */
static RESULT_void DNS__reply_query4(NET_Socket sk, const struct NET_InetAddr *src_addr,
                                     const struct DNS_Header *header, const struct DNS_Question *question,
                                     const struct NET_InetAddr *reply_addr, uint16_t reply_port)
{
    RESULT_void ret;
    DNS_Inet4Addr ip_addr;
    struct DNS_ResourceRecord rr;
    uint8_t *out_buf;
    size_t out_buf_len;

    ret = RESULT_ok_void();
    out_buf = NULL;
    ip_addr[0] = src_addr->addr.inet4[0];
    ip_addr[1] = src_addr->addr.inet4[1];
    ip_addr[2] = src_addr->addr.inet4[2];
    ip_addr[3] = src_addr->addr.inet4[3];

    ret = DNS_make_A_record((UTIL_ConstString)question->name, ip_addr, &rr);
    if (!ret.has_value)
        goto EXIT;

    out_buf_len = DNS_get_response_len(question, &rr, 1);
    if (out_buf_len > 512)
    {
        ret = RESULT_error_void(EFN_MAX_CAPACITY_EXCEEDED, EXC_CATEGORY_FN);
        goto EXIT;
    }

    out_buf = (uint8_t *)malloc(out_buf_len);
    if (out_buf == NULL)
    {
        ret = RESULT_ERRNO_void(EXC_CATEGORY_LIBC);
        goto EXIT;
    }

    ret = DNS_make_response(header, (struct DNS_Question *)question, &rr, 1, out_buf, out_buf_len, &out_buf_len);
    if (!ret.has_value)
    {
        goto EXIT;
    }

    /* Send response back to source address. */
    ret = NET_sendto(sk, out_buf, out_buf_len, 0, reply_addr, reply_port);
    if (!ret.has_value)
    {
        goto EXIT;
    }

EXIT:
    DNS_free_resource_record(&rr);
    free(out_buf);
    return ret;
}

/* (Ineternal) Reply to a DNS query for an IPv6 address. */
static RESULT_void DNS__reply_query6(NET_Socket sk, const struct NET_InetAddr *src_addr,
                                     const struct DNS_Header *header, const struct DNS_Question *question,
                                     const struct NET_InetAddr *reply_addr, uint16_t reply_port)
{
    RESULT_void ret;
    DNS_Inet6Addr ip_addr;
    struct DNS_ResourceRecord rr;
    uint8_t *out_buf;
    size_t out_buf_len;

    ret = RESULT_ok_void();
    out_buf = NULL;
    memcpy(ip_addr, src_addr->addr.inet6, sizeof(DNS_Inet6Addr));

    ret = DNS_make_AAAA_record((UTIL_ConstString)question->name, ip_addr, &rr);
    if (!ret.has_value)
        goto EXIT;

    out_buf_len = DNS_get_response_len(question, &rr, 1);
    if (out_buf_len > 512)
    {
        ret = RESULT_error_void(EFN_MAX_CAPACITY_EXCEEDED, EXC_CATEGORY_FN);
        goto EXIT;
    }

    out_buf = (uint8_t *)malloc(out_buf_len);
    if (out_buf == NULL)
    {
        ret = RESULT_ERRNO_void(EXC_CATEGORY_LIBC);
        goto EXIT;
    }

    ret = DNS_make_response(header, (struct DNS_Question *)question, &rr, 1, out_buf, out_buf_len, &out_buf_len);
    if (!ret.has_value)
    {
        goto EXIT;
    }

    /* Send response back to source address. */
    ret = NET_sendto(sk, out_buf, out_buf_len, 0, reply_addr, reply_port);
    if (!ret.has_value)
    {
        goto EXIT;
    }

EXIT:
    DNS_free_resource_record(&rr);
    free(out_buf);
    return ret;
}

/* Reply to a DNS query with a A/AAAA record for RECORD_ADDR with HEADER and QUESTION.
   Sends the reply to REPLY_ADDR:REPLY_PORT. */
RESULT_void DNS_reply_query(NET_Socket sk, const struct NET_InetAddr *record_addr, const struct DNS_Header *header,
                            const struct DNS_Question *question, const struct NET_InetAddr *reply_addr,
                            uint16_t reply_port)
{
    if (record_addr == NULL || header == NULL || question == NULL || reply_addr == NULL)
        abort();

    if (record_addr->version == NET_INET_VERSION_4)
    {
        return DNS__reply_query4(sk, record_addr, header, question, reply_addr, reply_port);
    }
    else if (record_addr->version == NET_INET_VERSION_6)
    {
        return DNS__reply_query6(sk, record_addr, header, question, reply_addr, reply_port);
    }
    else
        abort();

    return RESULT_ok_void();
}

/* Reply to a DNS query with NXDOMAIN error.
   Sends the reply to REPLY_ADDR:REPLY_PORT. */
RESULT_void DNS_reply_query_notfound(NET_Socket sk, const struct DNS_Header *header,
                                     const struct DNS_Question *question, const struct NET_InetAddr *reply_addr,
                                     uint16_t reply_port)
{
    RESULT_void ret;
    uint8_t *out_buf;
    size_t out_buf_len;
    size_t pos;
    struct DNS_Header resp;
    uint16_t net_id, net_flags, net_questions, net_answer, net_authority, net_additional;
    size_t name_encoded_len;
    uint16_t net_qtype, net_qclass;

    ret = RESULT_ok_void();
    out_buf = NULL;
    pos = 0;

    if (header == NULL || question == NULL || reply_addr == NULL || reply_addr == NULL)
        abort();

    /* Header(12) + Question(name + 4) */
    out_buf_len = sizeof(struct DNS_Header) + DNS_dns_name_encoded_len((UTIL_ConstString)question->name) + 4;
    if (out_buf_len > 512)
    {
        ret = RESULT_error_void(EFN_MAX_CAPACITY_EXCEEDED, EXC_CATEGORY_FN);
        goto EXIT;
    }

    out_buf = (uint8_t *)malloc(out_buf_len);
    if (!out_buf)
    {
        ret = RESULT_ERRNO_void(EXC_CATEGORY_LIBC);
        goto EXIT;
    }

    /* Fill response header. */
    memset(&resp, 0, sizeof(resp));
    resp.id = header->id;
    /* QR=1(response), OPCODE=0, AA=1, TC=0, RD=0, RA=0, Z=0, RCODE=3(NXDOMAIN)
       See RFC1035: QR(15) OPCODE(14-11) AA(10) TC(9) RD(8) RA(7) Z(6-4) RCODE(3-0) */
    resp.flags = (1 << 15) | /* QR=1 */
                 (1 << 10);  /* AA=1 */

    resp.flags |= 3; /* RCODE=3 (NXDOMAIN) */
    resp.questions = 1;
    resp.answer = 0;
    resp.authority = 0;
    resp.additional = 0;

    /* Write response header in network byte order. */
    net_id = htons(resp.id);
    net_flags = htons(resp.flags);
    net_questions = htons(resp.questions);
    net_answer = htons(resp.answer);
    net_authority = htons(resp.authority);
    net_additional = htons(resp.additional);

    memcpy(out_buf + pos, &net_id, 2);
    memcpy(out_buf + pos + 2, &net_flags, 2);
    memcpy(out_buf + pos + 4, &net_questions, 2);
    memcpy(out_buf + pos + 6, &net_answer, 2);
    memcpy(out_buf + pos + 8, &net_authority, 2);
    memcpy(out_buf + pos + 10, &net_additional, 2);
    pos += sizeof(struct DNS_Header);

    /* Write Question: name + qtype/qclass */
    name_encoded_len = 0;
    ret = DNS_encode_name((UTIL_ConstString)question->name, out_buf + pos, out_buf_len - pos, &name_encoded_len);
    if (!ret.has_value)
        goto EXIT;
    pos += name_encoded_len;

    if (pos + 4 > out_buf_len) /* qtype + qclass */
    {
        ret = RESULT_error_void(EFN_MAX_CAPACITY_EXCEEDED, EXC_CATEGORY_FN);
        goto EXIT;
    }
    net_qtype = htons(question->qtype);
    net_qclass = htons(question->qclass);
    memcpy(out_buf + pos, &net_qtype, 2);
    memcpy(out_buf + pos + 2, &net_qclass, 2);
    pos += 4;

    /* Send response back to source address. */
    ret = NET_sendto(sk, out_buf, pos, 0, reply_addr, reply_port);
    if (!ret.has_value)
        goto EXIT;

EXIT:
    free(out_buf);
    return ret;
}

#if 0
#pragma endregion

#pragma region user
#endif

/* Check if NAME is a valid hostname for FastNetworking. */
UTIL_Bool USER_name_is_valid(UTIL_ConstString name, size_t len)
{
    size_t i;

    if (name == NULL)
        return UTIL_FALSE;

    /* 1. Check if name is empty. */
    if (len == 0)
        return UTIL_FALSE;

    /* 3. Check if name contains space. */
    for (i = 0; i < len; i++)
    {
        if (isspace((int)(unsigned char)name[i]))
            return UTIL_FALSE;
    }

    /* 4. Only alphanumeric characters, '-' and '_' are allowed. */
    for (i = 0; i < len; i++)
    {
        if (!isalnum((int)(unsigned char)name[i]) && name[i] != '-' && name[i] != '_')
            return UTIL_FALSE;
    }

    return UTIL_TRUE;
}

/* Get system hostname, store in *PHOSTNAME. Notice that *PHOSTNAME is malloc'd. */
RESULT_void USER_get_hostname(UTIL_StringBuffer *phostname)
{
    RESULT_void ret;
    size_t bufsize;                 /* Size of temp buffer for hostname. */
    size_t hn_len;                  /* Length of hostname. */
    UTIL_StringBuffer hostname_buf; /* Temp buffer for hostname. */
#if defined(_SC_HOST_NAME_MAX)
    long _hn_max;
    _hn_max = sysconf(_SC_HOST_NAME_MAX);
    bufsize = (_hn_max > 0) ? (size_t)_hn_max + 1 : 256;
#elif defined(NI_MAXHOST)
    bufsize = NI_MAXHOST;
#else
    bufsize = 256; /* Fallback buffer size. */
#endif

    ret = RESULT_ok_void();
    hn_len = 0;
    hostname_buf = NULL;

    if (phostname == NULL)
        abort();

    *phostname = NULL;

    hostname_buf = (UTIL_StringBuffer)malloc(bufsize);
    if (hostname_buf == NULL)
    {
        ret = RESULT_ERRNO_void(EXC_CATEGORY_LIBC);
        goto EXIT;
    }

    if (gethostname(hostname_buf, bufsize) != 0)
    {
        ret = RESULT_ERRNO_void(EXC_CATEGORY_NET);
        goto EXIT;
    }
    hostname_buf[bufsize - 1] = '\0';

    hn_len = strlen(hostname_buf);

    /* Check if hostname is valid. */
    if (!USER_name_is_valid(hostname_buf, hn_len))
    {
        ret = RESULT_error_void(EFN_INVALID_HOSTNAME, EXC_CATEGORY_FN);
        goto EXIT;
    }

    *phostname = (UTIL_StringBuffer)malloc(hn_len + 1);
    if (*phostname == NULL)
    {
        ret = RESULT_ERRNO_void(EXC_CATEGORY_LIBC);
        goto EXIT;
    }
    memcpy(*phostname, hostname_buf, hn_len + 1);

EXIT:
    free(hostname_buf);
    if (!ret.has_value)
    {
        free(*phostname);
        *phostname = NULL;
    }

    return ret;
}

#if 0
#pragma endregion

#pragma region multicast
#endif

/* Multicast packet magic number. */
#define MULTICAST_PACKET_MAGIC (uint16_t)(0x1145)

/* Multicast packet type: boardcast. */
#define MULTICAST_TYPE_BOARDCAST (uint16_t)(1)
/* Multicast packet type: request. */
#define MULTICAST_TYPE_REQUEST (uint16_t)(3)
/* Multicast packet type: offline. */
#define MULTICAST_TYPE_OFFLINE (uint16_t)(4)
/* Multicast packet type: refuse. */
#define MULTICAST_TYPE_REFUSE (uint16_t)(5)

/* Multicast hostnames list. */
struct UTIL_StrArray g_MULTICAST_hostnames;

/* Deinitialize multicast module. */
void MULTICAST_deinitialize(void)
{
    UTIL_str_array_free(&g_MULTICAST_hostnames);
    memset(&g_MULTICAST_hostnames, 0, sizeof(g_MULTICAST_hostnames));
}

/* Initialize multicast module. If error occurs, the program must be terminated. */
RESULT_void MULTICAST_initialize(void)
{
    RESULT_void ret;

    ret = RESULT_ok_void();

    /* Allocate memory for hostnames. */
    ret = UTIL_str_array_init(&g_MULTICAST_hostnames, 16);
    if (!ret.has_value)
    {
        LOGGING_perror("UTIL_str_array_init", ret.error);
        goto ERROR;
    }

    /* Get system hostname. */
    ret = USER_get_hostname(&g_MULTICAST_hostnames.arr[0]);
    if (!ret.has_value)
    {
        LOGGING_perror("USER_get_hostname", ret.error);
        goto ERROR;
    }
    g_MULTICAST_hostnames.size = 1;

    return ret;

ERROR:
    MULTICAST_deinitialize();
    return ret;
}

/* Initialize multicast socket for IPv4. */
RESULT_void MULTICAST_inet4_init_multicast_socket(NET_Socket sk, UTIL_ConstString group)
{
    RESULT_void ret;
    struct ip_mreq mreq;
    struct sockaddr_in local_addr;
    struct ifaddrs *ifaddr, *ifa;
    struct in_addr multi_addr;
    UTIL_Bool joined;

    ret = RESULT_ok_void();
    ifaddr = NULL;
    ifa = NULL;
    joined = UTIL_FALSE;

    /* Bind the socket. */
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(MULTICAST_PORT);
    if (bind(sk, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0)
    {
        ret = RESULT_ERRNO_void(EXC_CATEGORY_NET);
        goto EXIT;
    }

    /* Load the multicast group address. */
    if (inet_pton(AF_INET, group, &multi_addr) != 1)
    {
        ret = RESULT_ERRNO_void(EXC_CATEGORY_NET);
        goto EXIT;
    }

    /* Iterate all net interfaces and join the multicast group on each. */
    if (getifaddrs(&ifaddr) == -1)
    {
        ret = RESULT_ERRNO_void(EXC_CATEGORY_NET);
        goto EXIT;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;
        if (ifa->ifa_addr->sa_family != AF_INET)
            continue;
        if (!(ifa->ifa_flags & IFF_UP))
            continue;
        if (!(ifa->ifa_flags & IFF_MULTICAST))
            continue;
        if (((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr == htonl(INADDR_LOOPBACK) ||
            ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr == htonl(INADDR_ANY))
            continue;

        /* Join the multicast group on this interface. */
        memcpy(&mreq.imr_multiaddr, &multi_addr, sizeof(mreq.imr_multiaddr));
        mreq.imr_interface.s_addr = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
        if (setsockopt(sk, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
        {
            ret = RESULT_ERRNO_void(EXC_CATEGORY_NET);
            /* Don't abort on error. */
            LOGGING_warn("Error joining multicast group on interface %s: %s", ifa->ifa_name,
                         EXC_strerror(EXC_get_last_error(EXC_CATEGORY_NET)));
            LOGGING_pwarn("setsockopt(IP_ADD_MEMBERSHIP)", EXC_get_last_error(EXC_CATEGORY_NET));
        }
        else
        {
            ret = RESULT_ok_void();
            LOGGING_debug("Joined IPv4 multicast group on interface: %s", ifa->ifa_name);
            joined = UTIL_TRUE;
        }
    }

    if (!joined)
    {
        /* No interfaces joined the multicast group. */
        ret = RESULT_error_void(EFN_NO_MULTICAST_JOINED, EXC_CATEGORY_FN);
        goto EXIT;
    }

EXIT:
    if (ifaddr != NULL)
        freeifaddrs(ifaddr);
    return ret;
}

/* Initialize multicast socket for IPv6. */
RESULT_void MULTICAST_inet6_init_multicast_socket(NET_Socket sk, UTIL_ConstString group)
{
    RESULT_void ret;
    struct ipv6_mreq mreq6;
    struct sockaddr_in6 local_addr6;
    struct ifaddrs *ifaddr, *ifa;
    struct in6_addr multi_addr6;
    UTIL_Bool joined;

    ret = RESULT_ok_void();
    ifaddr = NULL;
    ifa = NULL;
    joined = UTIL_FALSE;

    /* Bind the socket to [::]:MULTICAST_PORT. */
    memset(&local_addr6, 0, sizeof(local_addr6));
    local_addr6.sin6_family = AF_INET6;
    local_addr6.sin6_addr = in6addr_any; /* :: (any) */
    local_addr6.sin6_port = htons(MULTICAST_PORT);
    local_addr6.sin6_scope_id = 0;

    if (bind(sk, (struct sockaddr *)&local_addr6, sizeof(local_addr6)) < 0)
    {
        ret = RESULT_ERRNO_void(EXC_CATEGORY_NET);
        goto EXIT;
    }

    /* Load the multicast group address. */
    if (inet_pton(AF_INET6, group, &multi_addr6) != 1)
    {
        ret = RESULT_ERRNO_void(EXC_CATEGORY_NET);
        goto EXIT;
    }

    /* Iterate all net interfaces and join the multicast group on each. */
    if (getifaddrs(&ifaddr) == -1)
    {
        ret = RESULT_ERRNO_void(EXC_CATEGORY_NET);
        goto EXIT;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;
        if (ifa->ifa_addr->sa_family != AF_INET6)
            continue;
        if (!(ifa->ifa_flags & IFF_UP))
            continue;
        if (!(ifa->ifa_flags & IFF_MULTICAST))
            continue;
        if (IN6_IS_ADDR_LOOPBACK(&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr) ||
            IN6_IS_ADDR_UNSPECIFIED(&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr))
            continue;

        /* Prepare IPv6 multicast membership request. */
        memcpy(&mreq6.ipv6mr_multiaddr, &multi_addr6, sizeof(mreq6.ipv6mr_multiaddr));

        /* Use interface index instead of IP address for IPv6. */
        mreq6.ipv6mr_interface = if_nametoindex(ifa->ifa_name);
        if (mreq6.ipv6mr_interface == 0)
        {
            ret = RESULT_ERRNO_void(EXC_CATEGORY_NET);
            LOGGING_warn("Error joining multicast group on interface %s: %s", ifa->ifa_name,
                         EXC_strerror(EXC_get_last_error(EXC_CATEGORY_NET)));
            LOGGING_pwarn("if_nametoindex (IPv6)", EXC_get_last_error(EXC_CATEGORY_NET));
            continue;
        }

        if (setsockopt(sk, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq6, sizeof(mreq6)) < 0)
        {
            /* Don't abort on error. */
            ret = RESULT_ERRNO_void(EXC_CATEGORY_NET);
            LOGGING_warn("Error joining multicast group on interface %s: %s", ifa->ifa_name,
                         EXC_strerror(EXC_get_last_error(EXC_CATEGORY_NET)));
            LOGGING_pwarn("setsockopt(IPV6_JOIN_GROUP)", EXC_get_last_error(EXC_CATEGORY_NET));
        }
        else
        {
            ret = RESULT_ok_void();
            LOGGING_debug("Joined IPv6 multicast group on interface: %s", ifa->ifa_name);
            joined = UTIL_TRUE;
        }
    }

    if (!joined)
    {
        /* No interfaces joined the multicast group. */
        ret = RESULT_error_void(EFN_NO_MULTICAST_JOINED, EXC_CATEGORY_FN);
        goto EXIT;
    }

EXIT:
    if (ifaddr != NULL)
        freeifaddrs(ifaddr);
    return ret;
}

/* Multicast boardcast packet header structure. */
struct MULTICAST_BoardcastHeader
{
    /* Multicast packet magic number, must be MULTICAST_PACKET_MAGIC. */
    uint16_t magic;
    /* Multicast protocol version. */
    uint16_t version;
    /* Multicast packet type. */
    uint16_t type;
    /* Multicast packet length. */
    uint16_t length;
};

/* Size of packed multicast boardcast header. */
#define MULTICAST_BOARDCAST_HEADER_SIZE sizeof(uint64_t)

/* Pack a MULTICAST_BoardcastHeader structure into a 64-bit integer. */
uint64_t MULTICAST_pack_boardcast_header(struct MULTICAST_BoardcastHeader header)
{
    uint8_t b[8];
    uint64_t packed;
    uint64_t m = (uint64_t)htons(header.magic);
    uint64_t v = (uint64_t)htons(header.version);
    uint64_t t = (uint64_t)htons(header.type);
    uint64_t l = (uint64_t)htons(header.length);

    memcpy(b + 0, &m, 2);
    memcpy(b + 2, &v, 2);
    memcpy(b + 4, &t, 2);
    memcpy(b + 6, &l, 2);

    packed = 0;
    memcpy(&packed, b, sizeof(packed));
    return packed;
}

/* Unpack a packed multicast boardcast header into a MULTICAST_BoardcastHeader structure. */
void MULTICAST_unpack_boardcast_header(uint64_t packed, struct MULTICAST_BoardcastHeader *header)
{
    uint8_t b[8];
    uint16_t m, v, t, l;

    memcpy(b, &packed, sizeof(b));
    memcpy(&m, b + 0, 2);
    memcpy(&v, b + 2, 2);
    memcpy(&t, b + 4, 2);
    memcpy(&l, b + 6, 2);

    header->magic = ntohs(m);
    header->version = ntohs(v);
    header->type = ntohs(t);
    header->length = ntohs(l);
}

/* Send a multicast packet with type TYPE and data DATA of length DATA_LEN. */
RESULT_void MULTICAST_send_packet(NET_Socket sk, uint16_t type, UTIL_ConstString data, size_t data_len,
                                  enum NET_InetVersion inet_version)
{
    RESULT_void ret;
    uint8_t *buf;
    struct MULTICAST_BoardcastHeader header;
    uint64_t packed_header;
    struct sockaddr_storage addr;
    socklen_t addr_len;
    UTIL_SignedSize sent;

    ret = RESULT_ok_void();
    buf = NULL;
    memset(&addr, 0, sizeof(addr));

    if (data == NULL || data_len == 0)
        abort();

    if (data_len > 65535)
    {
        ret = RESULT_error_void(EFN_MAX_CAPACITY_EXCEEDED, EXC_CATEGORY_FN);
        goto EXIT;
    }

    header.magic = MULTICAST_PACKET_MAGIC;
    header.version = MULTICAST_VERSION;
    header.length = (uint16_t)(data_len);
    header.type = type;
    packed_header = MULTICAST_pack_boardcast_header(header);

    buf = (uint8_t *)malloc(sizeof(packed_header) + data_len);
    if (buf == NULL)
    {
        ret = RESULT_ERRNO_void(EXC_CATEGORY_LIBC);
        goto EXIT;
    }

    memcpy(buf, &packed_header, sizeof(packed_header));
    memcpy(buf + sizeof(packed_header), data, data_len);

    if (inet_version == NET_INET_VERSION_4)
    {
        struct sockaddr_in *addr_in;

        addr_in = (struct sockaddr_in *)&addr;
        addr_in->sin_family = AF_INET;
        addr_in->sin_port = htons(MULTICAST_PORT);
        if (inet_pton(AF_INET, MULTICAST_ADDR, &addr_in->sin_addr) != 1)
        {
            ret = RESULT_ERRNO_void(EXC_CATEGORY_NET);
            goto EXIT;
        }
        addr_len = sizeof(struct sockaddr_in);
    }
    else if (inet_version == NET_INET_VERSION_6)
    {
        struct sockaddr_in6 *addr_in6;

        addr_in6 = (struct sockaddr_in6 *)&addr;
        addr_in6->sin6_family = AF_INET6;
        addr_in6->sin6_port = htons(MULTICAST_PORT);
        if (inet_pton(AF_INET6, MULTICAST_ADDR6, &addr_in6->sin6_addr) != 1)
        {
            ret = RESULT_ERRNO_void(EXC_CATEGORY_NET);
            goto EXIT;
        }
        addr_in6->sin6_scope_id = 0;
        addr_len = sizeof(struct sockaddr_in6);
    }
    else
        abort();

    sent = sendto(sk, buf, sizeof(packed_header) + data_len, 0, (struct sockaddr *)&addr, addr_len);
    if (sent < 0)
    {
        if (NET_kern_buffer_is_full(EXC_get_last_error(EXC_CATEGORY_NET)))
        {
            LOGGING_warn("Kernel buffer full, multicast packet dropped.");
            ret = RESULT_error_void(EFN_TRY_AGAIN, EXC_CATEGORY_FN);
            goto EXIT;
        }
        ret = RESULT_ERRNO_void(EXC_CATEGORY_NET);
        goto EXIT;
    }

EXIT:
    free(buf);
    return ret;
}

/* Send a boardcast packet with hostname HOSTNAME of length LENGTH. */
RESULT_void MULTICAST_send_boardcast(NET_Socket sk, UTIL_ConstString hostname, size_t length,
                                     enum NET_InetVersion inet_version)
{
    return MULTICAST_send_packet(sk, MULTICAST_TYPE_BOARDCAST, hostname, length, inet_version);
}

/* Send a request packet with hostname HOSTNAME of length LENGTH. */
RESULT_void MULTICAST_send_request(NET_Socket sk, UTIL_ConstString hostname, size_t length,
                                   enum NET_InetVersion inet_version)
{
    return MULTICAST_send_packet(sk, MULTICAST_TYPE_REQUEST, hostname, length, inet_version);
}

/* Send an offline notice packet with hostname HOSTNAME of length LENGTH. */
RESULT_void MULTICAST_send_offline(NET_Socket sk, UTIL_ConstString hostname, size_t length,
                                   enum NET_InetVersion inet_version)
{
    return MULTICAST_send_packet(sk, MULTICAST_TYPE_OFFLINE, hostname, length, inet_version);
}

/* Send a refuse packet with hostname HOSTNAME of length LENGTH. */
RESULT_void MULTICAST_send_refuse(NET_Socket sk, UTIL_ConstString hostname, size_t length,
                                  enum NET_InetVersion inet_version)
{
    return MULTICAST_send_packet(sk, MULTICAST_TYPE_REFUSE, hostname, length, inet_version);
}

/* Boardcast all hostnames in g_MULTICAST_hostnames via socket SK. */
RESULT_void MULTICAST_boardcast_self(NET_Socket sk, enum NET_InetVersion inet_version)
{
    RESULT_void ret;
    size_t i;

    ret = RESULT_ok_void();
    i = 0;

    while (i < g_MULTICAST_hostnames.size)
    {
        LOGGING_debug("Boardcasting hostname: %s", g_MULTICAST_hostnames.arr[i]);
        ret = MULTICAST_send_boardcast(sk, g_MULTICAST_hostnames.arr[i], strlen(g_MULTICAST_hostnames.arr[i]),
                                       inet_version);
        if (!ret.has_value)
        {
            goto EXIT;
        }
        i++;
    }

EXIT:
    return ret;
}

/* Boardcast all hostnames in g_MULTICAST_hostnames via socket SK as offline notice. */
RESULT_void MULTICAST_offline_self(NET_Socket sk, enum NET_InetVersion inet_version)
{
    RESULT_void ret;
    size_t i;

    ret = RESULT_ok_void();
    i = 0;

    while (i < g_MULTICAST_hostnames.size)
    {
        ret = MULTICAST_send_offline(sk, g_MULTICAST_hostnames.arr[i], strlen(g_MULTICAST_hostnames.arr[i]),
                                     inet_version);
        if (!ret.has_value)
        {
            goto EXIT;
        }
        i++;
    }

EXIT:
    return ret;
}

/* Receive a boardcast packet from socket SK into DATA of length LEN. The received hostname will be stored in
   OUT_HEADER. The source address of the packet will be stored in OUT_SRC_ADDR. */
RESULT_void MULTICAST_recv_boardcast(NET_Socket sk, UTIL_StringBuffer data, size_t *len,
                                     struct MULTICAST_BoardcastHeader *out_header, struct NET_InetAddr *out_src_addr)
{
    RESULT_void ret;
    uint64_t packed_header;
    UTIL_SignedSize recv_len;
    uint8_t buf[MULTICAST_BOARDCAST_HEADER_SIZE + MULTICAST_MAX_DOMAIN_LEN];
    struct sockaddr_storage addr;
    socklen_t addr_len;

    ret = RESULT_ok_void();
    addr_len = sizeof(addr);

    if (data == NULL || len == NULL || out_header == NULL || out_src_addr == NULL)
        abort();

    recv_len = recvfrom(sk, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &addr_len);
    if (recv_len == 0)
    {
        LOGGING_warn("Received zero-length multicast packet, ignoring.");
        ret = RESULT_error_void(EFN_TRY_AGAIN, EXC_CATEGORY_FN);
        goto EXIT;
    }
    if (recv_len < 0)
    {
        /* Convert EAGAIN. */
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            ret = RESULT_error_void(EFN_TRY_AGAIN, EXC_CATEGORY_FN);
            goto EXIT;
        }
        ret = RESULT_ERRNO_void(EXC_CATEGORY_NET);
        goto EXIT;
    }
    if ((size_t)recv_len < sizeof(packed_header))
    {
        LOGGING_warn("Received multicast packet too small (%d < %d), ignoring.", recv_len, sizeof(packed_header));
        ret = RESULT_error_void(EFN_TRY_AGAIN, EXC_CATEGORY_FN);
        goto EXIT;
    }
    /* Now packed header is at least sizeof(packed_header), so we can memcpy it safely. */
    memcpy(&packed_header, buf, sizeof(packed_header));
    MULTICAST_unpack_boardcast_header(packed_header, out_header);

    /* Check header is valid. */
    if (out_header->magic != MULTICAST_PACKET_MAGIC)
    {
        /* Ignore other versions packet. */
        ret = RESULT_error_void(EFN_TRY_AGAIN, EXC_CATEGORY_FN);
        goto EXIT;
    }
    if (out_header->version != MULTICAST_VERSION) /* Version */
    {
        /* Ignore other versions packet. */
        ret = RESULT_error_void(EFN_TRY_AGAIN, EXC_CATEGORY_FN);
        goto EXIT;
    }
    if ((size_t)recv_len < sizeof(packed_header) + out_header->length || out_header->length == 0 ||
        out_header->length >= MULTICAST_MAX_DOMAIN_LEN)
    {
        ret = RESULT_error_void(EFN_MAX_CAPACITY_EXCEEDED, EXC_CATEGORY_FN);
        goto EXIT;
    }
    /* Now hostname is at least header.length, so we can memcpy it safely. */
    memcpy(data, buf + sizeof(packed_header), out_header->length);
    *len = out_header->length;
    data[out_header->length] = '\0'; /* Null-terminate the hostname string. */

    /* Last, load source address. */
    NET_load_addr(&addr, out_src_addr);
EXIT:
    return ret;
}

/* Check if HOSTNAME is in g_MULTICAST_hostnames. */
UTIL_Bool MULTICAST_boardcast_is_for_me(UTIL_ConstString hostname)
{
    return UTIL_str_in_array(hostname, &g_MULTICAST_hostnames);
}

#if 0
#pragma endregion

#pragma region config
#endif

/* Configuration entry types. */
enum CONFIG_ConfigEntryType
{
    CONFIG_CONFIG_ENTRY_TYPE_INVALID = -1,

    /* String type. */
    CONFIG_CONFIG_ENTRY_TYPE_STRING = 1,
    /* Integer type. */
    CONFIG_CONFIG_ENTRY_TYPE_INT = 2
};

/* A configuration entry structure. */
struct CONFIG_ConfigEntry
{
    /* Configuration entry type. */
    enum CONFIG_ConfigEntryType type;
    /* Configuration entry key. */
    UTIL_StringBuffer key;
    /* Configuration entry value. */
    union {
        UTIL_StringBuffer str_val;
        uint64_t int_val;
    } value;
};

/* Free a CONFIG_ConfigEntry structure. */
void CONFIG_free_config_entry(struct CONFIG_ConfigEntry *entry)
{
    if (entry == NULL)
        return;

    free(entry->key);
    if (entry->type == CONFIG_CONFIG_ENTRY_TYPE_STRING)
    {
        free(entry->value.str_val);
    }
    memset(entry, 0, sizeof(*entry));
}

/* Configuration options enumeration. */
enum CONFIG_FastNetworkingOptions
{
    OPT_INVALID = -1,

    OPT_MULTICAST_IPV4_ADDR = 0,
    OPT_MULTICAST_IPV6_ADDR = 1,
    OPT_MULTICAST_PORT = 2,
    OPT_DNS_IPV4_ADDR = 3,
    OPT_DNS_IPV6_ADDR = 4,
    OPT_LOGGING_LEVEL = 5,
    OPT_LOGGING_FORMAT_DEBUG = 6,
    OPT_LOGGING_FORMAT_INFO = 7,
    OPT_LOGGING_FORMAT_WARN = 8,
    OPT_LOGGING_FORMAT_ERROR = 9,
    OPT_MULTICAST_BOARDCAST_INTERVAL_MS = 10,

    OPT_MAX
};

/* (Internal) Global configuration entries array. */
static struct CONFIG_ConfigEntry g_FN_config_entries[(size_t)OPT_MAX];

#define CONFIG__strdup_unwrap(save_to, str)                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        (save_to) = UTIL_strdup((str));                                                                                \
        err = EXC_get_last_error(EXC_CATEGORY_LIBC);                                                                   \
        if ((save_to) == NULL)                                                                                         \
        {                                                                                                              \
            LOGGING_perror("UTIL_strdup.", err);                                                                       \
            goto FAIL;                                                                                                 \
        }                                                                                                              \
    } while (0)

/* Deinitialize configuration entries and free allocated memory. */
void CONFIG_deinitialize(void)
{
    size_t i;

    for (i = 0; i < (size_t)OPT_MAX; i++)
    {
        CONFIG_free_config_entry(&g_FN_config_entries[i]);
    }
    memset(g_FN_config_entries, 0, sizeof(g_FN_config_entries));
}

/* Initialize configuration entries with default values. */
RESULT_void CONFIG_initialize(void)
{
    struct EXC_ErrorCode err;

    err = RESULT_ok_void().error;
    memset(g_FN_config_entries, 0, sizeof(g_FN_config_entries));

    /* Set default values. */
    g_FN_config_entries[OPT_MULTICAST_IPV4_ADDR].type = CONFIG_CONFIG_ENTRY_TYPE_STRING;
    CONFIG__strdup_unwrap(g_FN_config_entries[OPT_MULTICAST_IPV4_ADDR].key, "MulticastIPv4Address");
    CONFIG__strdup_unwrap(g_FN_config_entries[OPT_MULTICAST_IPV4_ADDR].value.str_val, MULTICAST_ADDR);

    g_FN_config_entries[OPT_MULTICAST_IPV6_ADDR].type = CONFIG_CONFIG_ENTRY_TYPE_STRING;
    CONFIG__strdup_unwrap(g_FN_config_entries[OPT_MULTICAST_IPV6_ADDR].key, "MulticastIPv6Address");
    CONFIG__strdup_unwrap(g_FN_config_entries[OPT_MULTICAST_IPV6_ADDR].value.str_val, MULTICAST_ADDR6);

    g_FN_config_entries[OPT_MULTICAST_PORT].type = CONFIG_CONFIG_ENTRY_TYPE_INT;
    CONFIG__strdup_unwrap(g_FN_config_entries[OPT_MULTICAST_PORT].key, "MulticastPort");
    g_FN_config_entries[OPT_MULTICAST_PORT].value.int_val = MULTICAST_PORT;

    g_FN_config_entries[OPT_DNS_IPV4_ADDR].type = CONFIG_CONFIG_ENTRY_TYPE_STRING;
    CONFIG__strdup_unwrap(g_FN_config_entries[OPT_DNS_IPV4_ADDR].key, "DNSServerIPv4Address");
    CONFIG__strdup_unwrap(g_FN_config_entries[OPT_DNS_IPV4_ADDR].value.str_val, DNS_ADDR4);

    g_FN_config_entries[OPT_DNS_IPV6_ADDR].type = CONFIG_CONFIG_ENTRY_TYPE_STRING;
    CONFIG__strdup_unwrap(g_FN_config_entries[OPT_DNS_IPV6_ADDR].key, "DNSServerIPv6Address");
    CONFIG__strdup_unwrap(g_FN_config_entries[OPT_DNS_IPV6_ADDR].value.str_val, DNS_ADDR6);

    g_FN_config_entries[OPT_LOGGING_LEVEL].type = CONFIG_CONFIG_ENTRY_TYPE_INT;
    CONFIG__strdup_unwrap(g_FN_config_entries[OPT_LOGGING_LEVEL].key, "LoggingLevel");
    g_FN_config_entries[OPT_LOGGING_LEVEL].value.int_val = LOG_LEVEL_INFO;

    g_FN_config_entries[OPT_MULTICAST_BOARDCAST_INTERVAL_MS].type = CONFIG_CONFIG_ENTRY_TYPE_INT;
    CONFIG__strdup_unwrap(g_FN_config_entries[OPT_MULTICAST_BOARDCAST_INTERVAL_MS].key, "MulticastBoardcastIntervalMS");
    g_FN_config_entries[OPT_MULTICAST_BOARDCAST_INTERVAL_MS].value.int_val = MULTICAST_BOARDCAST_INTERVAL_MS;

    g_FN_config_entries[OPT_LOGGING_FORMAT_DEBUG].type = CONFIG_CONFIG_ENTRY_TYPE_STRING;
    CONFIG__strdup_unwrap(g_FN_config_entries[OPT_LOGGING_FORMAT_DEBUG].key, "LoggingFormatDebug");
    CONFIG__strdup_unwrap(g_FN_config_entries[OPT_LOGGING_FORMAT_DEBUG].value.str_val, "[%1] %2");

    g_FN_config_entries[OPT_LOGGING_FORMAT_INFO].type = CONFIG_CONFIG_ENTRY_TYPE_STRING;
    CONFIG__strdup_unwrap(g_FN_config_entries[OPT_LOGGING_FORMAT_INFO].key, "LoggingFormatInfo");
    CONFIG__strdup_unwrap(g_FN_config_entries[OPT_LOGGING_FORMAT_INFO].value.str_val, "[%1] %2");

    g_FN_config_entries[OPT_LOGGING_FORMAT_WARN].type = CONFIG_CONFIG_ENTRY_TYPE_STRING;
    CONFIG__strdup_unwrap(g_FN_config_entries[OPT_LOGGING_FORMAT_WARN].key, "LoggingFormatWarn");
    CONFIG__strdup_unwrap(g_FN_config_entries[OPT_LOGGING_FORMAT_WARN].value.str_val, "[%1] %2");

    g_FN_config_entries[OPT_LOGGING_FORMAT_ERROR].type = CONFIG_CONFIG_ENTRY_TYPE_STRING;
    CONFIG__strdup_unwrap(g_FN_config_entries[OPT_LOGGING_FORMAT_ERROR].key, "LoggingFormatError");
    CONFIG__strdup_unwrap(g_FN_config_entries[OPT_LOGGING_FORMAT_ERROR].value.str_val, "[%1] %2");

    return RESULT_ok_void();

FAIL:
    CONFIG_deinitialize();
    return RESULT_error_void_struct(err);
}

/* Set a configuration entry OPT to ENTRY. Notice that this function copy the ENTRY's key and value.
   if you don't want to set the key, set it to NULL. */
RESULT_void CONFIG_set_config(enum CONFIG_FastNetworkingOptions opt, struct CONFIG_ConfigEntry *entry)
{
    UTIL_StringBuffer old_key;

    old_key = NULL;

    if (opt <= OPT_INVALID || opt >= OPT_MAX || entry == NULL)
        abort();

    old_key = g_FN_config_entries[opt].key;
    g_FN_config_entries[opt].key = NULL; /* Don't free the old key because it may be recovered. */
    CONFIG_free_config_entry(&g_FN_config_entries[opt]);
    g_FN_config_entries[opt].type = entry->type;
    if (entry->key)
    {
        g_FN_config_entries[opt].key = UTIL_strdup(entry->key);
        if (g_FN_config_entries[opt].key == NULL)
        {
            return RESULT_ERRNO_void(EXC_CATEGORY_LIBC);
        }
    }
    else
    {
        g_FN_config_entries[opt].key = old_key; /* Recover the old key. */
        old_key = NULL;
    }
    if (entry->type == CONFIG_CONFIG_ENTRY_TYPE_STRING)
    {
        g_FN_config_entries[opt].value.str_val = UTIL_strdup(entry->value.str_val);
        if (g_FN_config_entries[opt].value.str_val == NULL)
        {
            return RESULT_ERRNO_void(EXC_CATEGORY_LIBC);
        }
    }
    else
    {
        g_FN_config_entries[opt].value.int_val = entry->value.int_val;
    }

    return RESULT_ok_void();
}

/* Set a configuration entry from KEY to ENTRY. Notice that this function copy the ENTRY's key and value. */
RESULT_void CONFIG_set_config_from_key(UTIL_ConstString key, struct CONFIG_ConfigEntry *entry)
{
    size_t i;
    RESULT_void ret;

    ret = RESULT_ok_void();

    if (key == NULL || entry == NULL)
        abort();

    for (i = 0; i < (size_t)OPT_MAX; i++)
    {
        if (strcmp(g_FN_config_entries[i].key, key) == 0)
        {
            ret = CONFIG_set_config((enum CONFIG_FastNetworkingOptions)i, entry);
            goto EXIT;
        }
    }

EXIT:
    return ret;
}

static enum CONFIG_ConfigEntryType CONFIG__get_entry_type_from_key(UTIL_ConstString key)
{
    size_t i;

    for (i = 0; i < (size_t)OPT_MAX; i++)
    {
        if (strcmp(g_FN_config_entries[i].key, key) == 0)
        {
            return g_FN_config_entries[i].type;
        }
    }

    return CONFIG_CONFIG_ENTRY_TYPE_INVALID; /* Not found. */
}

static RESULT_void CONFIG__read_until_space(FILE *file, UTIL_StringBuffer out_buf, size_t buf_size)
{
    size_t pos;
    int c;

    pos = 0;
    c = EOF;

    while (pos < buf_size - 1)
    {
        c = fgetc(file);
        if (c == EOF)
        {
            if (ferror(file))
            {
                return RESULT_ERRNO_void(EXC_CATEGORY_LIBC);
            }
            break; /* EOF reached. */
        }
        if (isspace(c))
        {
            break; /* Stop character reached. */
        }
        out_buf[pos++] = (char)c;
    }
    out_buf[pos] = '\0';

    return RESULT_ok_void();
}

static RESULT_uint64_t CONFIG__parse_int_value(FILE *file)
{
    RESULT_void ret;
    char buffer[10];

    memset(buffer, 0, sizeof(buffer));

    ret = CONFIG__read_until_space(file, buffer, sizeof(buffer));
    if (!ret.has_value)
    {
        return RESULT_error_uint64_t_struct(ret.error);
    }

    /* Convert string string to int. */
    return UTIL_str_to_unsigned_base10(buffer);
}

static void CONFIG__strip(UTIL_ConstString from, UTIL_StringBuffer to, size_t to_size)
{
    size_t start;
    size_t i;

    start = 0;
    while (from[start] != '\0' && isspace((unsigned char)from[start]))
    {
        start++;
    }

    for (i = 0; i < to_size - 1 && from[start + i] != '\0'; i++)
    {
        to[i] = from[start + i];
        to[i + 1] = '\0';
    }
}

static RESULT_void CONFIG__parse_config(FILE *file)
{
    RESULT_void ret;
    char key_buf_raw[64];
    char key_buf[64];
    char value_buf_raw[256];
    char value_buf[256];
    UTIL_Bool in_key;

    memset(key_buf_raw, 0, sizeof(key_buf_raw));
    memset(key_buf, 0, sizeof(key_buf));
    memset(value_buf_raw, 0, sizeof(value_buf_raw));
    memset(value_buf, 0, sizeof(value_buf));
    in_key = UTIL_TRUE;

    /* Read a line until newline or EOF. */
    while (!ferror(file) && !feof(file))
    {
        ret = CONFIG__read_until_space(file, in_key ? key_buf_raw : value_buf_raw,
                                       in_key ? sizeof(key_buf_raw) : sizeof(value_buf_raw));
        if (!ret.has_value)
        {
            return ret;
        }

        if (in_key)
        {
            CONFIG__strip(key_buf_raw, key_buf, sizeof(key_buf));
            in_key = UTIL_FALSE;
        }
        else
        {
            CONFIG__strip(value_buf_raw, value_buf, sizeof(value_buf));
            in_key = UTIL_TRUE;
            if (strlen(key_buf) == 0 || key_buf[0] == '#')
            {
                /* Empty key, skip. */
                continue;
            }
            /* Now parse the key-value pair. */
            {
                struct CONFIG_ConfigEntry entry;
                enum CONFIG_ConfigEntryType expected_type;

                expected_type = CONFIG__get_entry_type_from_key(key_buf);
                if (expected_type == CONFIG_CONFIG_ENTRY_TYPE_INVALID)
                {
                    LOGGING_error("Unknown configuration key: %s", key_buf);
                    continue; /* Unknown key, skip. */
                }

                entry.key = key_buf;
                entry.type = expected_type;
                if (expected_type == CONFIG_CONFIG_ENTRY_TYPE_STRING)
                {
                    entry.value.str_val = value_buf;
                }
                else if (expected_type == CONFIG_CONFIG_ENTRY_TYPE_INT)
                {
                    RESULT_uint64_t int_val;

                    int_val = CONFIG__parse_int_value(file);
                    if (!int_val.has_value)
                    {
                        LOGGING_error("Failed to parse integer value for key %s", key_buf);
                        return RESULT_error_void_struct(int_val.result.error);
                    }
                    entry.value.int_val = int_val.result.value;
                }
                else
                    abort();

                ret = CONFIG_set_config_from_key(key_buf, &entry);
                if (!ret.has_value)
                {
                    LOGGING_error("Failed to set configuration entry for key %s", key_buf);
                    return ret;
                }
            }
        }
    }

    return RESULT_ok_void();
}

/* Load configuration entries from a file. */
void CONFIG_load_from_file(UTIL_ConstString filename)
{
    RESULT_void ret;
    FILE *file;

    LOGGING_debug("Loading configuration from file: %s", filename);

    file = fopen(filename, "r");
    if (file == NULL)
    {
        LOGGING_perror("fopen", EXC_get_last_error(EXC_CATEGORY_LIBC));
        return;
    }

    ret = CONFIG__parse_config(file);
    if (!ret.has_value)
    {
        LOGGING_error("Failed to parse configuration file %s", filename);
        LOGGING_perror("CONFIG_load_from_file: CONFIG__parse_config", ret.error);
        goto EXIT;
    }

EXIT:
    if (file != NULL)
        fclose(file);
}

/* Load configuration entries from global, user, and environment variable files. */
void CONFIG_load(void)
{
    if (UTIL_is_file(CONFIG_GLOBAL_CONFIG_FILE_UNIX))
    {
        CONFIG_load_from_file(CONFIG_GLOBAL_CONFIG_FILE_UNIX);
    }

    if (UTIL_is_file(CONFIG_USER_CONFIG_FILE_UNIX))
    {
        CONFIG_load_from_file(CONFIG_USER_CONFIG_FILE_UNIX);
    }

    if (getenv(CONFIG_ENV_CONFIG_FILE) != NULL)
    {
        CONFIG_load_from_file(getenv(CONFIG_ENV_CONFIG_FILE));
    }
}

/* Get a configuration entry as string by KEY. */
RESULT_UTIL_ConstString CONFIG_get_as_str(enum CONFIG_FastNetworkingOptions key)
{
    if ((size_t)key < (size_t)0 || (size_t)key >= (size_t)OPT_MAX)
        abort();

    if (g_FN_config_entries[key].type != CONFIG_CONFIG_ENTRY_TYPE_STRING)
        abort();

    return RESULT_ok_UTIL_ConstString(g_FN_config_entries[key].value.str_val);
}

/* Get a configuration entry as integer by KEY. */
RESULT_uint64_t CONFIG_get_as_int(enum CONFIG_FastNetworkingOptions key)
{
    if ((size_t)key < (size_t)0 || (size_t)key >= (size_t)OPT_MAX)
        abort();

    if (g_FN_config_entries[key].type != CONFIG_CONFIG_ENTRY_TYPE_INT)
        abort();

    return RESULT_ok_uint64_t(g_FN_config_entries[key].value.int_val);
}

/* A configuration entry iterator. */
typedef struct CONFIG_ConfigEntry *CONFIG_ConfigEntryIter;

/* Start the configuration entry iterator. */
#define CONFIG_iter_start(iter) (iter) = &g_FN_config_entries[0]

/* Advance the configuration entry iterator. */
#define CONFIG_iter_next(iter)                                                                                         \
    do                                                                                                                 \
    {                                                                                                                  \
        (iter)++;                                                                                                      \
    } while (0)

/* Check if the configuration entry iterator has reached the end. */
#define CONFIG_iter_is_end(iter) ((size_t)((iter) - &g_FN_config_entries[0]) >= OPT_MAX)

#if 0
#pragma endregion

#pragma region main
#endif

static RESULT_void FN__apply_logging_settings(void)
{
    enum LOGGING_LogLevel log_level;
    RESULT_uint64_t log_level_res;
    RESULT_UTIL_ConstString fmt_res;

    log_level_res = CONFIG_get_as_int(OPT_LOGGING_LEVEL);
    if (!log_level_res.has_value)
    {
        return RESULT_error_void_struct(log_level_res.result.error);
    }
    log_level = (enum LOGGING_LogLevel)log_level_res.result.value;
    if (!LOGGING_is_valid_level(log_level))
    {
        LOGGING_error("Invalid logging level %d", log_level);
        abort();
    }
    LOGGING_set_level(log_level);

    fmt_res = CONFIG_get_as_str(OPT_LOGGING_FORMAT_DEBUG);
    if (!fmt_res.has_value)
    {
        return RESULT_error_void_struct(fmt_res.result.error);
    }
    LOGGING_set_format(LOG_LEVEL_DEBUG, fmt_res.result.value);
    fmt_res = CONFIG_get_as_str(OPT_LOGGING_FORMAT_INFO);
    if (!fmt_res.has_value)
    {
        return RESULT_error_void_struct(fmt_res.result.error);
    }
    LOGGING_set_format(LOG_LEVEL_INFO, fmt_res.result.value);
    fmt_res = CONFIG_get_as_str(OPT_LOGGING_FORMAT_WARN);
    if (!fmt_res.has_value)
    {
        return RESULT_error_void_struct(fmt_res.result.error);
    }
    LOGGING_set_format(LOG_LEVEL_WARN, fmt_res.result.value);
    fmt_res = CONFIG_get_as_str(OPT_LOGGING_FORMAT_ERROR);
    if (!fmt_res.has_value)
    {
        return RESULT_error_void_struct(fmt_res.result.error);
    }
    LOGGING_set_format(LOG_LEVEL_ERROR, fmt_res.result.value);

    return RESULT_ok_void();
}

#if defined(__cplusplus)
/* Convert a string literal to UTIL_StringBuffer. DON'T MODIFY IT! */
#define FN__STRVIEW(str) const_cast<UTIL_StringBuffer>(str)
#else
/* Convert a string literal to UTIL_StringBuffer. DON'T MODIFY IT! */
#define FN__STRVIEW(str) (str)
#endif

/* Initialize Fast Networking module. */
RESULT_void FN_initialize(void)
{
    RESULT_void ret;

    UTIL_initialize();
    LOGGING_initialize();

    ret = NET_initialize();
    if (!ret.has_value)
    {
        return ret;
    }

    ret = MULTICAST_initialize();
    if (!ret.has_value)
    {
        return ret;
    }

    ret = CONFIG_initialize();
    if (!ret.has_value)
    {
        return ret;
    }

    if (LOGGING_logfile_supports_color())
    {
        struct CONFIG_ConfigEntry entry;

        entry.type = CONFIG_CONFIG_ENTRY_TYPE_STRING;
        entry.key = NULL;

        entry.value.str_val = FN__STRVIEW("\x1b[34m%2\x1b[0m");
        CONFIG_set_config(OPT_LOGGING_FORMAT_DEBUG, &entry);

        entry.value.str_val = FN__STRVIEW("%2");
        CONFIG_set_config(OPT_LOGGING_FORMAT_INFO, &entry);

        entry.value.str_val = FN__STRVIEW("\x1b[33m%2\x1b[0m");
        CONFIG_set_config(OPT_LOGGING_FORMAT_WARN, &entry);

        entry.value.str_val = FN__STRVIEW("\x1b[31m%2\x1b[0m");
        CONFIG_set_config(OPT_LOGGING_FORMAT_ERROR, &entry);
    }

    CONFIG_load();

    ret = FN__apply_logging_settings();
    if (!ret.has_value)
    {
        return ret;
    }

    return RESULT_ok_void();
}

/* Status of adding a record. */
typedef enum FN_RecordStatus
{
    /* Record is successfully added. */
    FN_ADD_TO_RECORD_SUCCESS = 0,
    /* Record already exists. */
    FN_ADD_TO_RECORD_ALREADY_EXISTS,
    /* Record is updated. */
    FN_ADD_TO_RECORD_UPDATED
} FN_RecordStatus;

RESULT_GENERIC(FN_RecordStatus)

/* Add a record with hostname HOSTNAME and address ADDR into RECORD_MAP. */
RESULT_FN_RecordStatus FN_add_to_record(struct MAP_Map *record_map, UTIL_ConstString hostname,
                                        const struct NET_InetAddr *addr)
{
    RESULT_FN_RecordStatus ret;
    RESULT_void ret2;
    enum FN_RecordStatus status;
    UTIL_StringBuffer key;
    void *val;
    const void *existing_val;

    ret = RESULT_ok_FN_RecordStatus(FN_ADD_TO_RECORD_SUCCESS);
    status = FN_ADD_TO_RECORD_SUCCESS;
    key = NULL;
    val = NULL;

    if (record_map == NULL || hostname == NULL || addr == NULL)
        abort();

    key = UTIL_strdup(hostname);
    if (key == NULL)
    {
        ret = RESULT_ERRNO(FN_RecordStatus, EXC_CATEGORY_LIBC);
        goto EXIT;
    }
    val = UTIL_memdup(addr, sizeof(struct NET_InetAddr));
    if (val == NULL)
    {
        ret = RESULT_ERRNO(FN_RecordStatus, EXC_CATEGORY_LIBC);
        goto EXIT;
    }

    existing_val = MAP_get(record_map, key);
    if (existing_val != NULL)
    {
        if (memcmp(existing_val, val, sizeof(struct NET_InetAddr)) == 0)
        {
            status = FN_ADD_TO_RECORD_ALREADY_EXISTS;
        }
        else
            status = FN_ADD_TO_RECORD_UPDATED;
    }
    else
        status = FN_ADD_TO_RECORD_SUCCESS;

    ret2 = MAP_set(record_map, key, val);
    if (!ret2.has_value)
    {
        ret = RESULT_error_FN_RecordStatus_struct(ret2.error);
        goto EXIT;
    }

    ret = RESULT_ok_FN_RecordStatus(status);

EXIT:
    if (!ret.has_value) /* Failed to add record. */
    {
        free(key);
        free(val);
    }
    return ret;
}

/* Check if address ADDR is IPv4 or IPv6, then add it to the corresponding record map. */
void FN_add_to_record_auto(struct MAP_Map *record_inet4_map, struct MAP_Map *record_inet6_map,
                           UTIL_ConstString hostname, const struct NET_InetAddr *addr)
{
    FN_RecordStatus add_stat;
    RESULT_FN_RecordStatus add_res;

    if (record_inet4_map == NULL || record_inet6_map == NULL || hostname == NULL || addr == NULL)
        abort();

    if (addr->version == NET_INET_VERSION_4)
    {
        char out_buf[NET_ADDR_REPR_MAX_LENGTH];

        add_res = FN_add_to_record(record_inet4_map, hostname, addr);
        if (!add_res.has_value)
        {
            LOGGING_pwarn("IPv4 FN_add_to_record", add_res.result.error);
            return;
        }

        add_stat = add_res.result.value;

        NET_repr_inet_addr(addr, out_buf, sizeof(out_buf));
        if (add_stat == FN_ADD_TO_RECORD_SUCCESS)
            LOGGING_info("Added IPv4 record: %s -> %s", hostname, out_buf);
        else if (add_stat == FN_ADD_TO_RECORD_UPDATED)
            LOGGING_info("Updated IPv4 record: %s -> %s", hostname, out_buf);
        /* else {} */
    }
    else if (addr->version == NET_INET_VERSION_6)
    {
        char out_buf[NET_ADDR_REPR_MAX_LENGTH];

        add_res = FN_add_to_record(record_inet6_map, hostname, addr);
        if (!add_res.has_value)
        {
            LOGGING_pwarn("IPv6 FN_add_to_record", add_res.result.error);
            return;
        }

        add_stat = add_res.result.value;

        NET_repr_inet_addr(addr, out_buf, sizeof(out_buf));
        if (add_stat == FN_ADD_TO_RECORD_SUCCESS)
            LOGGING_info("Added IPv6 record: %s -> %s", hostname, out_buf);
        else if (add_stat == FN_ADD_TO_RECORD_UPDATED)
            LOGGING_info("Updated IPv6 record: %s -> %s", hostname, out_buf);
    }
    else
        abort();
}

/* Process offline notice. */
void FN_process_offline_notice(struct MAP_Map *record_inet4_map, struct MAP_Map *record_inet6_map,
                               UTIL_ConstString hostname)
{
    if (record_inet4_map == NULL || record_inet6_map == NULL || hostname == NULL)
        abort();

    if (MAP_contains(record_inet4_map, hostname) || MAP_contains(record_inet6_map, hostname))
        LOGGING_info("Hostname went offline: %s", hostname);

    MAP_remove(record_inet4_map, hostname);
    MAP_remove(record_inet6_map, hostname);
}

/* Process boardcast. */
RESULT_void FN_recv_boardcast(NET_Socket sk, struct MAP_Map *record_inet4_map, struct MAP_Map *record_inet6_map)
{
    RESULT_void ret;
    char data_buf[MULTICAST_MAX_DOMAIN_LEN];
    char recv_from_addr_repr[NET_ADDR_REPR_MAX_LENGTH];
    struct MULTICAST_BoardcastHeader header;
    struct NET_InetAddr recv_from_addr;
    size_t length;

    memset(data_buf, 0, sizeof(data_buf));
    memset(recv_from_addr_repr, 0, sizeof(recv_from_addr_repr));
    memset(&header, 0, sizeof(header));
    memset(&recv_from_addr, 0, sizeof(recv_from_addr));
    length = 0;

    ret = MULTICAST_recv_boardcast(sk, data_buf, &length, &header, &recv_from_addr);
    if (!ret.has_value && ret.error.code == EFN_TRY_AGAIN)
    {
        if (!NET_need_try_again(ret.error))
            LOGGING_perror("MULTICAST_recv_boardcast", ret.error);
        return ret;
    }
    LOGGING_debug("Received multicast packet: type=%u, hostname=%s", header.type, data_buf);
    if (!MULTICAST_boardcast_is_for_me(data_buf))
    {
        switch (header.type)
        {
        case MULTICAST_TYPE_BOARDCAST: {
            FN_add_to_record_auto(record_inet4_map, record_inet6_map, data_buf, &recv_from_addr);
            break;
        }
        case MULTICAST_TYPE_OFFLINE: {
            FN_process_offline_notice(record_inet4_map, record_inet6_map, data_buf);
            break;
        }
        default: {
            LOGGING_warn("Ignoring multicast packet with unsupported type %u for myself.", header.type);
            break;
        }
        }
    }
    else if (header.type == MULTICAST_TYPE_REQUEST)
    {
        /* A request that conatains same hostname as me. */
        NET_repr_inet_addr(&recv_from_addr, recv_from_addr_repr, sizeof(recv_from_addr_repr));
        LOGGING_info("Refusing conflicting hostname request '%s' from [%s]", data_buf, recv_from_addr_repr);
        MULTICAST_send_refuse(sk, data_buf, length, recv_from_addr.version);
    }

    return RESULT_ok_void();
}

RESULT_void FN_process_dns_query(NET_Socket sk, struct MAP_Map *record_inet4_map, struct MAP_Map *record_inet6_map)
{
    RESULT_void ret;
    struct DNS_Header header;
    struct DNS_Question question;
    struct NET_InetAddr src_addr;
    uint16_t src_port;
    char src_addr_repr[NET_ADDR_REPR_MAX_LENGTH];
    struct NET_InetAddr *record_A_addr;
    struct NET_InetAddr *record_AAAA_addr;

    memset(&header, 0, sizeof(header));
    memset(&question, 0, sizeof(question));
    memset(&src_addr, 0, sizeof(src_addr));
    memset(src_addr_repr, 0, sizeof(src_addr_repr));
    src_port = 0;
    record_A_addr = NULL;
    record_AAAA_addr = NULL;

    ret = DNS_recv_query(sk, &header, &question, &src_addr, &src_port);
    if (!ret.has_value && ret.error.code == EFN_TRY_AGAIN)
    {
        if (!NET_need_try_again(ret.error))
            LOGGING_perror("DNS_recv_query", ret.error);
        return ret;
    }
    NET_repr_inet_addr(&src_addr, src_addr_repr, sizeof(src_addr_repr));
    LOGGING_debug("Received DNS query: id=%u, qname=%s, qtype=%u, src=[%s]:%u", header.id, question.name,
                  question.qtype, src_addr_repr, src_port);

    /* Find record for hostname. */
    record_A_addr = (struct NET_InetAddr *)MAP_get(record_inet4_map, (UTIL_ConstString)question.name);
    record_AAAA_addr = (struct NET_InetAddr *)MAP_get(record_inet6_map, (UTIL_ConstString)question.name);

    if (record_A_addr == NULL && record_AAAA_addr == NULL)
    {
        LOGGING_warn("No record found for hostname: %s", question.name);
        ret = DNS_reply_query_notfound(sk, &header, &question, &src_addr, src_port);
        if (!ret.has_value)
        {
            LOGGING_perror("DNS_reply_query_notfound", ret.error);
        }
        goto EXIT;
    }
    LOGGING_debug("Found record for hostname: %s", question.name);
    if (question.qtype == DNS_TYPE_A)
    {
        if (record_A_addr != NULL)
        {
            ret = DNS_reply_query(sk, record_A_addr, &header, &question, &src_addr, src_port);
            if (!ret.has_value)
            {
                LOGGING_perror("DNS_reply_query", ret.error);
            }
        }
        else
        {
            LOGGING_warn("No A record found for hostname: %s", question.name);
            ret = DNS_reply_query_notfound(sk, &header, &question, &src_addr, src_port);
            if (!ret.has_value)
            {
                LOGGING_pwarn("DNS_reply_query_notfound", ret.error);
            }
        }
    }
    else if (question.qtype == DNS_TYPE_AAAA)
    {
        if (record_AAAA_addr != NULL)
        {
            ret = DNS_reply_query(sk, record_AAAA_addr, &header, &question, &src_addr, src_port);
            if (!ret.has_value)
            {
                LOGGING_perror("DNS_reply_query", ret.error);
            }
        }
        else
        {
            LOGGING_warn("No AAAA record found for hostname: %s", question.name);
            ret = DNS_reply_query_notfound(sk, &header, &question, &src_addr, src_port);
            if (!ret.has_value)
            {
                LOGGING_pwarn("DNS_reply_query_notfound", ret.error);
            }
        }
    }
    else
    {
        /* Only A, AAAA supported in this example. */
        LOGGING_warn("Unsupported DNS query type %u for hostname: %s", question.qtype, question.name);
    }

EXIT:
    DNS_free_question(&question);
    return ret;
}

/* Global variable to indicate if main thread is running. */
static UTIL_Bool g_FN_mainthread_running = UTIL_TRUE;

/* Signal handler for SIGINT. */
void FN_signal_handler(int signum)
{
    static int FN_forceexit_counter = FN_FORCEEXIT_COUNT;

    if (signum == SIGINT)
    {
        if (FN_forceexit_counter < FN_FORCEEXIT_COUNT)
        {
            if (FN_forceexit_counter <= 0)
            {
                LOGGING_error("Force exiting ...");
                exit(EXIT_FAILURE);
            }
            else
                LOGGING_error("Press Ctrl+C %d more times to force exit.", FN_forceexit_counter);
        }
        else
            LOGGING_info("SIGINT signal received. Exiting ...");
        FN_forceexit_counter--;
    }
    g_FN_mainthread_running = UTIL_FALSE;
}

/* Request to join multicast group for all hostnames in g_MULTICAST_hostnames via sockets SK4 and SK6. */
RESULT_void FN_request_join(NET_Socket sk4, NET_Socket sk6)
{
    RESULT_void ret;
    size_t i;
    int ask4_count;
    int ask6_count;
    struct UTIL_Timer timer;
    char data_buf[MULTICAST_MAX_DOMAIN_LEN];
    char addr_repr_buf[NET_ADDR_REPR_MAX_LENGTH];
    size_t recv_len;
    struct MULTICAST_BoardcastHeader header;
    struct NET_InetAddr out_src_addr;

    ret = RESULT_ok_void();
    i = 0;
    ask4_count = 3; /* Number of times to ask for each hostname. */
    ask6_count = ask4_count;
    UTIL_timer_start(&timer);
    memset(data_buf, 0, sizeof(data_buf));
    memset(addr_repr_buf, 0, sizeof(addr_repr_buf));
    recv_len = 0;
    memset(&header, 0, sizeof(header));
    memset(&out_src_addr, 0, sizeof(out_src_addr));

    while (g_FN_mainthread_running)
    {
        while (i < g_MULTICAST_hostnames.size)
        {
            /* Send request for hostname. */
            if (UTIL_timer_ontime(&timer, 1000))
            {
                if (ask4_count > 0)
                {
                    if (sk4 != NET_INVALID_SOCKET)
                    {
                        ret = MULTICAST_send_request(sk4, g_MULTICAST_hostnames.arr[i],
                                                     strlen(g_MULTICAST_hostnames.arr[i]), NET_INET_VERSION_4);
                        if (!ret.has_value)
                        {
                            LOGGING_pwarn("IPv4 MULTICAST_send_request", ret.error);
                            goto SEND_CONTINUE4;
                        }
                    }
                    ask4_count--;
                }

            SEND_CONTINUE4:
                if (ask6_count > 0)
                {
                    if (sk6 != NET_INVALID_SOCKET)
                    {
                        ret = MULTICAST_send_request(sk6, g_MULTICAST_hostnames.arr[i],
                                                     strlen(g_MULTICAST_hostnames.arr[i]), NET_INET_VERSION_6);
                        if (!ret.has_value)
                        {
                            LOGGING_pwarn("IPv6 MULTICAST_send_request", ret.error);
                            goto SEND_CONTINUE6;
                        }
                    }
                    ask6_count--;
                }
                UTIL_timer_start(&timer);
            SEND_CONTINUE6:;
            }

            /* Try to recv response for IPv4. */
            if (sk4 != NET_INVALID_SOCKET)
            {
                ret = MULTICAST_recv_boardcast(sk4, data_buf, &recv_len, &header, &out_src_addr);
                if (!ret.has_value)
                {
                    if (!NET_need_try_again(ret.error))
                    {
                        LOGGING_pwarn("IPv4 MULTICAST_recv_boardcast", ret.error);
                    }
                    goto RECV_CONTINUE4;
                }
                if (MULTICAST_boardcast_is_for_me(data_buf))
                {
                    if (header.type == MULTICAST_TYPE_REFUSE)
                    {
                        NET_repr_inet_addr(&out_src_addr, addr_repr_buf, sizeof(addr_repr_buf));
                        LOGGING_error("Hostname %s refused to join by [%s]", data_buf, addr_repr_buf);
                        ret = RESULT_error_void(EFN_JOIN_REFUSED, EXC_CATEGORY_FN);
                        goto EXIT;
                    }
                }
            }

        RECV_CONTINUE4:
            /* Try to recv response for IPv6. */
            if (sk6 != NET_INVALID_SOCKET)
            {
                ret = MULTICAST_recv_boardcast(sk6, data_buf, &recv_len, &header, &out_src_addr);
                if (!ret.has_value)
                {
                    if (!NET_need_try_again(ret.error))
                    {
                        LOGGING_pwarn("IPv6 MULTICAST_recv_boardcast", ret.error);
                    }
                    goto RECV_CONTINUE6;
                }
                if (MULTICAST_boardcast_is_for_me(data_buf))
                {
                    if (header.type == MULTICAST_TYPE_REFUSE)
                    {
                        NET_repr_inet_addr(&out_src_addr, addr_repr_buf, sizeof(addr_repr_buf));
                        LOGGING_error("Hostname %s refused to join by [%s]", data_buf, addr_repr_buf);
                        ret = RESULT_error_void(EFN_JOIN_REFUSED, EXC_CATEGORY_FN);
                        goto EXIT;
                    }
                }
            }

        RECV_CONTINUE6:
            /* Check if done asking for this hostname. */
            if (ask4_count <= 0 && ask6_count <= 0)
            {
                ret = RESULT_ok_void();
                goto EXIT;
            }

            i++;
        }
        i = 0; /* Restart for next round. */
        UTIL_sleep_ms(100);
    }

EXIT:
    return ret;
}

/* Print version information. */
void FN_print_version(void)
{
    printf("Fast Networking %s\n", FN_VERSION_STRING);
    printf("%s\n", FN_COPYRIGHT_STRING);
    printf("This is free software; see the source for copying conditions.  There is NO\n");
    printf("warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
    printf("Written by ChenPi11.\n");
}

/* Print help infomation. */
void FN_print_help(UTIL_StringBuffer prog_name)
{
    printf("Usage: %s [OPTIONS]\n", prog_name);
    printf("Fast Networking - Portable simple networking utility.\n\n");
    printf("Options:\n");
    printf("  -c, --config FILE\tLoad configuration from FILE.\n");
    printf("  -h, --help\t\tPrint this help message.\n");
    printf("  -v, --version\t\tPrint version information.\n");
    printf("  -l, --logging-level LEVEL\tSet logging level to LEVEL.\n");
    printf("  -i, --build-info\tPrint build information.\n");
}

/* Checking for runtime is little endian. */
UTIL_Bool FN_is_little_endian(void)
{
    uint16_t test = 0x1;
    uint8_t *p = (uint8_t *)&test;
    return p[0] == 0x1 ? UTIL_TRUE : UTIL_FALSE;
}

/* Print build infomation. */
void FN_print_build_info(void)
{
    printf("Build Info:\n");

    /* Language standard. */
#if defined(__cplusplus)
    printf("\tLanguage Standard: C++ %ld\n",
#if defined(_MSVC_LANG)
           _MSVC_LANG
#else
           __cplusplus
#endif
    );
#endif

#if defined(__STDC_VERSION__)
    printf("\tLanguage Standard: C %ld\n", __STDC_VERSION__);
#else
    printf("\tLanguage Standard: C89\n");
#endif

#if defined(__STDC__)
    printf("\tANSI C support: %s\n", __STDC__ ? "Yes" : "No");
#endif

    /* Compiler. */
#if defined(__GNUC__)
    printf("\tCompiler: GCC %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(__clang__)
    printf("\tCompiler: Clang %d.%d.%d\n", __clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(_MSC_VER)
    printf("\tCompiler: MSVC %d\n", _MSC_VER);
#else
    printf("\tCompiler: Unknown\n");
#endif

    /* Platform. */
    printf("\tPlatform: %s\n",
#if defined(_WIN32) || defined(_WIN64)
           "Windows"
#elif defined(__CYGWIN__)
           "Cygwin"
#elif defined(__MINGW32__) || defined(__MINGW64__)
           "Windows (MinGW)"
#elif defined(__linux__)
           "Linux"
#elif defined(__APPLE__)
           "Darwin"
#elif defined(__FreeBSD__)
           "FreeBSD"
#elif defined(__NetBSD__)
           "NetBSD"
#elif defined(__OpenBSD__)
           "OpenBSD"
#elif defined(__unix__) || defined(__unix)
           "Unix"
#else
           "Unknown"
#endif
    );

    /* Build date and time. */
    printf("\tBuild Date: %s\n", __DATE__);
    printf("\tBuild Time: %s\n", __TIME__);
    printf("\tBuild File: %s\n", __FILE__);

    /* Endianness. */
    printf("\tEndianness: %s\n",
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
           "Little Endian"
#elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
           "Big Endian"
#else
           "Unknown"
#endif
    );

    /* Runtime byte order. */
    printf("\tRuntime Byte Order: %s\n", FN_is_little_endian() ? "Little Endian" : "Big Endian");
}

/* Print all configuration entries. */
void FN_iter_config_entries(void)
{
    CONFIG_ConfigEntryIter iter;

    CONFIG_iter_start(iter);
    while (!CONFIG_iter_is_end(iter))
    {
        struct CONFIG_ConfigEntry *entry = iter;
        if (entry->type == CONFIG_CONFIG_ENTRY_TYPE_STRING)
        {
            LOGGING_info("string\t%s\t\t%s", entry->key, entry->value.str_val);
        }
        else if (entry->type == CONFIG_CONFIG_ENTRY_TYPE_INT)
        {
            LOGGING_info("int\t%s\t\t%d", entry->key, entry->value.int_val);
        }
        CONFIG_iter_next(iter);
    }
}

/* Parse FastNetworking arguments. */
int FN_parse_arguments(int argc, UTIL_StringBuffer argv[])
{
    RESULT_void res;
    int opt;

    for (opt = 1; opt < argc; opt++)
    {
        if (strcmp(argv[opt], "--config") == 0 || strcmp(argv[opt], "-c") == 0)
        {
            if (opt + 1 < argc)
            {
                CONFIG_load_from_file(argv[opt + 1]);
                opt++;
            }
            else
            {
                LOGGING_error("Missing argument for --config");
                return EXIT_FAILURE;
            }
        }
        else if (strcmp(argv[opt], "--help") == 0 || strcmp(argv[opt], "-h") == 0)
        {
            FN_print_help(argv[0]);
            return EXIT_SUCCESS;
        }
        else if (strcmp(argv[opt], "--version") == 0 || strcmp(argv[opt], "-v") == 0)
        {
            FN_print_version();
            return EXIT_SUCCESS;
        }
        else if (strcmp(argv[opt], "--logging-level") == 0 || strcmp(argv[opt], "-l") == 0)
        {
            if (opt + 1 < argc)
            {
                struct CONFIG_ConfigEntry entry;
                uint64_t level;
                RESULT_uint64_t level_res;

                level_res = UTIL_str_to_unsigned_base10(argv[opt + 1]);
                if (!level_res.has_value)
                {
                    LOGGING_error("Invalid logging level %s", argv[opt + 1]);
                    return EXIT_FAILURE;
                }

                level = level_res.result.value;
                entry.type = CONFIG_CONFIG_ENTRY_TYPE_INT;
                entry.key = NULL;
                entry.value.int_val = level;

                CONFIG_set_config(OPT_LOGGING_LEVEL, &entry);
                opt++;
            }
            else
            {
                LOGGING_error("Missing argument for --logging-level");
                return EXIT_FAILURE;
            }
        }
        else if (strcmp(argv[opt], "--list-config") == 0)
        {
            FN_iter_config_entries();
            return EXIT_SUCCESS;
        }
        else if (strcmp(argv[opt], "--build-info") == 0 || strcmp(argv[opt], "-i") == 0)
        {
            FN_print_build_info();
            return EXIT_SUCCESS;
        }
        else
        {
            LOGGING_error("Unknown argument: %s", (const char *)argv[opt]);
            return EXIT_FAILURE;
        }
    }

    /* Apply settings of logging. */
    res = FN__apply_logging_settings();
    if (!res.has_value)
    {
        LOGGING_perror("Error while apply logging settings", res.error);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* Main logic. */
int FN_main(int argc, UTIL_StringBuffer argv[])
{
    int proc_ret;
    RESULT_void ret;
    int ret2;
    struct MAP_Map record_inet4_map; /* Mostly important: store hostname4 -> record. */
    struct MAP_Map record_inet6_map; /* Mostly important: store hostname6 -> record. */

    NET_Socket multicast_sk4;
    NET_Socket multicast_sk6;
    NET_Socket dns_sk4;
    NET_Socket dns_sk6;
    RESULT_NET_Socket sk_res;

    struct UTIL_Timer boardcast_timer;

    proc_ret = EXIT_SUCCESS;

    /* Initialize Fast Networking. */
    ret = FN_initialize();
    if (!ret.has_value)
    {
        LOGGING_perror("Error while initializing Fast Networking", ret.error);
        return EXIT_FAILURE;
    }

    /* Parse arguments. */
    ret2 = FN_parse_arguments(argc, argv);
    if (ret2 != EXIT_SUCCESS)
    {
        return ret2;
    }

    multicast_sk4 = NET_INVALID_SOCKET;
    multicast_sk6 = NET_INVALID_SOCKET;
    dns_sk4 = NET_INVALID_SOCKET;
    dns_sk6 = NET_INVALID_SOCKET;
    g_FN_mainthread_running = UTIL_TRUE;

    /* Create DNS server sockets. */
    NET_async_socket(NET_INET_VERSION_6);
    NET_async_socket(NET_INET_VERSION_6);
    NET_async_socket(NET_INET_VERSION_6);
    sk_res = NET_async_socket(NET_INET_VERSION_4);
    if (!sk_res.has_value)
    {
        LOGGING_pwarn("IPv4 NET_async_socket", sk_res.result.error);
        dns_sk4 = NET_INVALID_SOCKET; /* Mark as invalid socket. */
        proc_ret = EXIT_FAILURE;
    }
    else
        dns_sk4 = sk_res.result.value;

    sk_res = NET_async_socket(NET_INET_VERSION_6);
    if (!sk_res.has_value)
    {
        LOGGING_pwarn("IPv6 NET_async_socket", sk_res.result.error);
        dns_sk6 = NET_INVALID_SOCKET; /* Mark as invalid socket. */
        proc_ret = EXIT_FAILURE;
    }
    else
        dns_sk6 = sk_res.result.value;

    /* Bind DNS server sockets. */
    if (dns_sk4 != NET_INVALID_SOCKET)
    {
        ret = DNS_bind_socket(dns_sk4, DNS_ADDR4, DNS_PORT, NET_INET_VERSION_4);
        if (!ret.has_value)
        {
            LOGGING_pwarn("IPv4 DNS_bind_socket", ret.error);
            NET_closesocket(dns_sk4);
            dns_sk4 = NET_INVALID_SOCKET; /* Mark as invalid socket. */
            proc_ret = EXIT_FAILURE;
        }
        else
            LOGGING_info("IPv4 DNS server socket created: [%s]:%u", DNS_ADDR4, DNS_PORT);
    }
    if (dns_sk6 != NET_INVALID_SOCKET)
    {
        ret = DNS_bind_socket(dns_sk6, DNS_ADDR6, DNS_PORT, NET_INET_VERSION_6);
        if (!ret.has_value)
        {
            LOGGING_pwarn("IPv6 DNS_bind_socket", ret.error);
            NET_closesocket(dns_sk6);
            dns_sk6 = NET_INVALID_SOCKET; /* Mark as invalid socket. */
            proc_ret = EXIT_FAILURE;
        }
        else
            LOGGING_info("IPv6 DNS server socket created: [%s]:%u", DNS_ADDR6, DNS_PORT);
    }
    /* Checking if no valid DNS server sockets. */
    if (dns_sk4 == NET_INVALID_SOCKET && dns_sk6 == NET_INVALID_SOCKET)
    {
        LOGGING_error("No valid DNS server sockets created. Exiting ...");
        proc_ret = EXIT_FAILURE;
        goto EXIT;
    }

    /* Multicast socket. */
    sk_res = NET_async_socket(NET_INET_VERSION_4);
    if (!sk_res.has_value)
    {
        LOGGING_pwarn("IPv4 NET_async_socket", sk_res.result.error);
        multicast_sk4 = NET_INVALID_SOCKET; /* Mark as invalid socket. */
        proc_ret = EXIT_FAILURE;
    }
    else
        multicast_sk4 = sk_res.result.value;
    sk_res = NET_async_socket(NET_INET_VERSION_6);
    if (!sk_res.has_value)
    {
        LOGGING_pwarn("IPv6 NET_async_socket", sk_res.result.error);
        multicast_sk6 = NET_INVALID_SOCKET; /* Mark as invalid socket. */
        proc_ret = EXIT_FAILURE;
    }
    else
        multicast_sk6 = sk_res.result.value;

    /* Initialize multicast sockets. */
    if (multicast_sk4 != NET_INVALID_SOCKET)
    {
        ret = MULTICAST_inet4_init_multicast_socket(multicast_sk4, MULTICAST_ADDR);
        if (!ret.has_value)
        {
            LOGGING_pwarn("IPv4 MULTICAST_inet4_init_multicast_socket", ret.error);
            proc_ret = EXIT_FAILURE;
            NET_closesocket(multicast_sk4);
            multicast_sk4 = NET_INVALID_SOCKET; /* Mark as invalid socket. */
        }
        else
            LOGGING_info("IPv4 multicast socket created: [%s]:%u", MULTICAST_ADDR, MULTICAST_PORT);
    }
    if (multicast_sk6 != NET_INVALID_SOCKET)
    {
        ret = MULTICAST_inet6_init_multicast_socket(multicast_sk6, MULTICAST_ADDR6);
        if (!ret.has_value)
        {
            LOGGING_pwarn("IPv6 MULTICAST_inet6_init_multicast_socket", ret.error);
            proc_ret = EXIT_FAILURE;
            NET_closesocket(multicast_sk6);
            multicast_sk6 = NET_INVALID_SOCKET; /* Mark as invalid socket. */
        }
        else
            LOGGING_info("IPv6 multicast socket created: [%s]:%u", MULTICAST_ADDR6, MULTICAST_PORT);
    }
    /* Checking if no valid multicast sockets. */
    if (multicast_sk4 == NET_INVALID_SOCKET && multicast_sk6 == NET_INVALID_SOCKET)
    {
        LOGGING_error("No valid multicast sockets created. Exiting ...");
        proc_ret = EXIT_FAILURE;
        goto EXIT;
    }

    /* Allocate memory for record maps. */
    ret = MAP_create(&record_inet4_map, 4);
    if (!ret.has_value)
    {
        LOGGING_perror("MAP_create for IPv4 record map", ret.error);
        proc_ret = EXIT_FAILURE;
        goto EXIT;
    }
    ret = MAP_create(&record_inet6_map, 4);
    if (!ret.has_value)
    {
        LOGGING_perror("MAP_create for IPv6 record map", ret.error);
        proc_ret = EXIT_FAILURE;
        goto EXIT;
    }

    /* Register signal handlers. */
    if (signal(SIGINT, FN_signal_handler) == SIG_ERR)
    {
        LOGGING_pwarn("signal", EXC_get_last_error(EXC_CATEGORY_LIBC));
    }

    /* Request to join the network. */
    ret = FN_request_join(multicast_sk4, multicast_sk6);
    if (!ret.has_value)
    {
        LOGGING_perror("FN_request_join", ret.error);
        proc_ret = EXIT_FAILURE;
        goto EXIT;
    }

    LOGGING_info("Fast Networking main loop started. Press Ctrl+C to exit.");
    if (multicast_sk4 != NET_INVALID_SOCKET)
    {
        ret = MULTICAST_boardcast_self(multicast_sk4, NET_INET_VERSION_4);
        if (!ret.has_value)
        {
            LOGGING_pwarn("IPv4 MULTICAST_boardcast_self", ret.error);
        }
    }
    if (multicast_sk6 != NET_INVALID_SOCKET)
    {
        ret = MULTICAST_boardcast_self(multicast_sk6, NET_INET_VERSION_6);
        if (!ret.has_value)
        {
            LOGGING_pwarn("IPv6 MULTICAST_boardcast_self", ret.error);
        }
    }
    UTIL_timer_start(&boardcast_timer);
    /* Main loop. */
    while (g_FN_mainthread_running)
    {
        /* Try to recv IPv4 response. */
        if (multicast_sk4 != NET_INVALID_SOCKET)
        {
            ret = FN_recv_boardcast(multicast_sk4, &record_inet4_map, &record_inet6_map);
            if (!ret.has_value)
            {
                if (NET_need_try_again(ret.error))
                {
                    goto RECV_CONTINUE4;
                }
                LOGGING_pwarn("FN_recv_boardcast", ret.error);
            }
        }

    RECV_CONTINUE4:
        /* Try to recv IPv6 response. */
        if (multicast_sk6 != NET_INVALID_SOCKET)
        {
            ret = FN_recv_boardcast(multicast_sk6, &record_inet4_map, &record_inet6_map);
            if (!ret.has_value)
            {
                if (NET_need_try_again(ret.error))
                {
                    goto RECV_CONTINUE6;
                }
                LOGGING_pwarn("FN_recv_boardcast", ret.error);
            }
        }

    RECV_CONTINUE6:
        /* Send boardcast packet. */
        if (UTIL_timer_ontime(&boardcast_timer, MULTICAST_BOARDCAST_INTERVAL_MS))
        {
            UTIL_timer_start(&boardcast_timer);
            if (multicast_sk4 != NET_INVALID_SOCKET)
            {
                ret = MULTICAST_boardcast_self(multicast_sk4, NET_INET_VERSION_4);
                if (!ret.has_value && !NET_need_try_again(ret.error))
                {
                    LOGGING_pwarn("IPv4 MULTICAST_boardcast_self", ret.error);
                }
            }
            if (multicast_sk6 != NET_INVALID_SOCKET)
            {
                ret = MULTICAST_boardcast_self(multicast_sk6, NET_INET_VERSION_6);
                if (!ret.has_value && !NET_need_try_again(ret.error))
                {
                    LOGGING_pwarn("IPv6 MULTICAST_boardcast_self", ret.error);
                }
            }
        }

        /* Recv DNS query. */
        if (dns_sk4 != NET_INVALID_SOCKET)
        {
            ret = FN_process_dns_query(dns_sk4, &record_inet4_map, &record_inet6_map);
            if (!ret.has_value)
            {
                if (!NET_need_try_again(ret.error))
                {
                    LOGGING_pwarn("IPv4 FN_recv_dns_query", ret.error);
                }
                goto DNS_CONTINUE4;
            }
        }

    DNS_CONTINUE4:
        if (dns_sk6 != NET_INVALID_SOCKET)
        {
            ret = FN_process_dns_query(dns_sk6, &record_inet4_map, &record_inet6_map);
            if (!ret.has_value)
            {
                if (!NET_need_try_again(ret.error))
                {
                    LOGGING_pwarn("IPv6 FN_recv_dns_query", ret.error);
                }
                goto DNS_CONTINUE6;
            }
        }

    DNS_CONTINUE6:
        /* Sleep for a short duration to avoid busy-waiting. */
        UTIL_sleep_ms(10);
    }

    /* Send offline notice to other nodes. */
    if (multicast_sk4 != NET_INVALID_SOCKET)
    {
        ret = MULTICAST_offline_self(multicast_sk4, NET_INET_VERSION_4);
        if (!ret.has_value && !NET_need_try_again(ret.error))
        {
            LOGGING_pwarn("IPv4 MULTICAST_offline_self", ret.error);
        }
    }
    if (multicast_sk6 != NET_INVALID_SOCKET)
    {
        ret = MULTICAST_offline_self(multicast_sk6, NET_INET_VERSION_6);
        if (!ret.has_value && !NET_need_try_again(ret.error))
        {
            LOGGING_pwarn("IPv6 MULTICAST_offline_self", ret.error);
        }
    }

    LOGGING_info("Fast Networking main loop stopped.");
    proc_ret = EXIT_SUCCESS;

EXIT:
    NET_closesocket(multicast_sk4);
    NET_closesocket(multicast_sk6);
    NET_closesocket(dns_sk4);
    NET_closesocket(dns_sk6);
    MAP_free(&record_inet4_map);
    MAP_free(&record_inet6_map);
    CONFIG_deinitialize();
    MULTICAST_deinitialize();
    NET_deinitialize();

    return proc_ret;
}

int main(int argc, char **argv)
{
    return FN_main(argc, argv);
}
