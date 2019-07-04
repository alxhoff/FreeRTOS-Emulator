/*!
  \file Draw_Rect.c
  \author Mario Palomo <mpalomo@ihman.com>
  \author Jose M. de la Huerga Fernández
  \author Pepe González Mora
  \date 05-2002

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "SDL/SDL.h"

#define SDL_DRAW_BPP  1

#if SDL_DRAW_BPP == 1
#define SDL_DRAW_PUTPIXEL \
  memset(p0, color, w);                \
  memset(p1, color, w);                \
                                       \
  if (h<3)  return;                    \
  p0 = (unsigned char*)super->pixels + (y+1)*super->pitch + x;       \
  p1 = (unsigned char*)super->pixels + (y+1)*super->pitch + (x+w-1); \
  i = h-2;                             \
  switch( i % 4 ) {                    \
    do{                                \
      case 0:                          \
        *p0 = color; p0+=super->pitch; \
        *p1 = color; p1+=super->pitch; \
      case 3:                          \
        *p0 = color; p0+=super->pitch; \
        *p1 = color; p1+=super->pitch; \
      case 2:                          \
        *p0 = color; p0+=super->pitch; \
        *p1 = color; p1+=super->pitch; \
      case 1:                          \
        *p0 = color; p0+=super->pitch; \
        *p1 = color; p1+=super->pitch; \
    }while( (i-=4) > 0 );              \
  }


#elif SDL_DRAW_BPP == 2
#define SDL_DRAW_PUTPIXEL \
  i = w;                                        \
  switch( i % 4 ) {                             \
    do{                                         \
      case 0:                                   \
        *(unsigned short*)p0 = color; p0+=2;            \
        *(unsigned short*)p1 = color; p1+=2;            \
      case 3:                                   \
        *(unsigned short*)p0 = color; p0+=2;            \
        *(unsigned short*)p1 = color; p1+=2;            \
      case 2:                                   \
        *(unsigned short*)p0 = color; p0+=2;            \
        *(unsigned short*)p1 = color; p1+=2;            \
      case 1:                                   \
        *(unsigned short*)p0 = color; p0+=2;            \
        *(unsigned short*)p1 = color; p1+=2;            \
    }while( (i-=4) > 0 );                       \
  }                                             \
  if (h<3)  return;                             \
  p0 = (unsigned char*)super->pixels + (y+1)*super->pitch + x*2;       \
  p1 = (unsigned char*)super->pixels + (y+1)*super->pitch + (x+w-1)*2; \
  i = h-2;                                      \
  switch( i % 4 ) {                             \
    do{                                         \
      case 0:                                   \
        *(unsigned short*)p0 = color; p0+=super->pitch; \
        *(unsigned short*)p1 = color; p1+=super->pitch; \
      case 3:                                   \
        *(unsigned short*)p0 = color; p0+=super->pitch; \
        *(unsigned short*)p1 = color; p1+=super->pitch; \
      case 2:                                   \
        *(unsigned short*)p0 = color; p0+=super->pitch; \
        *(unsigned short*)p1 = color; p1+=super->pitch; \
      case 1:                                   \
        *(unsigned short*)p0 = color; p0+=super->pitch; \
        *(unsigned short*)p1 = color; p1+=super->pitch; \
    }while( (i-=4) > 0 );                       \
  }


#elif SDL_DRAW_BPP == 3
#define SDL_DRAW_PUTPIXEL_BPP_3_AUX \
    if (SDL_BYTEORDER == SDL_BIG_ENDIAN) { \
      p0[0] = colorbyte2;                  \
      p0[1] = colorbyte1;                  \
      p0[2] = colorbyte0;                  \
      p1[0] = colorbyte2;                  \
      p1[1] = colorbyte1;                  \
      p1[2] = colorbyte0;                  \
    } else {                               \
      p0[0] = colorbyte0;                  \
      p0[1] = colorbyte1;                  \
      p0[2] = colorbyte2;                  \
      p1[0] = colorbyte0;                  \
      p1[1] = colorbyte1;                  \
      p1[2] = colorbyte2;                  \
    }

#define SDL_DRAW_PUTPIXEL \
  i = w;                                    \
  switch( i % 4 ) {                         \
    do{                                     \
      case 0:                               \
        SDL_DRAW_PUTPIXEL_BPP_3_AUX         \
        p0+=3; p1+=3;                       \
      case 3:                               \
        SDL_DRAW_PUTPIXEL_BPP_3_AUX         \
        p0+=3; p1+=3;                       \
      case 2:                               \
        SDL_DRAW_PUTPIXEL_BPP_3_AUX         \
        p0+=3; p1+=3;                       \
      case 1:                               \
        SDL_DRAW_PUTPIXEL_BPP_3_AUX         \
        p0+=3; p1+=3;                       \
    }while( (i-=4) > 0 );                   \
  }                                         \
  if (h<3)  return;                         \
  p0 = (unsigned char*)super->pixels + (y+1)*super->pitch + x*3;       \
  p1 = (unsigned char*)super->pixels + (y+1)*super->pitch + (x+w-1)*3; \
  i = h-2;                                  \
  switch( i % 4 ) {                         \
    do{                                     \
      case 0:                               \
        SDL_DRAW_PUTPIXEL_BPP_3_AUX         \
        p0+=super->pitch; p1+=super->pitch; \
      case 3:                               \
        SDL_DRAW_PUTPIXEL_BPP_3_AUX         \
        p0+=super->pitch; p1+=super->pitch; \
      case 2:                               \
        SDL_DRAW_PUTPIXEL_BPP_3_AUX         \
        p0+=super->pitch; p1+=super->pitch; \
      case 1:                               \
        SDL_DRAW_PUTPIXEL_BPP_3_AUX         \
        p0+=super->pitch; p1+=super->pitch; \
    }while( (i-=4) > 0 );                   \
  }


#elif SDL_DRAW_BPP == 4

#ifdef __linux__
#define SDL_DRAW_WMEMSET_START \
if (sizeof(wchar_t) == sizeof(unsigned int)) { \
  wmemset((wchar_t*)p0, color, w); \
  wmemset((wchar_t*)p1, color, w); \
} else {
#define SDL_DRAW_WMEMSET_END }
#else
#define SDL_DRAW_WMEMSET_START
#define SDL_DRAW_WMEMSET_END
#endif

#define SDL_DRAW_PUTPIXEL \
SDL_DRAW_WMEMSET_START                          \
  i = w;                                        \
  switch( i % 4 ) {                             \
    do{                                         \
      case 0:                                   \
        *(unsigned int*)p0 = color; p0+=4;            \
        *(unsigned int*)p1 = color; p1+=4;            \
      case 3:                                   \
        *(unsigned int*)p0 = color; p0+=4;            \
        *(unsigned int*)p1 = color; p1+=4;            \
      case 2:                                   \
        *(unsigned int*)p0 = color; p0+=4;            \
        *(unsigned int*)p1 = color; p1+=4;            \
      case 1:                                   \
        *(unsigned int*)p0 = color; p0+=4;            \
        *(unsigned int*)p1 = color; p1+=4;            \
    }while( (i-=4) > 0 );                       \
  }                                             \
SDL_DRAW_WMEMSET_END                            \
  if (h<3)  return;                             \
  p0 = (unsigned char*)super->pixels + (y+1)*super->pitch + x*4;       \
  p1 = (unsigned char*)super->pixels + (y+1)*super->pitch + (x+w-1)*4; \
  i = h-2;                                      \
  switch( i % 4 ) {                             \
    do{                                         \
      case 0:                                   \
        *(unsigned int*)p0 = color; p0+=super->pitch; \
        *(unsigned int*)p1 = color; p1+=super->pitch; \
      case 3:                                   \
        *(unsigned int*)p0 = color; p0+=super->pitch; \
        *(unsigned int*)p1 = color; p1+=super->pitch; \
      case 2:                                   \
        *(unsigned int*)p0 = color; p0+=super->pitch; \
        *(unsigned int*)p1 = color; p1+=super->pitch; \
      case 1:                                   \
        *(unsigned int*)p0 = color; p0+=super->pitch; \
        *(unsigned int*)p1 = color; p1+=super->pitch; \
    }while( (i-=4) > 0 );                       \
  }

#endif /*SDL_DRAW_BPP*/


