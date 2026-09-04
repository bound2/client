//----------------------------------------------------------------------
// test_spritesurface_pal_blit.cpp
//----------------------------------------------------------------------
//
// Tests for the SDL backend's palette sprite blits in
// Client/SpriteLib/CSpriteSurface_Adapter.cpp and the screen blend
// tables in Client/SpriteLib/CSpriteSurface_SDL.cpp.
//
// Two things went wrong on the effect path after the SDL port and both
// are pinned here:
//
//   - BltSpritePalEffect, the BLT_SCREEN effect blit, was an empty
//     stub, so every screen-blend skill effect drew nothing; and the
//     effect function table it selects from was all NULL, so even a
//     working blit would have plain-copied.
//   - BltAlphaSpritePal, the BLT_EFFECT blit, refused any sprite that
//     crossed a surface edge instead of clipping it.
//
// The member blits lock a backend surface, which a test binary does
// not have. They hand the locked pixels to the static *To functions,
// and those are what is driven here, against a small array with every
// pixel outside the expected footprint checked untouched.
//
// Sprite container layout (CSpritePalBase::LoadFromFile):
//
//      DWORD  size      bytes of packed pixel data
//      WORD   width
//      WORD   height
//      BYTE   data[size]
//      WORD   index[height]   per-scanline byte lengths
//
// and each scanline is run length encoded as a segment count, then
// per segment a transparent run, a colour run, and the colour run's
// pixels: one palette index byte for CSpritePal, an (alpha, index)
// pair for CAlphaSpritePal.
//
//----------------------------------------------------------------------
#include "test_framework.h"
#include "CSpriteSurface.h"
#include "CSpritePal.h"
#include "CAlphaSpritePal.h"
#include "MPalette.h"
#include "ColorDraw.h"
#include <cstdio>
#include <fstream>
#include <vector>

namespace {

const char* const	kTempFile		= "spritesurface_pal_blit_test.bin";
const int		kSpriteWidth	= 4;
const int		kSpriteHeight	= 2;

// A colour whose three channels are distinct and mid-range, so a
// channel put in the wrong place or scaled wrongly shows up.
const BYTE		kRed	= 10;
const BYTE		kGreen	= 40;
const BYTE		kBlue	= 20;

void	PushWord(std::vector<unsigned char>& bytes, WORD value)
{
	bytes.push_back((unsigned char)(value & 0xFF));
	bytes.push_back((unsigned char)((value >> 8) & 0xFF));
}

void	PushDword(std::vector<unsigned char>& bytes, DWORD value)
{
	PushWord(bytes, (WORD)(value & 0xFFFF));
	PushWord(bytes, (WORD)((value >> 16) & 0xFFFF));
}

//----------------------------------------------------------------------
// One fully opaque scanline of palette index 1, with or without the
// alpha byte an alpha sprite carries per pixel.
//----------------------------------------------------------------------
std::vector<unsigned char>	OpaqueScanline(bool withAlpha)
{
	std::vector<unsigned char>	scanline;

	scanline.push_back(1);						// one segment
	scanline.push_back(0);						// no transparent run
	scanline.push_back((unsigned char)kSpriteWidth);	// colour run

	for (int i = 0; i < kSpriteWidth; i++)
	{
		if (withAlpha)
			scanline.push_back(0x20);			// alpha 32: source replaces dest
		scanline.push_back(0x01);				// palette index
	}

	return scanline;
}

//----------------------------------------------------------------------
// Writes a kSpriteWidth x kSpriteHeight sprite whose every pixel is
// opaque palette index 1, and loads it into the sprite handed in.
//----------------------------------------------------------------------
template <class Sprite>
bool	LoadOpaqueSprite(Sprite& sprite, bool withAlpha)
{
	std::vector<unsigned char>	scanline = OpaqueScanline(withAlpha);
	std::vector<unsigned char>	bytes;

	PushDword(bytes, (DWORD)(scanline.size() * kSpriteHeight));
	PushWord(bytes, (WORD)kSpriteWidth);
	PushWord(bytes, (WORD)kSpriteHeight);

	for (int row = 0; row < kSpriteHeight; row++)
		bytes.insert(bytes.end(), scanline.begin(), scanline.end());

	for (int row = 0; row < kSpriteHeight; row++)
		PushWord(bytes, (WORD)scanline.size());

	{
		std::ofstream	out(kTempFile, std::ios::binary | std::ios::trunc);
		out.write((const char*)&bytes[0], (std::streamsize)bytes.size());
	}

	std::ifstream	in(kTempFile, std::ios::binary);
	bool		loaded = sprite.LoadFromFile(in);

	in.close();
	std::remove(kTempFile);

	return loaded;
}

//----------------------------------------------------------------------
// A palette with the test colour at index 1.
//----------------------------------------------------------------------
void	FillPalette(MPalette& pal)
{
	pal.Init(2);
	pal[0] = 0;
	pal[1] = ColorDraw::Color(kRed, kGreen, kBlue);
}

//----------------------------------------------------------------------
// A pixel buffer standing in for a locked surface, cleared to black.
// Footprint(x, y, w, h) says whether every pixel inside that rectangle
// holds the test colour and every pixel outside it is still black -
// the whole statement a clipped blit has to satisfy.
//----------------------------------------------------------------------
class Surface
{
public:
	Surface(int width, int height)
		: m_Width(width), m_Height(height), m_Pixels((size_t)(width * height), 0) {}

