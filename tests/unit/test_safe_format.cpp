//----------------------------------------------------------------------
// test_safe_format.cpp
//----------------------------------------------------------------------
//
// SafeFormat (basic/SafeFormat.h) - the checked formatter that stands
// between Data/Info/String.inf and sprintf (docs/RESTRUCTURING.md task
// 5.4, docs/code-health-review-2026-08-29.md finding C19).
//
// Two things are under test and they pull in opposite directions. The
// first is that the ordinary vocabulary of the string table - "%s",
// "%d", "%02d", "%s (%d,%d)" - still prints exactly what sprintf printed,
// because a formatter that changes the user interface is not a fix. The
// second is that a format the call site does not match can no longer make
// the program read or write anything: the specification comes out as
// text.
//
// The hostile cases are the reason the file exists, so they are written
// as the observable contract - the literal text that appears and the
// value that is left alone - rather than as "does not crash", which an
// out of bounds read in C++ would pass anyway.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "SafeFormat.h"

#include <cstring>
#include <string>


namespace {

//----------------------------------------------------------------------
// Compared through a helper rather than by CHECK_EQ, which is integer
// only, and NULL tolerant so a regression fails the check instead of
// crashing the whole run.
//----------------------------------------------------------------------
bool
Is(const char* expected, const char* actual)
{
	return actual != NULL && std::strcmp(actual, expected) == 0;
}

} // anonymous namespace


//----------------------------------------------------------------------
// What the string table really contains
//----------------------------------------------------------------------

TEST(SafeFormat, PrintsTheOrdinaryTableVocabularyExactlyAsSprintfDid)
{
	char buf[64];

	SafeFormat::Format(buf, "You can resurrect in %d seconds.", 12);
	CHECK(Is("You can resurrect in 12 seconds.", buf));

	SafeFormat::Format(buf, "%s (%d,%d)", "Adam", 132, 47);
	CHECK(Is("Adam (132,47)", buf));

	SafeFormat::Format(buf, "%02d:%02d:%02d", 7, 5, 9);
	CHECK(Is("07:05:09", buf));

	SafeFormat::Format(buf, "%s(%d)", "Gold", 250);
	CHECK(Is("Gold(250)", buf));

	SafeFormat::Format(buf, "%d/%d/%d", 2026, 9, 3);
	CHECK(Is("2026/9/3", buf));
}

TEST(SafeFormat, CopiesAFormatWithNoConversionsThrough)
{
	char buf[64];

	const int written = SafeFormat::Format(buf, "The connection is unstable.");

	CHECK(Is("The connection is unstable.", buf));
	CHECK_EQ(27, written);
}

TEST(SafeFormat, TreatsADoubledPercentAsOneLiteralPercent)
{
	char buf[32];

	SafeFormat::Format(buf, "100%% done");

	CHECK(Is("100% done", buf));
}

TEST(SafeFormat, AcceptsAStdStringArgument)
{
	char buf[64];
	const std::string name = "Lorraine";

	SafeFormat::Format(buf, "[System]%s", name);

	CHECK(Is("[System]Lorraine", buf));
}

TEST(SafeFormat, ReturnsTheNumberOfBytesItWrote)
{
	char buf[64];

	CHECK_EQ(5, SafeFormat::Format(buf, "%d", 12345));
	CHECK_EQ(0, SafeFormat::Format(buf, ""));
	CHECK_EQ(8, SafeFormat::Format(buf, "%s", "12345678"));
}


//----------------------------------------------------------------------
// The primitive finding C19 describes: a conversion with no argument
//----------------------------------------------------------------------

TEST(SafeFormat, PrintsAConversionWithNoArgumentAsText)
{
	char buf[64];

	// This is the whole exploit in one line. sprintf here reads a stack
	// word as a char* and copies it, unbounded, into buf.
	SafeFormat::Format(buf, "%s");

	CHECK(Is("%s", buf));
}

TEST(SafeFormat, StopsSubstitutingWhenTheArgumentsRunOut)
{
	char buf[64];

	SafeFormat::Format(buf, "%s%s%s%s", "one");

	CHECK(Is("one%s%s%s", buf));
}

TEST(SafeFormat, PrintsAConversionTheArgumentTypeCannotSatisfyAsText)
{
	char buf[64];

	// An int is not a char*, so the %s is not performed - and it does not
	// consume the argument either, so the %d that follows still gets it.
	SafeFormat::Format(buf, "%s %d", 5);

	CHECK(Is("%s 5", buf));
}

