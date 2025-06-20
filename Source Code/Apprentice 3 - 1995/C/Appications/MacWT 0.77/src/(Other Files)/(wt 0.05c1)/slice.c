/*
**  wt -- a 3d game engine
**
**  Copyright (C) 1994 by Chris Laurel
**  email:  claurel@mr.net
**  snail mail:  Chris Laurel, 5700 W Lake St #208,  St. Louis Park, MN  55416
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#define PIXEL_GUARD_VAL ((Pixel) ~0)


inline void draw_wall_slice(Pixel *start, Pixel *end, Pixel *tex_base,
			    fixed tex_y, fixed tex_dy, int fb_width, 
			    int tex_height, int npix);
inline void draw_transparent_slice(Pixel *start, Pixel *end, Pixel *tex_base,
				   fixed tex_y, fixed tex_dy, int fb_width, 
				   int tex_height, int npix);
inline void draw_floor_slice(Pixel *start, Pixel *tex,
			     fixed x, fixed y, fixed dx, fixed dy,
			     int tex_width);



/* npix is not used in this version of draw_wall_slice, but it is
**   used in others for loop unrolling.
*/
inline void __draw_wall_slice(Pixel *start, Pixel *end, Pixel *tex_base,
			    fixed tex_y, fixed tex_dy, int fb_width, 
			    int tex_height, int npix)
{
     unsigned int utex_y;

     /* As a speed optimization, tex_y and tex_dy are shifted left by several
     **   bits so that tex_y can overflow correctly without explicit
     **   and-masking.  For this to occur correctly, it is necessary to
     **   make utex_y and unsigned quantity.  This is probably non-portable,
     **   but should work with most architectures.
     */
     utex_y = (unsigned int) tex_y;

     if (tex_height == 64) {

	  utex_y <<= 10;
	  tex_dy <<= 10;

	  while (start >= end) {
	       *start = tex_base[utex_y >> 26];
	       utex_y += tex_dy;
	       start -= fb_width;
	  }

     } else {

	  utex_y <<= 9;
	  tex_dy <<= 9;

	  while (start >= end) {
	       *start = tex_base[utex_y >> 25];
	       utex_y += tex_dy;
	       start -= fb_width;
	  }

     }
}

inline void draw_wall_slice(Pixel *s, Pixel *e, Pixel *tb,
			    fixed ty, fixed tdy, int fbw, 
			    int log2_tex_height, int npix)
{
     switch (log2_tex_height) {
     case 4: __draw_wall_slice(s, e, tb, ty, tdy, fbw, 4, npix); break;
     case 5: __draw_wall_slice(s, e, tb, ty, tdy, fbw, 5, npix); break;
     case 6: __draw_wall_slice(s, e, tb, ty, tdy, fbw, 6, npix); break;
     case 7: __draw_wall_slice(s, e, tb, ty, tdy, fbw, 7, npix); break;
     case 8: __draw_wall_slice(s, e, tb, ty, tdy, fbw, 8, npix); break;
     case 9: __draw_wall_slice(s, e, tb, ty, tdy, fbw, 9, npix); break;
     case 10: __draw_wall_slice(s, e, tb, ty, tdy, fbw, 10, npix); break;
     case 11: __draw_wall_slice(s, e, tb, ty, tdy, fbw, 11, npix); break;
     case 12: __draw_wall_slice(s, e, tb, ty, tdy, fbw, 12, npix); break;
     }
}


/* npix is not used in this version of draw_transparent_slice, but it is
**   used in others for loop unrolling.
*/
inline void __draw_transparent_slice(Pixel *start, Pixel *end, Pixel *tex_base,
				     fixed tex_y, fixed tex_dy, int fb_width, 
				     int tex_height, int npix)
{
     unsigned int utex_y;
     Pixel p;


     /* As a speed optimization, tex_y and tex_dy are shifted left by several
     **   bits so that tex_y can overflow correctly without explicit
     **   and-masking.  For this to occur correctly, it is necessary to
     **   make utex_y and unsigned quantity.  This is probably non-portable,
     **   but should work with most architectures.
     */
     utex_y = (unsigned int) tex_y;

     if (tex_height == 64) {

	  utex_y <<= 10;
	  tex_dy <<= 10;

	  while (start >= end) {
	       p = tex_base[utex_y >> 26];
	       if (p != PIXEL_GUARD_VAL)
		    *start = p;
	       utex_y += tex_dy;
	       start -= fb_width;
	  }

     } else {

	  utex_y <<= 9;
	  tex_dy <<= 9;

	  while (start >= end) {
	       p = tex_base[utex_y >> 25];
	       if (p != PIXEL_GUARD_VAL)
		    *start = p;
	       utex_y += tex_dy;
	       start -= fb_width;
	  }

     }
}

inline void draw_transparent_slice(Pixel *s, Pixel *e, Pixel *tb,
				   fixed ty, fixed tdy, int fbw, 
				   int log2_tex_height, int np)
{
     switch (log2_tex_height) {
     case 4: __draw_transparent_slice(s, e, tb, ty, tdy, fbw, 4, np); break;
     case 5: __draw_transparent_slice(s, e, tb, ty, tdy, fbw, 5, np); break;
     case 6: __draw_transparent_slice(s, e, tb, ty, tdy, fbw, 6, np); break;
     case 7: __draw_transparent_slice(s, e, tb, ty, tdy, fbw, 7, np); break;
     case 8: __draw_transparent_slice(s, e, tb, ty, tdy, fbw, 8, np); break;
     case 9: __draw_transparent_slice(s, e, tb, ty, tdy, fbw, 9, np); break;
     case 10: __draw_transparent_slice(s, e, tb, ty, tdy, fbw, 10, np); break;
     case 11: __draw_transparent_slice(s, e, tb, ty, tdy, fbw, 11, np); break;
     case 12: __draw_transparent_slice(s, e, tb, ty, tdy, fbw, 12, np); break;
     }
}

inline void __draw_floor_slice(Pixel *start, Pixel *tex,
			       fixed x, fixed y, fixed dx, fixed dy,
			       int log2_tex_width)
{
     register Pixel temp;


     temp = *start;
     if (temp != PIXEL_GUARD_VAL)
	  return;

     y <<= log2_tex_width;	/* Perhaps this should be the height? */
     dy <<= log2_tex_width;

     do {
	  x &= 0xffffffff >> (16 - log2_tex_width);
	  x &= 0x3fffff;
	  /* No need to mask y--it rolls automatically *if* ints
	  **   are 32 bits.
	  */

	  temp = tex[FIXED_TO_INT((y & (((1<<log2_tex_width)-1)
					<<(log2_tex_width+16))) | x)];
	  *start++ = temp;
	  x += dx;
	  temp = *start;
	  y += dy;

     } while (temp == PIXEL_GUARD_VAL);

}

inline void draw_floor_slice(Pixel *s, Pixel *tex,
			       fixed x, fixed y, fixed dx, fixed dy,
			       int tex_width)
{
     switch (tex_width) {
     case 64: __draw_floor_slice(s, tex, x, y, dx, dy, 6); break;
     case 128: __draw_floor_slice(s, tex, x, y, dx, dy, 7); break;
     case 256: __draw_floor_slice(s, tex, x, y, dx, dy, 8); break;
	  /* Not sure 512 or more will work. */
     }
}