	void	Fill(WORD value)	{ m_Pixels.assign(m_Pixels.size(), value); }

	WORD*	Pixels()		{ return &m_Pixels[0]; }
	int	Pitch() const		{ return m_Width * (int)sizeof(WORD); }
	int	Width() const		{ return m_Width; }
	int	Height() const		{ return m_Height; }
	WORD	At(int x, int y) const	{ return m_Pixels[(size_t)(y * m_Width + x)]; }

	bool	FootprintIs(int left, int top, int width, int height,
				WORD inside = ColorDraw::Color(kRed, kGreen, kBlue), WORD outside = 0) const
	{
		for (int y = 0; y < m_Height; y++)
		{
			for (int x = 0; x < m_Width; x++)
			{
				bool	covered = (x >= left && x < left + width && y >= top && y < top + height);
				WORD	expected = covered ? inside : outside;

				if (At(x, y) != expected)
					return false;
			}
		}
		return true;
	}

	bool	IsBlack() const	{ return FootprintIs(0, 0, 0, 0); }

private:
	int			m_Width;
	int			m_Height;
	std::vector<WORD>	m_Pixels;
};

void	ScreenBlit(Surface& surface, int x, int y, CSpritePal& sprite, MPalette& pal)
{
	POINT	point;
	point.x = x;
	point.y = y;
	CSpriteSurface::SetPalEffect(CSpriteSurface::EFFECT_SCREEN);
	CSpriteSurface::BltSpritePalEffectTo(surface.Pixels(), surface.Pitch(),
										surface.Width(), surface.Height(),
										&point, &sprite, pal);
}

void	AlphaBlit(Surface& surface, int x, int y, CAlphaSpritePal& sprite, MPalette& pal)
{
	POINT	point;
	point.x = x;
	point.y = y;
	CSpriteSurface::BltAlphaSpritePalTo(surface.Pixels(), surface.Pitch(),
										surface.Width(), surface.Height(),
										&point, &sprite, pal);
}

} // namespace

//======================================================================
// ClipSpriteToSurface
//======================================================================

//----------------------------------------------------------------------
// A sprite that fits needs no clipping: the whole sprite rectangle,
// drawn at the point given.
//----------------------------------------------------------------------
TEST(ClipSpriteToSurface, WhollyVisibleSpriteIsNotClipped)
{
	RECT	rect;
	POINT	dest;

	CHECK_EQ((int)CSpriteSurface::SPRITE_CLIP_NONE,
		(int)CSpriteSurface::ClipSpriteToSurface(1, 1, 4, 2, 6, 4, &rect, &dest));
	CHECK_EQ(0, rect.left);
	CHECK_EQ(0, rect.top);
	CHECK_EQ(4, rect.right);
	CHECK_EQ(2, rect.bottom);
	CHECK_EQ(1, dest.x);
	CHECK_EQ(1, dest.y);
}

