//----------------------------------------------------------------------
// UIUtilityFunctions.cpp - macOS UI utility functions
//----------------------------------------------------------------------
#include "../Client_PCH.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//----------------------------------------------------------------------
// String Reduction Functions
//
// All three write in place through a caller-supplied pointer and are given
// no capacity, so the only bound they can honour is the string itself: they
// guarantee that every index they touch lies at or before the existing
// terminator, and nothing beyond that. A caller whose buffer is shorter
// than the string it holds is still on its own.
//
// The guarantee comes from one shared precondition - strlen(pStr) > maxWidth
// before anything is written. It makes pStr[maxWidth] the last byte touched
// and puts it at or before the terminator, which is why each function has to
// establish it and none may write without it.
//----------------------------------------------------------------------

// Reduce string to fit width (basic version)
void ReduceString(char* pStr, int maxWidth)
{
    if (pStr == NULL || maxWidth <= 0)
        return;

    int len = (int)strlen(pStr);
    if (len <= maxWidth)
        return;

    // The marker costs three of the maxWidth characters. Under that there is
    // no room for it and maxWidth - 3 would index in front of the string, so
    // cut without a marker; len > maxWidth keeps pStr[maxWidth] in bounds.
    if (maxWidth < 3)
    {
        pStr[maxWidth] = '\0';
        return;
    }

    // Truncate and add "..."
    pStr[maxWidth - 3] = '.';
    pStr[maxWidth - 2] = '.';
    pStr[maxWidth - 1] = '.';
    pStr[maxWidth] = '\0';
}

// Reduce string variant 2 (for DBCS strings)
void ReduceString2(char* pStr, int maxWidth)
{
    if (pStr == NULL || maxWidth <= 0)
        return;

    // The precondition the port dropped. VS_UI/src/hangul/FL2.cpp:425, which
    // this supersedes, wrapped its whole body in `if (lt > len)`; here the
    // length came instead from a loop that stopped at the terminator, so a
    // string shorter than maxWidth still reached the write below and stamped
    // "..." at pStr[maxWidth - 3 .. maxWidth] - past a string that needed no
    // reducing at all.
    //
    // That loop also stepped p += 2 on any byte with the high bit set, which
    // walks over the terminator when a DBCS lead byte is the last byte of the
    // string - and the callers here fill their buffers with sprintf("%s"),
    // which cuts multi-byte text wherever the buffer happens to end. strlen
    // answers the same question and cannot run off the end.
    int len = (int)strlen(pStr);
    if (len <= maxWidth)
        return;

    if (maxWidth < 3)
    {
        pStr[maxWidth] = '\0';
        return;
    }

    pStr[maxWidth - 3] = '.';
    pStr[maxWidth - 2] = '.';
    pStr[maxWidth - 1] = '.';
    pStr[maxWidth] = '\0';
}

// Reduce string variant 3 (with ellipsis in middle)
void ReduceString3(char* pStr, int maxWidth)
{
    if (pStr == NULL || maxWidth <= 0)
        return;

    int len = strlen(pStr);
    if (len <= maxWidth)
        return;

    // Truncate from end and add "..."
    int cutLen = maxWidth - 3;
    if (cutLen > 0)
    {
        pStr[cutLen] = '.';
        pStr[cutLen + 1] = '.';
        pStr[cutLen + 2] = '.';
        pStr[cutLen + 3] = '\0';
    }
}

//----------------------------------------------------------------------
// Face Style
//----------------------------------------------------------------------
// UI_GetFaceStyle - Implemented in PacketFunction.cpp
// int UI_GetFaceStyle(bool bMale, int index) { ... }

//----------------------------------------------------------------------
// Input Update Functions
//----------------------------------------------------------------------
// NOTE: UpdateInput() and UpdateMouse() are implemented in GameMain.cpp
// Stub implementations removed to avoid duplicate symbols
/*
void UpdateInput()
{
    // Update DirectInput state (mouse buttons, keyboard, etc.)
    if (g_pSDLInput) {
        g_pSDLInput->UpdateInput();
    }
}

void UpdateMouse()
{
    // Stub: SDL input system handles this
}
*/

//----------------------------------------------------------------------
// File I/O Helper
//----------------------------------------------------------------------
// NOTE: FileOpenBinary() is implemented in GameMain.cpp
// Stub implementation removed to avoid duplicate symbol
/*
bool FileOpenBinary(const char* pFilename, std::ifstream& file)
{
    if (pFilename == NULL)
        return false;

    file.open(pFilename, std::ios::binary);
    return file.is_open();
}
*/

//----------------------------------------------------------------------
// SPRITE_FILEPOSITION_NODE
//----------------------------------------------------------------------
// Methods already exist elsewhere - commenting out stubs
// bool SPRITE_FILEPOSITION_NODE::LoadFromFile(std::ifstream& file)
// {
//     if (!file.is_open())
//         return false;
//     file.read((char*)&Position, sizeof(POINT));
//     file.read((char*)&Width, sizeof(WORD));
//     file.read((char*)&Height, sizeof(WORD));
//     return file.good();
// }
