//----------------------------------------------------------------------
// test_mstring_format.cpp
//----------------------------------------------------------------------
//
// MString::FormatChecked (Client/MString.h) - the checked sibling of
// MString::Format, added by task 5.4's fifth slice
// (docs/code-health-review-2026-08-29.md finding C19).
//
// MString::Format is an ordinary varargs printf reached as a method,
// which is why every pattern in this repo that hunts for a data-file
// format string walked past it: R3, R7 and the format_arity audit all
// match on a printf's NAME. Three call sites in Client/GameUI.cpp were
// handing it a String.inf entry as the format, and the vsnprintf inside
// it bounds the write but not the read - one %s more than the call site
// passed still makes the CRT take a stack word as a char* and copy from
// wherever that points.
//
// MString is in gamemodel, so unlike the twenty-four VS_UI sites the
// same slice converted, this half of the fix has a test path at all.
// That is the reason the checked entry point went here rather than into
// a local buffer at each call site.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "MString.h"

#include <cstring>


namespace {

bool
Is(const char* expected, const MString& actual)
{
	return actual.GetString() != NULL
		&& std::strcmp(actual.GetString(), expected) == 0;
}

} // anonymous namespace


TEST(MStringFormatChecked, PrintsWhatFormatPrintedForAnEntryThatMatchesItsCallSite)
{
	MString msg;

	// The three converted call sites, in the shape they actually have:
	// a zone name, and for the third a zone name and a count.
	msg.FormatChecked("The %s lair has opened.", "Silent");
	CHECK(Is("The Silent lair has opened.", msg));

	msg.FormatChecked("%d minutes left to enter %s.", 5, "the lair");
	CHECK(Is("5 minutes left to enter the lair.", msg));
}

TEST(MStringFormatChecked, PrintsAConversionWithNoArgumentAsText)
{
	MString msg;

	// The primitive of finding C19: an entry carrying one conversion
	// more than the call site passes. Through Format() this reads a
	// stack word as a char*.
	msg.FormatChecked("The %s lair has opened. %s");

	CHECK(Is("The %s lair has opened. %s", msg));
}

TEST(MStringFormatChecked, LeavesTheStringUsableWhenTheEntryIsMissing)
{
	MString msg;

	// GetGameString answers "" for an id the table does not hold, and
	// the subscript answers a default MString whose GetString() is NULL.
	// Both reach this method at the converted call sites, so neither may
	// leave the result unreadable.
	msg.FormatChecked("");
	CHECK(Is("", msg));

	msg.FormatChecked(NULL);
	CHECK(Is("", msg));
}

TEST(MStringFormatChecked, RefusesAConversionThatWouldWriteThroughItsArgument)
{
	MString	msg;
	int		counter = 4242;

	msg.FormatChecked("opened%n", &counter);

	CHECK(Is("opened%n", msg));
	CHECK_EQ(4242, counter);
}

TEST(MStringFormatChecked, LeavesTheLengthAgreeingWithTheText)
{
	MString msg;

	// FormatChecked assigns through operator=, so the recorded length
	// has to come from the formatted text rather than from the format.
	msg.FormatChecked("%s", "abcdef");

	CHECK_EQ(6, (int)msg.GetLength());

	msg.FormatChecked("%s");

	CHECK_EQ(2, (int)msg.GetLength());
}
