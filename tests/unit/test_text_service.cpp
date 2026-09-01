//----------------------------------------------------------------------
// test_text_service.cpp
//----------------------------------------------------------------------
//
// Regression guards for the accept condition inside ConvertEncoding
// (Client/TextSystem/TextService.cpp), which decides whether a candidate
// code page "explains" a string.
//
// The defect these cover was that a conversion which stopped part way
// through counted as a success, so NormalizeText took the converted prefix
// and dropped the rest. It is already fixed - 43458c1 moved the function
// off SDL_iconv and added the `inBytes != 0` test that rejects a partial
// run - so nothing here reproduces a live bug.
//
// What they guard is NormalizeText's observable contract: a code page that
// cannot explain the whole string must not claim it, and the bytes must
// come back unchanged when none can. They do NOT guard the `inBytes != 0`
// half of the accept condition specifically, and an earlier version of this
// comment wrongly claimed they did. That was measured and disproved: a
// conforming iconv returns a non-negative count only when it has consumed
// all of its input, so `inBytes != 0` implies `res == (size_t)-1` and the
// two halves cannot be told apart by any input. Deleting either one leaves
// every test below green. The clause is worth keeping as a guard against a
// non-conforming iconv, but nothing here would catch its removal - if you
// want that, it needs a stubbed iconv, not a byte vector.
//
// tests/unit/test_textservice_normalize.cpp covers the conversions that are
// meant to succeed; this file covers the ones that must not. Its vectors
// are real bytes from Data/Info/NPCScript.inf and the ones below are the
// first two syllables of the same name, so the two files can be read
// against each other.
//
// Note on the fallback: when no candidate accepts the bytes, NormalizeText
// returns its input unchanged rather than an empty string, and the caller
// draws it through Utf8Decode, which renders an unrecognised byte as U+FFFD.
// So a stricter accept condition costs replacement glyphs, never a blank
// line - and the last test pins that down, because "return empty on
// failure" is the tempting simplification that would turn text the user can
// at least see into nothing at all.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "TextService.h"

#include <string>

namespace {

//----------------------------------------------------------------------
// Two Hangul syllables in CP949, and the UTF-8 they have to become.
//----------------------------------------------------------------------
const char CP949_TWO_SYLLABLES[] = "\xB1\xD7\xB7\xE7";
const char UTF8_TWO_SYLLABLES[]  = "\xEA\xB7\xB8\xEB\xA3\xA8";

//----------------------------------------------------------------------
// The UTF-8 for the first syllable alone: the prefix a truncated
// conversion of either vector below would hand back if a partial run were
// accepted as success.
//----------------------------------------------------------------------
const char UTF8_FIRST_SYLLABLE[] = "\xEA\xB7\xB8";

std::string Normalize(const std::string& in)
{
	return TextSystem::TextService::NormalizeText(in);
}

} // namespace

//----------------------------------------------------------------------
// Positive control. Without it the rejection tests below could pass
// against a ConvertEncoding that converts nothing at all.
//----------------------------------------------------------------------
TEST(TextServiceEncodingGuard, WhollyValidCp949StillConverts)
{
	CHECK(std::string(UTF8_TWO_SYLLABLES) == Normalize(std::string(CP949_TWO_SYLLABLES)));
}

//----------------------------------------------------------------------
// A dangling lead byte at the end - iconv's EINVAL. 0xB7 opens a
// double-byte sequence in every candidate code page (CP949, EUC-KR, GBK,
// GB2312 and BIG5 all take it as a lead), so every one of them runs out of
// input, and none may claim the first syllable it decoded on the way.
//----------------------------------------------------------------------
TEST(TextServiceEncodingGuard, TruncatedTailIsNotAcceptedAsAPrefix)
{
	// Three of the four bytes: the trail byte of the second syllable is gone.
	const std::string truncated(CP949_TWO_SYLLABLES, 3);

	CHECK(std::string(UTF8_FIRST_SYLLABLE) != Normalize(truncated));
	CHECK(truncated == Normalize(truncated));
}

//----------------------------------------------------------------------
// An unmappable byte in the middle - iconv's EILSEQ. 0xFF is outside the
// lead-byte range of all five candidates, so the whole string has to be
// rejected rather than cut off at the bad byte.
//----------------------------------------------------------------------
TEST(TextServiceEncodingGuard, UnmappableByteRejectsTheWholeString)
{
	const std::string spliced =
		std::string(CP949_TWO_SYLLABLES) + "\xFF" + CP949_TWO_SYLLABLES;

	CHECK(std::string(UTF8_TWO_SYLLABLES) != Normalize(spliced));
	CHECK(spliced == Normalize(spliced));
}

//----------------------------------------------------------------------
// The fallback contract. Text no code page explains still has to come back
// as bytes to draw; returning nothing would erase the line instead of
// showing it as replacement glyphs.
//----------------------------------------------------------------------
TEST(TextServiceEncodingGuard, UnrecognisedTextFallsBackToTheInput)
{
	const std::string garbage = "\xFF\xFE\xFF";

	CHECK(!Normalize(garbage).empty());
	CHECK(garbage == Normalize(garbage));
}
