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


#include <stdlib.h>
#include <string.h>
#include "wt.h"
#include "error.h"
#include "wtmem.h"
#include "framebuf.h"
#include "graphics.h"


Framebuffer *new_framebuffer(int width, int height)
{
	Framebuffer *fb;

	fb = wtmalloc(sizeof(Framebuffer));
	fb->fb_width = width;
	fb->fb_height = height;
	fb->pixels = get_framebuffer_memory(width, height);
     
	return fb;
}


void clear_framebuffer(Framebuffer *fb)
{
	int i;
	Pixel *p;

	memset(fb->pixels, 0xff, fb->fb_height * fb->fb_width * sizeof(Pixel));

	p = fb->pixels + fb->fb_width - 1;
	for (i = 0; i < fb->fb_height; i++, p += fb->fb_width)
		*p = 0;
}