//----------------------------------------------------------------------
// Off the left edge: rect.left is how many source columns to skip and
// the destination is the surface's first column.
//----------------------------------------------------------------------
TEST(ClipSpriteToSurface, LeftOverhangSkipsSourceColumns)
{
	RECT	rect;
	POINT	dest;

	CHECK_EQ((int)CSpriteSurface::SPRITE_CLIP_LEFT,
		(int)CSpriteSurface::ClipSpriteToSurface(-3, 1, 4, 2, 6, 4, &rect, &dest));
	CHECK_EQ(3, rect.left);
	CHECK_EQ(4, rect.right);
	CHECK_EQ(0, dest.x);
	CHECK_EQ(1, dest.y);
}

//----------------------------------------------------------------------
// Off the right edge: rect.right is the visible width.
//----------------------------------------------------------------------
TEST(ClipSpriteToSurface, RightOverhangShortensTheRow)
{
	RECT	rect;
	POINT	dest;

	CHECK_EQ((int)CSpriteSurface::SPRITE_CLIP_RIGHT,
		(int)CSpriteSurface::ClipSpriteToSurface(4, 0, 4, 2, 6, 4, &rect, &dest));
	CHECK_EQ(0, rect.left);
	CHECK_EQ(2, rect.right);
	CHECK_EQ(4, dest.x);
}

//----------------------------------------------------------------------
// Wider than the surface: both edges cut.
//----------------------------------------------------------------------
TEST(ClipSpriteToSurface, SpriteWiderThanSurfaceIsCutOnBothSides)
{
	RECT	rect;
	POINT	dest;

	CHECK_EQ((int)CSpriteSurface::SPRITE_CLIP_WIDTH,
		(int)CSpriteSurface::ClipSpriteToSurface(-1, 0, 4, 2, 2, 4, &rect, &dest));
	CHECK_EQ(1, rect.left);
	CHECK_EQ(3, rect.right);
	CHECK_EQ(0, dest.x);
}

//----------------------------------------------------------------------
// Rows only: a sprite hanging off the top or the bottom keeps every
// column, so the row-only variant is chosen, and the destination row
// is the first visible one.
//----------------------------------------------------------------------
TEST(ClipSpriteToSurface, VerticalOverhangClipsRowsOnly)
{
	RECT	rect;
	POINT	dest;

	CHECK_EQ((int)CSpriteSurface::SPRITE_CLIP_HEIGHT,
		(int)CSpriteSurface::ClipSpriteToSurface(1, -1, 4, 2, 6, 4, &rect, &dest));
	CHECK_EQ(1, rect.top);
	CHECK_EQ(2, rect.bottom);
	CHECK_EQ(0, rect.left);
	CHECK_EQ(4, rect.right);
	CHECK_EQ(1, dest.x);
	CHECK_EQ(0, dest.y);

	CHECK_EQ((int)CSpriteSurface::SPRITE_CLIP_HEIGHT,
		(int)CSpriteSurface::ClipSpriteToSurface(1, 3, 4, 2, 6, 4, &rect, &dest));
	CHECK_EQ(0, rect.top);
	CHECK_EQ(1, rect.bottom);
	CHECK_EQ(3, dest.y);
}

//----------------------------------------------------------------------
// A corner: columns and rows both cut, reported as the column case,
// which walks only the visible rows anyway.
//----------------------------------------------------------------------
TEST(ClipSpriteToSurface, CornerOverhangClipsBothAxes)
{
	RECT	rect;
	POINT	dest;

	CHECK_EQ((int)CSpriteSurface::SPRITE_CLIP_LEFT,
		(int)CSpriteSurface::ClipSpriteToSurface(-1, -1, 4, 2, 6, 4, &rect, &dest));
	CHECK_EQ(1, rect.left);
	CHECK_EQ(1, rect.top);
	CHECK_EQ(4, rect.right);
	CHECK_EQ(2, rect.bottom);
	CHECK_EQ(0, dest.x);
	CHECK_EQ(0, dest.y);
}

