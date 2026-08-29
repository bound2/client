//----------------------------------------------------------------------
// test_colordraw.cpp
//----------------------------------------------------------------------
//
// Tests for the RGB 5:6:5 / 5:5:5 conversion helpers in basic/ColorDraw.h.
//
// Channel layout:
//
//      5:6:5   R = bits 11-15 (5 bits)
//              G = bits  5-10 (6 bits)
//              B = bits  0-4  (5 bits)
//
//      5:5:5   R = bits 10-14 (5 bits)
//              G = bits  5-9  (5 bits)
//              B = bits  0-4  (5 bits)
//
// Converting 5:6:5 to 5:5:5 therefore moves red down one bit, drops the
// least significant green bit, and leaves blue exactly where it is.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "ColorDraw.h"

namespace {

//----------------------------------------------------------------------
// Component accessors for a 5:5:5 pixel. ColorDraw's own Red/Green/Blue
// helpers decode 5:6:5, so they cannot be used to inspect the result.
//----------------------------------------------------------------------
inline int	Red555(WORD pixel)	{ return (pixel >> 10) & 0x1F; }
inline int	Green555(WORD pixel)	{ return (pixel >> 5) & 0x1F; }
inline int	Blue555(WORD pixel)	{ return pixel & 0x1F; }

//----------------------------------------------------------------------
// Builds a 5:6:5 pixel from its components.
//----------------------------------------------------------------------
inline WORD	Make565(int r, int g, int b)
{
	return (WORD)(((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F));
}

} // namespace

//----------------------------------------------------------------------
// Blue occupies the same bits in both formats, so it must survive the
// conversion untouched.
//----------------------------------------------------------------------
TEST(ColorDraw, Convert565to555PreservesBlue)
{
	CHECK_EQ(0x1F, Blue555(ColorDraw::Convert565to555(Make565(0, 0, 0x1F))));
	CHECK_EQ(0x15, Blue555(ColorDraw::Convert565to555(Make565(0, 0, 0x15))));
	CHECK_EQ(0x01, Blue555(ColorDraw::Convert565to555(Make565(0, 0, 0x01))));
}

//----------------------------------------------------------------------
// Green loses its least significant bit going from 6 bits to 5, and must
// not bleed into the blue field below it.
//----------------------------------------------------------------------
TEST(ColorDraw, Convert565to555TruncatesGreenWithoutBleeding)
{
	CHECK_EQ(0x1F, Green555(ColorDraw::Convert565to555(Make565(0, 0x3F, 0))));
	CHECK_EQ(0x00, Blue555(ColorDraw::Convert565to555(Make565(0, 0x3F, 0))));

	// Green 0x01 is the bit that gets dropped; it must not appear in blue.
	CHECK_EQ(0x00, Green555(ColorDraw::Convert565to555(Make565(0, 0x01, 0))));
	CHECK_EQ(0x00, Blue555(ColorDraw::Convert565to555(Make565(0, 0x01, 0))));
}

//----------------------------------------------------------------------
// Red keeps all five bits, shifted down one position.
//----------------------------------------------------------------------
TEST(ColorDraw, Convert565to555PreservesRed)
{
	CHECK_EQ(0x1F, Red555(ColorDraw::Convert565to555(Make565(0x1F, 0, 0))));
	CHECK_EQ(0x0A, Red555(ColorDraw::Convert565to555(Make565(0x0A, 0, 0))));
}

//----------------------------------------------------------------------
// 5:5:5 -> 5:6:5 -> 5:5:5 must be an identity. Widening green appends a
// zero bit and narrowing it removes that same bit again, so no
// information is lost in the round trip.
//----------------------------------------------------------------------
TEST(ColorDraw, RoundTripThrough565IsLossless)
{
	for (int r = 0; r < 32; r += 7)
	{
		for (int g = 0; g < 32; g += 7)
		{
			for (int b = 0; b < 32; b += 7)
			{
				const WORD original =
					(WORD)((r << 10) | (g << 5) | b);

				const WORD roundTripped =
					ColorDraw::Convert565to555(
						ColorDraw::Convert555to565(original));

				CHECK_EQ(original, roundTripped);
			}
		}
	}
}

//----------------------------------------------------------------------
// A fully saturated pixel exercises every field at once.
//----------------------------------------------------------------------
TEST(ColorDraw, Convert565to555HandlesWhite)
{
	const WORD white555 = ColorDraw::Convert565to555(Make565(0x1F, 0x3F, 0x1F));

	CHECK_EQ(0x1F, Red555(white555));
	CHECK_EQ(0x1F, Green555(white555));
	CHECK_EQ(0x1F, Blue555(white555));
}
