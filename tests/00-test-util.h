/* @NO MRG */
#ifndef FN_TEST_UTIL_H_INC
	#define FN_TEST_UTIL_H_INC

	#include "07-string.h"
	#include "13-fmt.h"
/* @END NO MRG */

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>

static void _red()
{
	#if HAS_WINDOWS
	HANDLE logfile;

	logfile = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(logfile, FOREGROUND_RED | FOREGROUND_INTENSITY);
	#else
	printf("\033[0;31m");
	#endif /* HAS_WINDOWS */
}

static void _green()
{
	#if HAS_WINDOWS
	HANDLE logfile;

	logfile = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(logfile, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	#else
	printf("\033[0;32m");
	#endif /* HAS_WINDOWS */
}

static void _reset()
{
	#if HAS_WINDOWS
	HANDLE logfile;

	logfile = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(logfile, FOREGROUND_RED | FOREGROUND_GREEN |
										 FOREGROUND_BLUE);
	#else
	printf("\033[0;0m");
	#endif /* HAS_WINDOWS */
}

static void log_ok(UTIL_ConstString name, UTIL_ConstString file, int lineno)
{
	_green();
	printf("Test \"%s\" passed (%s:%d)\n", name, file, lineno);
	_reset();
}

static void log_err(UTIL_ConstString name, UTIL_ConstString file, int lineno,
					EXC_ErrorCode code)
{
	_red();
	printf("Test \"%s\" failed (%s:%d): %s\n", name, file, lineno,
		   EXC_strerror(code));
	_reset();
}

static void log_test_failed(UTIL_ConstString name, UTIL_ConstString file,
							int lineno)
{
	_red();
	printf("Test \"%s\" failed (%s:%d): Incorrect test result.\n", name, file,
		   lineno);
	_reset();
}

static void test_fmt(UTIL_ConstString name, UTIL_StringBuffer buf,
					 UTIL_ConstString expected, UTIL_ConstString file,
					 int lineno)
{
	if (strcmp(buf, expected) != 0)
	{
		log_test_failed(name, file, lineno);
		_red();
		fprintf(stderr, "Expected: \"%s\"\n", expected);
		fprintf(stderr, "Got: \"%s\"\n", buf);
		_reset();
		free(buf);
		exit(EXIT_FAILURE);
	}
	free(buf);
}

static void FNT_test(UTIL_ConstString file, int lineno, UTIL_ConstString name,
					 UTIL_ConstString expected, UTIL_ConstString fmt, ...)
{
	va_list args;
	UTIL_StringBuffer buf;
	RESULT_void res;

	va_start(args, fmt);
	res = FMT_vformat(&buf, fmt, args);
	va_end(args);

	if (!res.has_value)
	{
		log_err(name, file, lineno, res.error);
		exit(EXIT_FAILURE);
	}
	test_fmt(name, buf, expected, file, lineno);
	log_ok(name, file, lineno);
}

/* @NO MRG */
#endif /* FN_TEST_UTIL_H_INC */
/* @END NO MRG */
