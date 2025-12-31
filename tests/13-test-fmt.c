/* @NO MRG */
#include "00-test-util.h"
/* @END NO MRG */

int main()
{
	atexit(_reset);

	EXC_initialize();

	FNT_test(__FILE__, __LINE__, "Simple usage.", "Hello World!",
			 "Hello World!");
	FNT_test(__FILE__, __LINE__, "Single string placeholder.", "Value: 42",
			 "Value: %1", FMT_str("42"));
	FNT_test(__FILE__, __LINE__, "Multiple string placeholders.", "A-B-C",
			 "%1-%2-%3", FMT_str("A"), FMT_str("B"), FMT_str("C"));
	FNT_test(__FILE__, __LINE__, "Escaped percent.", "Progress: 50%",
			 "Progress: %1%%", FMT_str("50"));
	FNT_test(__FILE__, __LINE__, "1 Uint16.", "Value: 0", "Value: %1",
			 FMT_uint16_autoalign(0));
	FNT_test(__FILE__, __LINE__, "2 Uint16.", "Value: 10", "Value: %1",
			 FMT_uint16_autoalign(10));
	FNT_test(__FILE__, __LINE__, "3 Uint16.", "Value: 100", "Value: %1",
			 FMT_uint16_autoalign(100));
	FNT_test(__FILE__, __LINE__, "4 Uint16.", "Value: 1000", "Value: %1",
			 FMT_uint16_autoalign(1000));
	FNT_test(__FILE__, __LINE__, "5 Uint16.", "Value: 65535", "Value: %1",
			 FMT_uint16_autoalign(65535));
	FNT_test(__FILE__, __LINE__, "Hex16.", "Value: 0xABCD", "Value: %1",
			 FMT_hex16_4(0xABCD));
	FNT_test(__FILE__, __LINE__, "Hex16 with leading zero.", "Value: 0x00FF",
			 "Value: %1", FMT_hex16_4(0x00FF));

	/* Basic format string tests. */
	FNT_test(__FILE__, __LINE__, "Empty format string", "", "");
	FNT_test(__FILE__, __LINE__, "Format string with only %", "%", "%%");
	FNT_test(__FILE__, __LINE__, "Multiple consecutive %", "%%%", "%%%%%%");
	FNT_test(__FILE__, __LINE__, "Single % at end", "Test%", "Test%%");

	/* Placeholder tests. */
	FNT_test(__FILE__, __LINE__, "Placeholder %1 only", "Value", "%1",
			 FMT_str("Value"));
	FNT_test(__FILE__, __LINE__, "Placeholder %9 only", "Last", "%9",
			 FMT_str("1"), FMT_str("2"), FMT_str("3"), FMT_str("4"),
			 FMT_str("5"), FMT_str("6"), FMT_str("7"), FMT_str("8"),
			 FMT_str("Last"));
	FNT_test(__FILE__, __LINE__, "Placeholder %0 (invalid)", "%0", "%0");
	FNT_test(__FILE__, __LINE__, "Placeholder %10 (two-digit, invalid)", "%10",
			 "%10");
	FNT_test(__FILE__, __LINE__, "Placeholder %999 (multi-digit, invalid)",
			 "%999", "%999");

	/* Argument missing tests. */
	FNT_test(__FILE__, __LINE__, "Missing argument for %1", "%1", "%1",
			 FMT_END);
	FNT_test(__FILE__, __LINE__, "Missing argument for %2", "A%2%3", "%1%2%3",
			 FMT_str("A"), FMT_END);
	FNT_test(__FILE__, __LINE__, "Only first argument provided", "A%2%3",
			 "%1%2%3", FMT_str("A"), FMT_END);
	FNT_test(__FILE__, __LINE__, "Skip middle placeholder", "a%2%3", "%1%2%3",
			 FMT_str("a"), FMT_END, FMT_str("c"));

	/* Placeholder order and repetition tests. */
	FNT_test(__FILE__, __LINE__, "Reverse order placeholders", "321", "%3%2%1",
			 FMT_str("1"), FMT_str("2"), FMT_str("3"));
	FNT_test(__FILE__, __LINE__, "Repeated placeholder", "AAA", "%1%1%1",
			 FMT_str("A"));
	FNT_test(__FILE__, __LINE__, "Mixed repeated placeholders", "ABAB",
			 "%1%2%1%2", FMT_str("A"), FMT_str("B"));
	FNT_test(__FILE__, __LINE__, "Non-sequential placeholders", "A2C4",
			 "%1%2%3%4", FMT_str("A"), FMT_str("2"), FMT_str("C"),
			 FMT_str("4"));

	/* Mixed type tests. */
	FNT_test(__FILE__, __LINE__, "String and uint16 mix",
			 "Count: 42, Text: Hello", "Count: %1, Text: %2",
			 FMT_uint16_autoalign(42), FMT_str("Hello"));
	FNT_test(__FILE__, __LINE__, "String and hex mix",
			 "Address: 0x1234, Data: test", "Address: %1, Data: %2",
			 FMT_hex16_4(0x1234), FMT_str("test"));
	FNT_test(__FILE__, __LINE__, "All types mix",
			 "Dec: 100, Hex: 0x00FF, Str: mixed", "Dec: %1, Hex: %2, Str: %3",
			 FMT_uint16_autoalign(100), FMT_hex16_4(0x00FF), FMT_str("mixed"));

	/* Uint16 value tests. */
	FNT_test(__FILE__, __LINE__, "Uint16 min value", "0", "%1",
			 FMT_uint16_autoalign(0));
	FNT_test(__FILE__, __LINE__, "Uint16 max value", "65535", "%1",
			 FMT_uint16_autoalign(65535));
	FNT_test(__FILE__, __LINE__, "Uint16 value 42", "42", "%1",
			 FMT_uint16_autoalign(42));

	/* Uint64 value tests. */
	FNT_test(__FILE__, __LINE__, "Uint64 min value", "0", "%1",
			 FMT_uint64_autoalign(0));
	FNT_test(__FILE__, __LINE__, "Uint64 max value", "18446744073709551615",
			 "%1", FMT_uint64_autoalign((UTIL_UInt64)(-1)));
	FNT_test(__FILE__, __LINE__, "Uint64 value 42", "42", "%1",
			 FMT_uint64_autoalign((UTIL_UInt64)42));

	/* Hex16 value tests. */
	FNT_test(__FILE__, __LINE__, "Hex16 min value", "0x0000", "%1",
			 FMT_hex16_4(0x0000));
	FNT_test(__FILE__, __LINE__, "Hex16 max value", "0xFFFF", "%1",
			 FMT_hex16_4(0xFFFF));
	FNT_test(__FILE__, __LINE__, "Hex16 value 0x42", "0x0042", "%1",
			 FMT_hex16_4(0x42));

	/* Complex string tests. */
	FNT_test(__FILE__, __LINE__, "Empty string argument",
			 "Empty: ", "Empty: %1", FMT_str(""));
	FNT_test(__FILE__, __LINE__, "String with special chars", "Tabs\tNewline\n",
			 "%1", FMT_str("Tabs\tNewline\n"));
	FNT_test(__FILE__, __LINE__, "String with % sign inside", "50% done", "%1",
			 FMT_str("50% done"));
	FNT_test(__FILE__, __LINE__, "String looks like placeholder", "Test%1",
			 "%1", FMT_str("Test%1"));
	FNT_test(__FILE__, __LINE__, "String contains digits", "Version 2.1.0",
			 "%1", FMT_str("Version 2.1.0"));

	/* Miscellaneous format string tests. */
	FNT_test(__FILE__, __LINE__, "Placeholder at start", "Hello World",
			 "%1 World", FMT_str("Hello"));
	FNT_test(__FILE__, __LINE__, "Placeholder at end", "Hello World",
			 "Hello %1", FMT_str("World"));
	FNT_test(__FILE__, __LINE__, "Multiple placeholders consecutive", "123",
			 "%1%2%3", FMT_str("1"), FMT_str("2"), FMT_str("3"));
	FNT_test(__FILE__, __LINE__, "Placeholders with spaces", "a b c",
			 "%1 %2 %3", FMT_str("a"), FMT_str("b"), FMT_str("c"));

	/* Invalid % usage tests. */
	FNT_test(__FILE__, __LINE__, "% followed by space", "Test % more",
			 "Test % more");
	FNT_test(__FILE__, __LINE__, "% followed by letter", "Test %abc",
			 "Test %abc");
	FNT_test(__FILE__, __LINE__, "% followed by punctuation", "Test %! Test",
			 "Test %! Test");
	FNT_test(__FILE__, __LINE__, "Trailing % without escape", "Test%", "Test%");

	/* Maximum argument count tests. */
	FNT_test(__FILE__, __LINE__, "All 9 placeholders used", "123456789",
			 "%1%2%3%4%5%6%7%8%9", FMT_str("1"), FMT_str("2"), FMT_str("3"),
			 FMT_str("4"), FMT_str("5"), FMT_str("6"), FMT_str("7"),
			 FMT_str("8"), FMT_str("9"));
	FNT_test(__FILE__, __LINE__, "9 mixed arguments", "1a0x00022b0xFFFF3cX",
			 "%1%2%3%4%5%6%7%8%9", FMT_uint16_autoalign(1), FMT_str("a"),
			 FMT_hex16_4(0x0002), FMT_str("2"), FMT_str("b"),
			 FMT_hex16_4(0xFFFF), FMT_uint16_autoalign(3), FMT_str("c"),
			 FMT_str("X"), FMT_END);

	/* Real-world format string tests. */
	FNT_test(__FILE__, __LINE__, "Log message format",
			 "[INFO] User 42 connected from 0x0A0B",
			 "[%1] User %2 connected from %3", FMT_str("INFO"),
			 FMT_uint16_autoalign(42), FMT_hex16_4(0x0A0B));
	FNT_test(__FILE__, __LINE__, "Error message", "Error 404: File not found",
			 "Error %1: %2", FMT_uint16_autoalign(404),
			 FMT_str("File not found"));
	FNT_test(__FILE__, __LINE__, "Network packet format",
			 "Packet: id=0x1234, seq=100, data=test",
			 "Packet: id=%1, seq=%2, data=%3", FMT_hex16_4(0x1234),
			 FMT_uint16_autoalign(100), FMT_str("test"));

	/* Nested and complex placeholder combinations. */
	FNT_test(__FILE__, __LINE__, "Complex escaped %", "50% of 0x1000 = 2048",
			 "%1%% of %2 = %3", FMT_str("50"), FMT_hex16_4(0x1000),
			 FMT_uint16_autoalign(2048));
	FNT_test(__FILE__, __LINE__, "Placeholder after escaped %", "Test%1Result",
			 "Test%%%1Result", FMT_str("1"));
	FNT_test(__FILE__, __LINE__, "%% within placeholders", "A%B%C",
			 "%1%%%2%%%3", FMT_str("A"), FMT_str("B"), FMT_str("C"));

	/* Number combinations in format strings. */
	FNT_test(__FILE__, __LINE__, "Number after placeholder", "Value1 is 10",
			 "Value%1 is %2", FMT_str("1"), FMT_uint16_autoalign(10));

	return EXIT_SUCCESS;
}
