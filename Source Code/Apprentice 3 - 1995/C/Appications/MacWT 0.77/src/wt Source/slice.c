/*
**  MacWT -- a 3d game engine for the Macintosh
**  © 1995, Bill Hayden and Nikol Software
**
**  On the Internet:
**  bmoc1@aol.com (my personal address)
**  nikolsw@grove.ufl.edu (my school address)
**	MacWT anonymous FTP site: ftp.circa.ufl.edu/pub/software/ufmug/mirrors/LocalSW/Hayden/
**  http://grove.ufl.edu:80/~nikolsw (my WWW page, containing MacWT info)
**
**  based on wt, by Chris Laurel (claurel@mr.net)
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

inline void draw_floor_slice(Pixel *start, Pixel *tex,
			     fixed x, fixed y, fixed dx, fixed dy,
			     int tex_width);

inline void draw_transparent_slice(Pixel *start, Pixel *end, Pixel *tex_base,
			    fixed tex_y, fixed tex_dy, int fb_width, 
			    int tex_height, int npix);



/*****************************************************************/



inline void draw_wall_slice(Pixel *start, Pixel *end, Pixel *tex_base,
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



inline void draw_floor_slice(Pixel *start, Pixel *tex,
			     fixed x, fixed y, fixed dx, fixed dy,
			     int tex_width)
{
     register Pixel temp;


     temp = *start;
     if (temp != PIXEL_GUARD_VAL)
	  return;

     if (tex_width == 64) {
	  /* width of 64 == 2^6 */
	  y <<= 6;
	  dy <<= 6;

	  do {

	       x &= 0x3fffff;
	       /* No need to mask y--it rolls automatically *if* ints
               **   are 32 bits.
	       */

	       temp = tex[FIXED_TO_INT((y & 0xfc00000) | x)];
	       *start++ = temp;
	       x += dx;
	       temp = *start;
	       y += dy;

	  } while (temp == PIXEL_GUARD_VAL);

     } else {

	  /* tex_width == 128 == 2^7*/
	  y <<= 7;
	  dy <<= 7;

	  do {

	       x &= 0x7fffff;

	       temp = tex[FIXED_TO_INT((y & 0x3f800000) | x)];
	       *start++ = temp;
	       x += dx;
	       temp = *start;
	       y += dy;

	  } while (temp == PIXEL_GUARD_VAL);

     }
}



/* npix is not used in this version of draw_transparent_slice, but it is
**   used in others for loop unrolling.
*/
inline void draw_transparent_slice(Pixel *start, Pixel *end, Pixel *tex_base,
			    fixed tex_y, fixed tex_dy, int fb_width, 
			    int tex_height, int npix)
{
     unsigned int utex_y;
     Pixel p;


     /* As a speed optimization, tex_y and tex_dy are shifted left by several
     **   bits so that tex_y can overflow correctly without explicit
     **   and-masking.  For this to occur correctly, it is necessary to
     **   make utex_y an unsigned quantity.  This is probably non-portable,
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





