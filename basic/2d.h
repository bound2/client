#ifndef __2D__
#define __2D__

#include "Typedef.h"
/* Do not include the real <DDraw.h>: this project no longer uses real
   DirectDraw, and no type declared in this file needs it. Pulling it in
   redefines DDPIXELFORMAT/DDSCAPS2/... (SDL stand-ins used elsewhere)
   with incompatible types. */
#include "DLL.h"

/*-----------------------------------------------------------------------------
  S_SURFACEINFO (ǥ������)
-----------------------------------------------------------------------------*/
typedef struct
{
   void  *p_surface;
   int   width;
   int   height;
   int   pitch;

} S_SURFACEINFO;

/*-----------------------------------------------------------------------------
  �簢�� ����ü
-----------------------------------------------------------------------------*/
typedef struct
{
   int   x;    // x ��ǥ
   int   y;    // y ��ǥ
   int   w;    // ����
   int   h;    // ����

} S_RECT;

#endif
