//----------------------------------------------------------------------
// UISafeText.h
//----------------------------------------------------------------------
//
// Pure byte-preserving preparation for UI text. Game data mixes UTF-8 and
// legacy encodings, so these helpers do not normalize or reinterpret strings.
// They only establish safe storage and the boundaries the line editor and
// item tooltip already mean to use.
//
//----------------------------------------------------------------------

#ifndef __UI_SAFE_TEXT_H__
#define __UI_SAFE_TEXT_H__

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <string>


namespace UISafeText {

namespace Detail {

//----------------------------------------------------------------------
// Decode one unit with the line editor's established rules. Standalone
// continuation bytes and 0xF8..0xFF are dropped. A lead byte followed by too
// few continuation bytes is dropped together with the continuation bytes it
// consumed. Structurally complete sequences are accepted without adding new
// Unicode normalization or scalar-value restrictions.
//----------------------------------------------------------------------
inline bool
DecodeNext(const unsigned char*& p, uint32_t& c)
{
	const unsigned char b = *p++;
	int nNeed = 0;

	if (b < 0x80)
	{
		c = b;
		return true;
	}
	else if ((b >> 5) == 0x6)
	{
		c = b & 0x1F;
		nNeed = 1;
	}
	else if ((b >> 4) == 0xE)
	{
		c = b & 0x0F;
		nNeed = 2;
	}
	else if ((b >> 3) == 0x1E)
	{
		c = b & 0x07;
		nNeed = 3;
	}
	else
	{
		return false;
	}

	while (nNeed>0 && (*p & 0xC0)==0x80)
	{
		c = (c << 6) | (*p++ & 0x3F);
		--nNeed;
	}

	return nNeed == 0;
}

} // namespace Detail


//----------------------------------------------------------------------
// The decoder used by LineEditor input. Keeping it beside Utf8Prefix makes
// the cursor's character count follow the exact same acceptance rules.
//----------------------------------------------------------------------
inline size_t
Utf8ToUtf32(const char* pText, uint32_t* pOut, size_t nCapacity)
{
	if (pText==NULL || pOut==NULL)
	{
		return 0;
	}

	const unsigned char* p = (const unsigned char*)pText;
	size_t nOut = 0;

	while (*p!='\0' && nOut<nCapacity)
	{
		uint32_t c = 0;

		if (Detail::DecodeNext(p, c))
		{
			pOut[nOut++] = c;
		}
	}

	return nOut;
}


//----------------------------------------------------------------------
// Password rendering historically displays one '*' for every stored UTF-8
// byte, not every decoded character. Preserve that visible and byte-limit
// contract, but use dynamic storage so a four-byte character cannot make a
// fixed 1,024-byte local buffer overflow.
//----------------------------------------------------------------------
inline std::string
MakePasswordMask(const char* pText)
{
	return pText!=NULL ? std::string(strlen(pText), '*') : std::string();
}


//----------------------------------------------------------------------
// Return the bytes through nCharacters decoded editor characters. The old
// loop stopped immediately after the final lead byte, leaving the last UTF-8
// sequence incomplete. DecodeNext advances through all of its continuation
// bytes first. Invalid bytes remain in the returned prefix when they precede
// the requested character; this function selects a boundary and otherwise
// leaves the source bytes untouched.
//----------------------------------------------------------------------
inline std::string
Utf8Prefix(const char* pText, size_t nCharacters)
{
	if (pText==NULL || nCharacters==0)
	{
		return std::string();
	}

	const unsigned char* const pBegin = (const unsigned char*)pText;
	const unsigned char* p = pBegin;
	size_t nDecoded = 0;

	while (*p!='\0' && nDecoded<nCharacters)
	{
		uint32_t c = 0;

		if (Detail::DecodeNext(p, c))
		{
			++nDecoded;
		}
	}

	return std::string(pText, (size_t)(p-pBegin));
}


struct ItemTooltipNames
{
	std::string local;
	std::string english;
};


//----------------------------------------------------------------------
// Finish the two item-name rows shared by tooltip measurement and rendering.
// Prefixes already contain time-item and option text; the English prefix also
// contains its opening parenthesis. Grade entries use the legacy two-byte
// token at grade*2, but only when the data entry really owns both bytes.
//----------------------------------------------------------------------
inline ItemTooltipNames
FinishItemTooltipNames(const std::string& localPrefix,
					   const std::string& englishPrefix,
					   const char* pLocalName,
					   const char* pEnglishName,
					   const char* pGradeTable,
					   int nGrade)
{
	ItemTooltipNames names;
	names.local = localPrefix;
	names.english = englishPrefix;

	if (pLocalName != NULL)
	{
		names.local += pLocalName;
	}

	if (pEnglishName != NULL)
	{
		names.english += pEnglishName;
	}

	if (pGradeTable!=NULL && nGrade>0)
	{
		const size_t nTableLength = strlen(pGradeTable);

		if (nTableLength>=2 && (size_t)nGrade<=(nTableLength-2)/2)
		{
			const size_t nOffset = (size_t)nGrade * 2;
			names.local.append(pGradeTable+nOffset, 2);
			names.english.append(pGradeTable+nOffset, 2);
		}
	}

	names.english += ')';

	return names;
}

} // namespace UISafeText

#endif // __UI_SAFE_TEXT_H__