void SDL_DRAWFUNCTION(SDL_Surface *super,
                  signed short x, signed short y, unsigned short w, unsigned short h,
                  unsigned int color)
{
#if SDL_DRAW_BPP == 3
  unsigned char colorbyte0 = (unsigned char) (color & 0xff);
  unsigned char colorbyte1 = (unsigned char) ((color >> 8)  & 0xff);
  unsigned char colorbyte2 = (unsigned char) ((color >> 16) & 0xff);
#endif

  register unsigned char *p0;
  register unsigned char *p1;
  register signed short i;

  if (w==0 || h==0)  return;

  p0 = (unsigned char*)super->pixels +    y    * super->pitch + x * SDL_DRAW_BPP;
  p1 = (unsigned char*)super->pixels + (y+h-1) * super->pitch + x * SDL_DRAW_BPP;

  /* Lock surface */
  if (SDL_MUSTLOCK(super)) {
      if (SDL_LockSurface(super) < 0)  { return; }
  }
  
  SDL_DRAW_PUTPIXEL;

  /* Unlock surface */
  if (SDL_MUSTLOCK(super))  { SDL_UnlockSurface(super); }
  
}/*Draw_Rect*/


#undef SDL_DRAW_PUTPIXEL
#undef SDL_DRAW_PUTPIXEL_BPP_3_AUX

#undef SDL_DRAW_WMEMSET_START
#undef SDL_DRAW_WMEMSET_END

