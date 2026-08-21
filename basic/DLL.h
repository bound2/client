/*-----------------------------------------------------------------------------

	DLL.h

	Dynamic Link Library general header file.

	2000.8.8. KJTINC

-----------------------------------------------------------------------------*/

#ifndef __DLL_H__
#define __DLL_H__

#define _CPP

#ifdef PLATFORM_WINDOWS
// `export`/`import` used to be defined here as plain macros and then
// aliased into DllExport/DllImport below. `export` is a reserved C++
// keyword (historically for exported templates), and the C++ standard
// library explicitly guards against it being macroized - <xkeycheck.h>
// #errors out (C1189) the moment any standard header is included in a
// translation unit where `export` is a macro. Defining DllExport/DllImport
// directly, without a bare `export`/`import` macro ever existing, avoids
// that collision entirely; nothing outside this file used the bare names.
#ifndef _CPP
#define DllExport extern "C" __declspec (dllexport)
#define DllImport extern "C" __declspec (dllimport)
#else
#define DllExport __declspec (dllexport)
#define DllImport __declspec (dllimport)
#endif

#ifdef _DLL_EXPORT
#define DllClass		DllExport
#else
#define DllClass		DllImport
#endif
#else
// Non-Windows platforms: empty macros
#define DllExport
#define DllImport extern
#define DllClass
#endif

#endif