//--------------------------------------------------------------------------------
//
// Filename   : PacketAssert.h
//
// Renamed from Assert.h: the old name collided with the standard C++
// <cassert>/<assert.h> header. With Client/Packet on the include path,
// <cassert>'s internal #include <assert.h> resolved to this file instead
// of the real CRT header, so the standard assert() macro was never
// defined - breaking any translation unit that used plain assert().
//
// Written By : Reiot
//
//--------------------------------------------------------------------------------

#ifndef __ASSERT_H__
#define __ASSERT_H__

// include files
#include "Types.h"
#include "Exception.h"

//--------------------------------------------------------------------------------
//
// Config.h 에 NDEBUG 가 정의되면, 모든 Assert를 무시한다.
// 그렇지 않은 경우 Assert가 실패하면 파일에 로그를 한후, AssertError를 리턴한다.
//
//--------------------------------------------------------------------------------
void __assert__ ( const char * file , uint line , const char * func , const char * expr ) throw ( AssertionError );

#if defined(NDEBUG)
	#define Assert(expr) ((void)0)
#elif __LINUX__
	#define Assert(expr) ((void)((expr)?0:(__assert__(__FILE__,__LINE__,__PRETTY_FUNCTION__,#expr),0)))
#elif __WIN_CONSOLE__ || __WIN32__
	#define Assert(expr) ((void)((expr)?0:(__assert__(__FILE__,__LINE__,"",#expr),0)))
#elif __MFC__
	#define Assert(expr) ASSERT(expr)
#elif defined(__APPLE__) || defined(__macos__)
	#define Assert(expr) ((void)((expr)?0:(__assert__(__FILE__,__LINE__,__PRETTY_FUNCTION__,#expr),0)))
#endif

#endif
