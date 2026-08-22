#ifndef __2D__
#define __2D__

#include "Typedef.h"
/* Do not include the real <DDraw.h>: this project no longer uses real
   DirectDraw, and no type declared in this file needs it. Pulling it in
   redefines DDPIXELFORMAT/DDSCAPS2/... (SDL stand-ins used elsewhere)
   with incompatible types. */
#include "DLL.h"

/*-----------------------------------------------------------------------------
  S_SURFACEINFO (표면정보)
-----------------------------------------------------------------------------*/
typedef struct
{
   void  *p_surface;
   int   width;
   int   height;
   int   pitch;

} S_SURFACEINFO;

/*-----------------------------------------------------------------------------
  사각형 구조체
-----------------------------------------------------------------------------*/
typedef struct
{
   int   x;    // x 좌표
   int   y;    // y 좌표
   int   w;    // 넓이
   int   h;    // 높이

} S_RECT;

#endif