//----------------------------------------------------------------------
// Nothing visible: past any edge, touching an edge from outside, or
// with no area at all.
//----------------------------------------------------------------------
TEST(ClipSpriteToSurface, InvisibleSpriteIsOutside)
{
	RECT	rect;
	POINT	dest;

	CHECK_EQ((int)CSpriteSurface::SPRITE_CLIP_OUTSIDE,
		(int)CSpriteSurface::ClipSpriteToSurface(6, 0, 4, 2, 6, 4, &rect, &dest));
	CHECK_EQ((int)CSpriteSurface::SPRITE_CLIP_OUTSIDE,
		(int)CSpriteSurface::ClipSpriteToSurface(0, 4, 4, 2, 6, 4, &rect, &dest));
	CHECK_EQ((int)CSpriteSurface::SPRITE_CLIP_OUTSIDE,
		(int)CSpriteSurface::ClipSpriteToSurface(-4, 0, 4, 2, 6, 4, &rect, &dest));
	CHECK_EQ((int)CSpriteSurface::SPRITE_CLIP_OUTSIDE,
		(int)CSpriteSurface::ClipSpriteToSurface(0, -2, 4, 2, 6, 4, &rect, &dest));
	CHECK_EQ((int)CSpriteSurface::SPRITE_CLIP_OUTSIDE,
		(int)CSpriteSurface::ClipSpriteToSurface(0, 0, 0, 2, 6, 4, &rect, &dest));
	CHECK_EQ((int)CSpriteSurface::SPRITE_CLIP_OUTSIDE,
		(int)CSpriteSurface::ClipSpriteToSurface(0, 0, 4, 2, 0, 0, &rect, &dest));
}

//======================================================================
// The screen blend
//======================================================================

//----------------------------------------------------------------------
// screen(d, s) = 1 - (1 - d)(1 - s): black leaves the source as it is,
// white stays white, and the operation is symmetric. Green is the
// 6-bit field, so its table has to answer for a value of 63.
//----------------------------------------------------------------------
TEST(ScreenBlend, TableFollowsTheScreenFormula)
{
	CSpriteSurface::InitEffectTable();

	CHECK_EQ((WORD)(17 << ColorDraw::s_bSHIFT_R), CSpriteSurface::s_EffectScreenTableR[0][17]);
	CHECK_EQ((WORD)(31 << ColorDraw::s_bSHIFT_R), CSpriteSurface::s_EffectScreenTableR[31][17]);
	CHECK_EQ(CSpriteSurface::s_EffectScreenTableR[9][23], CSpriteSurface::s_EffectScreenTableR[23][9]);

	CHECK_EQ((WORD)17, CSpriteSurface::s_EffectScreenTableB[0][17]);
	CHECK_EQ((WORD)31, CSpriteSurface::s_EffectScreenTableB[31][17]);

	CHECK_EQ((WORD)(63 << ColorDraw::s_bSHIFT_G), CSpriteSurface::s_EffectScreenTableG[0][63]);
	CHECK_EQ((WORD)(63 << ColorDraw::s_bSHIFT_G), CSpriteSurface::s_EffectScreenTableG[63][40]);
	CHECK_EQ(CSpriteSurface::s_EffectScreenTableG[12][50], CSpriteSurface::s_EffectScreenTableG[50][12]);

	// 16 over 16 with a maximum of 32: 16 + 16 * 16 / 32 = 24
	CHECK_EQ((WORD)(24 << ColorDraw::s_bSHIFT_R), CSpriteSurface::s_EffectScreenTableR[16][16]);
	// 32 over 32 with a maximum of 64: 32 + 32 * 32 / 64 = 48
	CHECK_EQ((WORD)(48 << ColorDraw::s_bSHIFT_G), CSpriteSurface::s_EffectScreenTableG[32][32]);
}

//----------------------------------------------------------------------
// EFFECT_SCREEN is registered, so selecting it blends rather than
// copying: over black the palette colour comes through exactly, over
// white the pixel stays white.
//----------------------------------------------------------------------
TEST(ScreenBlend, SelectingScreenBlendsThePixel)
{
	CSpriteSurface::InitEffectTable();
	CSpriteSurface::SetPalEffect(CSpriteSurface::EFFECT_SCREEN);

	MPalette	pal;
	FillPalette(pal);

	BYTE	source[2] = { 1, 1 };
	WORD	dest[2] = { 0, 0xFFFF };

	CSpriteSurface::memcpyPalEffect(dest, source, 2, pal);

	CHECK_EQ(ColorDraw::Color(kRed, kGreen, kBlue), dest[0]);
	CHECK_EQ((WORD)0xFFFF, dest[1]);
}