TEST(SafeFormat, RefusesAConversionThatWouldWriteThroughItsArgument)
{
	char buf[64];
	int counter = 4242;

	SafeFormat::Format(buf, "abc%n", &counter);

	CHECK(Is("abc%n", buf));
	CHECK_EQ(4242, counter);
}

TEST(SafeFormat, RefusesAWidthTakenFromAnArgument)
{
	char buf[64];

	// %*d consumes two arguments where the call site counted on one, so
	// every later conversion in a real entry would be reading the wrong
	// value. It is refused, and consumes nothing.
	SafeFormat::Format(buf, "[%*d]", 10, 5);

	CHECK(Is("[%*d]", buf));
}

TEST(SafeFormat, RefusesAWideConversionThatWouldRetypeACharPointer)
{
	char buf[64];

	// %S reads a char* as a wchar_t* and scans past the end of it. The
	// argument count looks right, which is what makes it dangerous.
	SafeFormat::Format(buf, "%S", "narrow");

	CHECK(Is("%S", buf));
}

TEST(SafeFormat, PrintsALengthPrefixedStringConversionNarrow)
{
	char buf[64];

	// %ls is the same trap spelled differently. Here the length modifier
	// is dropped rather than the conversion refused: the argument really
	// is a char*, so it prints as one.
	SafeFormat::Format(buf, "%ls", "narrow");

	CHECK(Is("narrow", buf));
}

TEST(SafeFormat, RefusesAFloatingConversionAgainstAnIntegerArgument)
{
	char buf[64];

	// The review measured this one against the real MSVC runtime: %f
	// needs no width and emitted 308 bytes from a register the call site
	// never set.
	SafeFormat::Format(buf, "%f", 3);

	CHECK(Is("%f", buf));
}

TEST(SafeFormat, PerformsAFloatingConversionWhenTheArgumentIsADouble)
{
	char buf[64];

	SafeFormat::Format(buf, "%.2f", 1.5);

	CHECK(Is("1.50", buf));
}

TEST(SafeFormat, RefusesAPointerConversionAgainstANonPointer)
{
	char buf[64];

	SafeFormat::Format(buf, "%p", 7);

	CHECK(Is("%p", buf));
}

TEST(SafeFormat, PrintsTheHostileNoticeEntryAsMostlyText)
{
	char buf[64];

	// The shape finding C19 and finding C22 both describe: one argument,
	// four string conversions and a write.
	SafeFormat::Format(buf, "[System]%s%s%s%s%n", "hello");

	CHECK(Is("[System]hello%s%s%s%n", buf));
}


//----------------------------------------------------------------------
// Bounds
//----------------------------------------------------------------------

TEST(SafeFormat, TruncatesAtTheDestinationSizeAndTerminates)
{
	char buf[8];
	std::memset(buf, 'Z', sizeof(buf));

	const int written = SafeFormat::Format(buf, "%s", "abcdefghijklmnop");

	CHECK(Is("abcdefg", buf));
	CHECK_EQ(7, written);
	CHECK_EQ('\0', buf[7]);
}

TEST(SafeFormat, TruncatesInTheMiddleOfALiteralRunAndTerminates)
{
	char buf[6];

	const int written = SafeFormat::Format(buf, "%d apples", 12);

	CHECK(Is("12 ap", buf));
	CHECK_EQ(5, written);
}

TEST(SafeFormat, WritesOnlyATerminatorIntoASingleByteDestination)
{
	char buf[2];
	buf[1] = 'Z';

	const int written = SafeFormat::Format(buf, 1, "abcdef");

	CHECK_EQ(0, written);
	CHECK_EQ('\0', buf[0]);

	// Nothing beyond the size it was given.
	CHECK_EQ('Z', buf[1]);
}

TEST(SafeFormat, ClampsAFieldWidthTheDataAsksFor)
{
	char buf[128];

	// A width of 9000 into a 128 byte row would be the whole row. The cap
	// is the one SanitizeGameStringTable rejects at, so a legitimate
	// entry - whose widest real field is 2 - is unaffected.
	const int written = SafeFormat::Format(buf, "%9000d|", 1);

	CHECK_EQ(33, written);
	CHECK_EQ('|', buf[32]);
	CHECK_EQ('1', buf[31]);
}

TEST(SafeFormat, DoesNotOverflowTheWidthAccumulatorOnALongDigitRun)
{
	char buf[64];

	SafeFormat::Format(buf, "%99999999999999999999d|", 7);

	// Whatever the digits said, the field is capped and the rest of the
	// entry still prints.
	CHECK_EQ(33, (int)std::strlen(buf));
	CHECK_EQ('|', buf[32]);
}

