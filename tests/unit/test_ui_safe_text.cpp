//----------------------------------------------------------------------
// test_ui_safe_text.cpp
//----------------------------------------------------------------------
//
// Pure text preparation used by the line editor and item tooltips. The
// rendering classes themselves pull in the whole UI runtime, so these helpers
// live in a header compiled by both VS_UI and this test binary.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "VS_UI/src/header/UISafeText.h"

#include <stdint.h>
#include <string>


//----------------------------------------------------------------------
// Password masks preserve the legacy one-star-per-byte display contract. This
// matters for old byte limits and for multibyte game text; storage is dynamic
// so the legacy count no longer overruns a 1,024-byte local array.
//----------------------------------------------------------------------
TEST(UISafeText, PasswordMaskPreservesOneGlyphPerInputByte)
{
	const char text[] = { 'A', (char)0xEC, (char)0x95, (char)0x88, '\0' };
	const std::string mask = UISafeText::MakePasswordMask(text);

	CHECK(mask == "****");
}


TEST(UISafeText, PasswordMaskHandlesTheEditorsLargestUtf8Result)
{
	std::string text;
	for (int i=0; i<1023; i++)
	{
		text.append("\xF0\x9F\x98\x80", 4);
	}

	const std::string mask = UISafeText::MakePasswordMask(text.c_str());

	CHECK_EQ(4092, mask.size());
	CHECK(mask == std::string(4092, '*'));
}


TEST(UISafeText, PasswordMaskTreatsNullAndEmptyInputAsEmpty)
{
	CHECK(UISafeText::MakePasswordMask(NULL).empty());
	CHECK(UISafeText::MakePasswordMask("").empty());
}


//----------------------------------------------------------------------
// A cursor position is counted in decoded editor characters. The prefix must
// therefore include every byte of the character at that position, including
// a four-byte supplementary-plane value.
//----------------------------------------------------------------------
TEST(UISafeText, Utf8PrefixEndsAfterACompleteMultibyteCharacter)
{
	const char text[] = {
		'A',
		(char)0xEC, (char)0x95, (char)0x88,
		(char)0xF0, (char)0x9F, (char)0x98, (char)0x80,
		'B', '\0'
	};
	uint32_t decoded[4] = { 0, 0, 0, 0 };

	CHECK_EQ(4, UISafeText::Utf8ToUtf32(text, decoded, 4));
	CHECK_EQ('A', decoded[0]);
	CHECK_EQ(0xC548, decoded[1]);
	CHECK_EQ(0x1F600, decoded[2]);
	CHECK_EQ('B', decoded[3]);

	CHECK(UISafeText::Utf8Prefix(text, 0).empty());
	CHECK(UISafeText::Utf8Prefix(text, 1) == std::string(text, 1));
	CHECK(UISafeText::Utf8Prefix(text, 2) == std::string(text, 4));
	CHECK(UISafeText::Utf8Prefix(text, 3) == std::string(text, 8));
	CHECK(UISafeText::Utf8Prefix(text, 4) == std::string(text, 9));
}


TEST(UISafeText, Utf8DecoderAndPrefixStopSafelyAtATruncatedTail)
{
	const char truncated[] = {
		'A', (char)0xE2, (char)0x82, '\0'
	};
	uint32_t decoded[2] = { 0, 0xFEEDBEEF };

	const size_t count = UISafeText::Utf8ToUtf32(truncated, decoded, 2);
	const std::string prefix = UISafeText::Utf8Prefix(truncated, 2);

	CHECK_EQ(1, count);
	CHECK_EQ('A', decoded[0]);
	CHECK_EQ(0xFEEDBEEF, decoded[1]);
	CHECK(prefix == std::string(truncated, sizeof(truncated)-1));
}


TEST(UISafeText, Utf8DecoderHonoursASmallOutputCapacity)
{
	struct GuardedOutput
	{
		uint32_t value;
		uint32_t sentinel;
	} output = { 0, 0xFEEDBEEF };

	const size_t count = UISafeText::Utf8ToUtf32("AB", &output.value, 1);

	CHECK_EQ(1, count);
	CHECK_EQ('A', output.value);
	CHECK_EQ(0xFEEDBEEF, output.sentinel);
}


TEST(UISafeText, Utf8PrefixUsesTheEditorDecodersMalformedInputRules)
{
	const char malformed[] = {
		(char)0x80,                 // orphan continuation: dropped
		(char)0xF8,                 // invalid lead: dropped
		(char)0xE2, (char)0x82,     // truncated sequence: dropped
		'A', '\0'
	};
	uint32_t decoded[4] = { 0, 0, 0, 0 };

	const size_t count = UISafeText::Utf8ToUtf32(malformed, decoded, 4);
	const std::string prefix = UISafeText::Utf8Prefix(malformed, 1);

	CHECK_EQ(1, count);
	CHECK_EQ('A', decoded[0]);
	CHECK(prefix == std::string(malformed, sizeof(malformed)-1));
}


TEST(UISafeText, Utf8PrefixDoesNotTruncateALongSupplementarySequence)
{
	std::string text;
	for (int i=0; i<1023; i++)
	{
		text.append("\xF0\x9F\x98\x80", 4);
	}

	const std::string prefix = UISafeText::Utf8Prefix(text.c_str(), 1023);

	CHECK_EQ(4092, prefix.size());
	CHECK(prefix == text);
}


//----------------------------------------------------------------------
// Tooltip prefixes can contain option names from the server. They and the
// base item names are kept byte-for-byte, while the fixed-width legacy grade
// token is appended only when both requested bytes exist in the table entry.
//----------------------------------------------------------------------
TEST(UISafeText, ItemTooltipNamesPreserveLongAndMultibyteText)
{
	std::string localPrefix(180, 'L');
	localPrefix.append("\xEC\x95\x88", 3);
	std::string englishPrefix("(");
	englishPrefix.append(180, 'E');

	const UISafeText::ItemTooltipNames names =
		UISafeText::FinishItemTooltipNames(localPrefix, englishPrefix,
			"local", "english", NULL, 0);

	CHECK_EQ(188, names.local.size());
	CHECK(names.local == localPrefix + "local");
	CHECK(names.english == englishPrefix + "english)");
}


TEST(UISafeText, ItemTooltipNamesAppendTheLegacyTwoByteGradeToken)
{
	std::string gradeTable(22, '_');
	gradeTable[20] = 'X';
	gradeTable[21] = 'Y';

	const UISafeText::ItemTooltipNames names =
		UISafeText::FinishItemTooltipNames("option ", "(option ",
			"Blade", "Blade", gradeTable.c_str(), 10);

	CHECK(names.local == "option BladeXY");
	CHECK(names.english == "(option BladeXY)");
}


TEST(UISafeText, ItemTooltipNamesRejectAnIncompleteGradeToken)
{
	const UISafeText::ItemTooltipNames names =
		UISafeText::FinishItemTooltipNames("", "(", "Blade", "Blade",
			"012345", 3);

	CHECK(names.local == "Blade");
	CHECK(names.english == "(Blade)");
}