//======================================================================
// BltSpritePalEffectTo - the BLT_SCREEN effect path
//======================================================================

TEST(BltSpritePalEffectTo, DrawsTheWholeSpriteWhenItFits)
{
	CSpriteSurface::InitEffectTable();

	CSpritePal	sprite;
	CHECK(LoadOpaqueSprite(sprite, false));

	MPalette	pal;
	FillPalette(pal);

	Surface		surface(6, 4);
	ScreenBlit(surface, 1, 1, sprite, pal);

	CHECK(surface.FootprintIs(1, 1, kSpriteWidth, kSpriteHeight));
}

//----------------------------------------------------------------------
// Over a black surface the screen blend and a plain copy produce the
// same bytes, so the footprint tests above cannot tell them apart.
// This one draws over a grey and asserts the blended value: per
// channel, high + low * (max - high) / max, with red and blue out of
// 32 and green out of 64. A plain copy would leave the palette colour.
//----------------------------------------------------------------------
TEST(BltSpritePalEffectTo, BlendsOverANonBlackSurface)
{
	CSpriteSurface::InitEffectTable();

	CSpritePal	sprite;
	CHECK(LoadOpaqueSprite(sprite, false));

	MPalette	pal;
	FillPalette(pal);

	const WORD	grey = ColorDraw::Color(16, 32, 16);

	// red:   16 + 10 * (32 - 16) / 32 = 21
	// green: 40 + 32 * (64 - 40) / 64 = 52
	// blue:  20 + 16 * (32 - 20) / 32 = 26
	const WORD	blended = ColorDraw::Color(21, 52, 26);

	Surface		surface(6, 4);
	surface.Fill(grey);
	ScreenBlit(surface, 1, 1, sprite, pal);

	CHECK(blended != ColorDraw::Color(kRed, kGreen, kBlue));
	CHECK(surface.FootprintIs(1, 1, kSpriteWidth, kSpriteHeight, blended, grey));
}

//----------------------------------------------------------------------
// Off the top-left corner, only the overlapping cells are written,
// and the sprite's own top-left column and row are the ones skipped.
//----------------------------------------------------------------------
TEST(BltSpritePalEffectTo, ClipsAtTheTopLeftCorner)
{
	CSpriteSurface::InitEffectTable();

	CSpritePal	sprite;
	CHECK(LoadOpaqueSprite(sprite, false));

	MPalette	pal;
	FillPalette(pal);

	Surface		surface(6, 4);
	ScreenBlit(surface, -1, -1, sprite, pal);

	CHECK(surface.FootprintIs(0, 0, kSpriteWidth - 1, kSpriteHeight - 1));
}

TEST(BltSpritePalEffectTo, ClipsAtTheBottomRightCorner)
{
	CSpriteSurface::InitEffectTable();

	CSpritePal	sprite;
	CHECK(LoadOpaqueSprite(sprite, false));

	MPalette	pal;
	FillPalette(pal);

	Surface		surface(6, 4);
	ScreenBlit(surface, 4, 3, sprite, pal);

	CHECK(surface.FootprintIs(4, 3, 2, 1));
}

//----------------------------------------------------------------------
// Wider than the surface: the middle of the sprite is what shows.
//----------------------------------------------------------------------
TEST(BltSpritePalEffectTo, ClipsBothSidesOfAWideSprite)
{
	CSpriteSurface::InitEffectTable();

	CSpritePal	sprite;
	CHECK(LoadOpaqueSprite(sprite, false));

	MPalette	pal;
	FillPalette(pal);

	Surface		surface(2, 4);
	ScreenBlit(surface, -1, 1, sprite, pal);

	CHECK(surface.FootprintIs(0, 1, 2, kSpriteHeight));
}

//----------------------------------------------------------------------
// Hanging off the top only: every column, the visible rows.
//----------------------------------------------------------------------
TEST(BltSpritePalEffectTo, ClipsRowsOnly)
{
	CSpriteSurface::InitEffectTable();

	CSpritePal	sprite;
	CHECK(LoadOpaqueSprite(sprite, false));

	MPalette	pal;
	FillPalette(pal);

	Surface		surface(6, 4);
	ScreenBlit(surface, 1, -1, sprite, pal);

	CHECK(surface.FootprintIs(1, 0, kSpriteWidth, kSpriteHeight - 1));
}

