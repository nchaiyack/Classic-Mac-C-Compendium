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


#include <string.h>
#include "wt.h"
#include "error.h"
#include "wtmem.h"
#include "framebuf.h"
#include "graphics.h"
#include "dither.h"


static int16 *ditherpix;


/* Calculate the table we use for fast dithering from 16 bpp to 8 bpp.  */
void calc_dither(Graphics_info *ginfo)
{
     int r, g, b;
     int i;
     int levels = 32;
     int device_levels = ginfo->palette.rgb_cube_size;
     int from_device = 256 / device_levels;
     int from_texture = 256 / levels;
     int thresh[4];


     for (i = 0; i < 4; i++)
	  thresh[i] = i * from_device / 4;

     ditherpix = wtmalloc(2 * sizeof(int16) * levels * levels * levels);

     for (r = 0; r < levels; r++)
     for (g = 0; g < levels; g++)
     for (b = 0; b < levels; b++) {
	  int index = ((r * levels + g) * levels + b) * 2;
	  int red, green, blue;
	  long pix[4];
	  

	  for (i = 0; i < 4; i++) {
	       red = MIN(r * from_texture / from_device, device_levels - 2);
	       if (r * from_texture - red * from_device > thresh[i])
		    red++;
	       green = MIN(g * from_texture / from_device, device_levels - 2);
	       if (g * from_texture - green * from_device > thresh[i])
		    green++;
	       blue = MIN(b * from_texture / from_device, device_levels - 2);
	       if (b * from_texture - blue * from_device > thresh[i])
		    blue++;
	       pix[i] =
		    ginfo->palette.color_lookup[(red * device_levels + green) *
						device_levels + blue];
	  }

	  ditherpix[index] = pix[3] * 256 + pix[0];
	  ditherpix[index + 1] = pix[1] * 256 + pix[2];
     }
}
     

/* Expand each pixel in the framebuffer to a 4 pixel block. */
void expand(register Pixel *bufp, register DevicePixel *buf2p,
	    int width, int height)
{
     register int xpix;
     Pixel *last_pixel = bufp + height * width - 1;


     xpix = width;
     while (bufp < last_pixel) {
	  register int8 p;

	  p = *bufp++;
	  *buf2p++ = p;
	  *buf2p++ = p;
	  if (--xpix == 0) {
	       xpix = width;
	       memcpy(buf2p, buf2p - width * 2, width * 2);
	       buf2p += width * 2;
	  }
     }
}


/* Expand each pixel in the framebuffer to a 4 pixel dithered block.  This
**   code is endianness dependent, but it doesn't matter if the dither
**   pattern is 'backward'
*/
void expand_dither(register Pixel *bufp, register DevicePixel *buf2p,
		   int width, int height)
{
     register int xpix;
     Pixel *last_pixel = bufp + height * width - 1;
     short *out = (short *) buf2p;


     xpix = width;
     while (bufp < last_pixel) {
	  register long p;

	  p = (long) (*bufp++);
	  out[0] = ditherpix[p + p];
	  out[width] = ditherpix[p + p + 1];
	  out++;

	  if (--xpix == 0) {
	       xpix = width;
	       out += width;
	  }
     }
}


void dither(register Pixel *bufp, register DevicePixel *buf2p,
	    int width, int height)
{
     int8 *ditherpix8 = (int8 *) ditherpix;
     register int x;
     int y;


     for (y = 0; y < height; y++) {
	  register int z = y & 0x1;
	  
	  for (x = 0; x < width; x++) {
	       *buf2p++ = ditherpix8[((long) (*bufp++) << 2) + z];
	       z ^= 0x2;
	  }
     }
}