TEST(SafeFormat, TruncatesAStringConversionToItsPrecision)
{
	char buf[64];

	SafeFormat::Format(buf, "%.4s!", "abcdefgh");

	CHECK(Is("abcd!", buf));
}


//----------------------------------------------------------------------
// Malformed input
//----------------------------------------------------------------------

TEST(SafeFormat, PrintsATrailingPercentAsText)
{
	char buf[32];

	SafeFormat::Format(buf, "50%");

	CHECK(Is("50%", buf));
}

TEST(SafeFormat, PrintsAFormatThatEndsInsideASpecificationAsText)
{
	char buf[32];

	SafeFormat::Format(buf, "abc%-12", 5);

	CHECK(Is("abc%-12", buf));
}

TEST(SafeFormat, EmptiesTheDestinationForANullFormat)
{
	char buf[16];
	std::memset(buf, 'Z', sizeof(buf));

	const int written = SafeFormat::Format(buf, (const char*)NULL, 1);

	CHECK_EQ(0, written);
	CHECK(Is("", buf));
}

TEST(SafeFormat, SurvivesANullStringArgument)
{
	char buf[32];
	const char* pMissing = NULL;

	SafeFormat::Format(buf, "[%s]", pMissing);

	CHECK(Is("[]", buf));
}

TEST(SafeFormat, IgnoresAnArgumentCountWithNoArgumentArray)
{
	char buf[32];

	// FormatV is the entry point the templates call; a caller that says
	// three arguments and hands over none must not read the pointer.
	const int written = SafeFormat::FormatV(buf, sizeof(buf), "%d%s%d", NULL, 3);

	CHECK(Is("%d%s%d", buf));
	CHECK_EQ(6, written);
}

TEST(SafeFormat, ReportsNothingWrittenForANullDestination)
{
	CHECK_EQ(0, SafeFormat::Format((char*)NULL, 16, "%d", 1));
	CHECK_EQ(0, SafeFormat::Format((char*)NULL, 0, "%d", 1));
}


//----------------------------------------------------------------------
// Argument widths and kinds
//----------------------------------------------------------------------

TEST(SafeFormat, PrintsAnIntegerAtTheWidthTheCallSiteDeclared)
{
	char buf[32];

	// printf("%x", -1) prints ffffffff for an int. Packing every integer
	// into a long long and issuing %llx would print sixteen f here and
	// silently change what these call sites already print.
	SafeFormat::Format(buf, "%x", -1);
	CHECK(Is("ffffffff", buf));

	SafeFormat::Format(buf, "%x", (long long)-1);
	CHECK(Is("ffffffffffffffff", buf));
}

TEST(SafeFormat, PrintsASixtyFourBitValueWithoutTruncatingIt)
{
	char buf[32];

	SafeFormat::Format(buf, "%d", (long long)5000000000LL);

	CHECK(Is("5000000000", buf));
}

TEST(SafeFormat, AcceptsEitherSignednessForAnIntegerConversion)
{
	char buf[32];

	SafeFormat::Format(buf, "%d", (unsigned short)7);
	CHECK(Is("7", buf));

	SafeFormat::Format(buf, "%u", 7);
	CHECK(Is("7", buf));
}

TEST(SafeFormat, AcceptsAnEnumerationAsAnInteger)
{
	enum Race { RACE_SLAYER = 0, RACE_VAMPIRE = 1, RACE_OUSTER = 2 };

	char buf[32];

	SafeFormat::Format(buf, "race %d", RACE_OUSTER);

	CHECK(Is("race 2", buf));
}

TEST(SafeFormat, PrintsACharacterConversion)
{
	char buf[32];

	SafeFormat::Format(buf, "[%c]", 'x');

	CHECK(Is("[x]", buf));
}

TEST(SafeFormat, HonoursFlagsAndWidthTogether)
{
	char buf[32];

	SafeFormat::Format(buf, "[%-6d][%+d]", 42, 42);

	CHECK(Is("[42    ][+42]", buf));
}

TEST(SafeFormat, TakesTheDestinationSizeFromARealArray)
{
	char small[4];

	// The array overload is the one call sites should use: the bound
	// comes from the declaration, so it cannot be stated wrongly.
	SafeFormat::Format(small, "%s", "abcdefgh");

	CHECK(Is("abc", small));
}
