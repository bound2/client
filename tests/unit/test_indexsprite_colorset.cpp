//----------------------------------------------------------------------
// test_indexsprite_colorset.cpp
//----------------------------------------------------------------------
//
// Regression coverage for the generated RGB 5:6:5 colour-set table.
// The shipped item-option table assigns colour set 377 to items without
// an option. That set is generated from the light-grey seed, so every
// gradation must remain neutral: red and blue match, while green carries
// twice their value because its RGB565 field has six bits instead of five.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "CIndexSprite.h"
#include "ColorDraw.h"

#include <cstdlib>

namespace {

struct ColorSeed
{
	int	red;
	int	green;
	int	blue;
};

const ColorSeed kRgb565Seeds[] =
{
	{ 16, 32, 16 },
	{ 24, 48, 24 },
	{ 8, 16, 8 },
	{ 30, 48, 18 },
	{ 25, 30, 11 },
	{ 21, 24, 11 },
	{ 19, 30, 13 },
	{ 21, 36, 11 },
	{ 22, 32, 9 }
};

} // namespace

TEST(CIndexSpriteColorSet, DefaultItemColorSetIsNeutralGrey)
{
	const int defaultItemColorSet = 377;

	CIndexSprite::SetColorSet();

	for (int gradation = 0; gradation < MAX_COLORGRADATION; gradation++)
	{
		const WORD color = CIndexSprite::ColorSet[defaultItemColorSet][gradation];
		const int red = ColorDraw::Red(color);
		const int green = ColorDraw::Green(color);
		const int blue = ColorDraw::Blue(color);

		CHECK_EQ(red, blue);
		CHECK(std::abs(green - red * 2) <= 1);
	}
}

//----------------------------------------------------------------------
// These seeds came from a 5:5:5 table. RGB565 doubles their green
// component; pinning the generated seed colour prevents a source-data
// restoration from silently copying the old numeric values back again.
//----------------------------------------------------------------------
TEST(CIndexSpriteColorSet, LegacySeedsAreScaledForRgb565)
{
	const int firstSeed = 24;

	CIndexSprite::SetColorSet();

	for (int index = 0; index < (int)(sizeof(kRgb565Seeds) / sizeof(kRgb565Seeds[0])); index++)
	{
		const int colorSet = (firstSeed + index) * MAX_COLORSET_SEED_MODIFY;
		const WORD seedColor = CIndexSprite::ColorSet[colorSet][MAX_COLORGRADATION_HALF];
		const ColorSeed& expected = kRgb565Seeds[index];

		CHECK_EQ(expected.red, ColorDraw::Red(seedColor));
		CHECK_EQ(expected.green, ColorDraw::Green(seedColor));
		CHECK_EQ(expected.blue, ColorDraw::Blue(seedColor));
	}
}