TEST(BltSpritePalEffectTo, DrawsNothingOutsideOrUninitialised)
{
	CSpriteSurface::InitEffectTable();

	CSpritePal	sprite;
	CHECK(LoadOpaqueSprite(sprite, false));

	MPalette	pal;
	FillPalette(pal);

	Surface		surface(6, 4);
	ScreenBlit(surface, 6, 0, sprite, pal);
	ScreenBlit(surface, 0, 4, sprite, pal);
	ScreenBlit(surface, -4, 0, sprite, pal);
	CHECK(surface.IsBlack());

	CSpritePal	empty;
	ScreenBlit(surface, 1, 1, empty, pal);
	CHECK(surface.IsBlack());
}

//======================================================================
// BltAlphaSpritePalTo - the BLT_EFFECT path
//======================================================================

TEST(BltAlphaSpritePalTo, DrawsTheWholeSpriteWhenItFits)
{
	CAlphaSpritePal	sprite;
	CHECK(LoadOpaqueSprite(sprite, true));

	MPalette	pal;
	FillPalette(pal);

	Surface		surface(6, 4);
	AlphaBlit(surface, 1, 1, sprite, pal);

	CHECK(surface.FootprintIs(1, 1, kSpriteWidth, kSpriteHeight));
}

//----------------------------------------------------------------------
// The case the log showed: a sprite hanging off the top of the screen
// used to be skipped whole. Its visible rows draw now.
//----------------------------------------------------------------------
TEST(BltAlphaSpritePalTo, SpriteHangingOffTheTopDrawsItsVisibleRows)
{
	CAlphaSpritePal	sprite;
	CHECK(LoadOpaqueSprite(sprite, true));

	MPalette	pal;
	FillPalette(pal);

	Surface		surface(6, 4);
	AlphaBlit(surface, 1, -1, sprite, pal);

	CHECK(surface.FootprintIs(1, 0, kSpriteWidth, kSpriteHeight - 1));
}

TEST(BltAlphaSpritePalTo, ClipsAtTheTopLeftCorner)
{
	CAlphaSpritePal	sprite;
	CHECK(LoadOpaqueSprite(sprite, true));

	MPalette	pal;
	FillPalette(pal);

	Surface		surface(6, 4);
	AlphaBlit(surface, -1, -1, sprite, pal);

	CHECK(surface.FootprintIs(0, 0, kSpriteWidth - 1, kSpriteHeight - 1));
}

TEST(BltAlphaSpritePalTo, ClipsAtTheBottomRightCorner)
{
	CAlphaSpritePal	sprite;
	CHECK(LoadOpaqueSprite(sprite, true));

	MPalette	pal;
	FillPalette(pal);

	Surface		surface(6, 4);
	AlphaBlit(surface, 4, 3, sprite, pal);

	CHECK(surface.FootprintIs(4, 3, 2, 1));
}

TEST(BltAlphaSpritePalTo, ClipsBothSidesOfAWideSprite)
{
	CAlphaSpritePal	sprite;
	CHECK(LoadOpaqueSprite(sprite, true));

	MPalette	pal;
	FillPalette(pal);

	Surface		surface(2, 4);
	AlphaBlit(surface, -1, 1, sprite, pal);

	CHECK(surface.FootprintIs(0, 1, 2, kSpriteHeight));
}

TEST(BltAlphaSpritePalTo, DrawsNothingOutsideOrUninitialised)
{
	CAlphaSpritePal	sprite;
	CHECK(LoadOpaqueSprite(sprite, true));

	MPalette	pal;
	FillPalette(pal);

	Surface		surface(6, 4);
	AlphaBlit(surface, 6, 0, sprite, pal);
	AlphaBlit(surface, 0, 4, sprite, pal);
	AlphaBlit(surface, 0, -2, sprite, pal);
	CHECK(surface.IsBlack());

	CAlphaSpritePal	empty;
	AlphaBlit(surface, 1, 1, empty, pal);
	CHECK(surface.IsBlack());
}
