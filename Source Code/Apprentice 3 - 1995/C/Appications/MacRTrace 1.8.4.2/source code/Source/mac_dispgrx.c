/*
 * Copyright (c) 1993, 1994 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Greg Ferrar         - original code
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Antonio Costa, at INESC-Norte. The name of the author and
 * INESC-Norte may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "config.h"

/* Externals */

extern WindowPtr image_window;			/* the image window */
extern int  screen_size_x;
extern long	*offscreen_bitmap_base;   		/* Base address of offscreen bitmap */
extern Boolean image_window_visible;		/* True if the image window is visible */

/***************************** Local Prototypes ********************/
void display_finished(void);
void display_close(void);
void display_init(int width, int height);
void display_plot(int x, int y, real r, real g, real b);


void
display_finished(void)
{

  /* No need to pause on the Macintosh after display */

}

void display_close(void)
{

  /* Macintosh window stays open after display */

}

void display_init(int width, int height)
{

  /* Initialization done elsewhere */

}

void display_plot(int x, int y, real r, real g, real b)
{

	long *pixelAddress;
	
	RGBColor color;

	/* Set the color */	
	color.red = 0xFFFF * r;
	color.green = 0xFFFF * g;
	color.blue = 0xFFFF * b;

	/* Draw the pixel, if it can be seen */
	if (image_window_visible)
		SetCPixel(x, y, &color);

	/* Write the RGB value directly to the offscreen Picture */
	pixelAddress = offscreen_bitmap_base + y*screen_size_x + x;
	*pixelAddress = ((color.red & 0xFF00) << 8) +
					(color.green & 0xFF00) +
					(color.blue >> 8);

}	/* display_plot() */

