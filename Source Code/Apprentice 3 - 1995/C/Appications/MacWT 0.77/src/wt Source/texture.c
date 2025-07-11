/*
**  MacWT -- a 3d game engine for the Macintosh
**  � 1995, Bill Hayden and Nikol Software
**
**  On the Internet:
**  bmoc1@aol.com (my personal address)
**  nikolsw@grove.ufl.edu (my school address)
**	MacWT anonymous FTP site: ftp.circa.ufl.edu/pub/software/ufmug/mirrors/LocalSW/Hayden/
**  http://grove.ufl.edu:80/~nikolsw (my WWW page, containing MacWT info)
**
**  based on wt, by Chris Laurel (claurel@mr.net)
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* GIF87a code from Thomas Hamren (d0hampe@dtek.chalmers.se) */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "wt.h"
#include "wtmem.h"
#include "error.h"
#include "color.h"
#include "graphfile.h"
#include "framebuf.h"
#include "graphics.h"
#include "texture.h"
#include "MacWT.h"


static int ilog2(int x);
static void quantize_texture(Graphic_file *gfile, Texture *t);




Texture *new_texture(int width, int height)
{
	Texture *t;

	if (height <= 0 || width <= 0)
		fatal_error("new_texture:  bad texture dimensions");

	t = wtmalloc(sizeof(Texture));
	t->texels = wtmalloc(height * width * sizeof(Pixel));

	t->width = width;
	t->height = height;
	t->log2height = ilog2(height);
     
	return t;
}



Texture *read_texture_file(char *filename)
{
	Graphic_file *gfile;
	Texture *t;


	gfile = read_graphic_file(filename);
	if (gfile == NULL)
		fatal_error("Error reading texture %s.", filename);

	/* The height and width should be powers of two for efficient
	**   texture mapping.  Here, we enforce this.
	*/
	if (ilog2(gfile->height) == -1)
		fatal_error("Height of texture %s is not a power of two.", filename);
     
	t = new_texture(gfile->width, gfile->height);
	quantize_texture(gfile, t);

	free_graphic_file(gfile);
     
	return t;
}



/* Return the log base 2 of the argument, or -1 if the argument is not
**   an integer power of 2.
*/
static int ilog2(int x)
{
	int i;
	unsigned int n;

	if (x <= 0)
		return -1;
	else
		n = (unsigned int) x;
     
	for (i = 0; (n & 0x1) == 0; i++, n >>= 1);

	if (n == 1)
		return i;
	else
		return -1;
}


static void quantize_texture(Graphic_file *gfile, Texture *t)
{
	RGBcolor	pixel;		// Generic Color
	RGBColor	macColor;	// Macintosh Color  (notice the case)
	short		x, y;
	Pixel		*dest_pixel = t->texels;


	for (x = gfile->width - 1; x >= 0; x--)
		for (y = gfile->height - 1; y >= 0; y--)
			{
			if (!graphic_file_pixel(gfile, x, y, &pixel))
				*dest_pixel++ = (Pixel) ~0;
			else
				{
				if (gTrueColor)
					{
					*dest_pixel++ = (Pixel)(( (pixel.red >> 3) << 10) +
											( (pixel.green >> 3) << 5) +
											(pixel.blue >> 3));
					}
				else
					{
					macColor.red = pixel.red << 8;
					macColor.green = pixel.green << 8;
					macColor.blue = pixel.blue << 8;
					*dest_pixel++ = (Pixel)Color2Index( &macColor );
					}
				}
			}
}
