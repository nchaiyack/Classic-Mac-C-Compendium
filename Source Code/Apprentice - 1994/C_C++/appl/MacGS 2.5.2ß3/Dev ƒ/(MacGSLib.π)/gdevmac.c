/* Copyright (C) 1993 Aladdin Enterprises.  All rights reserved.
   Distributed by Free Software Foundation, Inc.

This file is part of Ghostscript.

Ghostscript is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.  Refer
to the Ghostscript General Public License for full details.

Everyone is granted permission to copy, modify and redistribute
Ghostscript, but only under the conditions described in the Ghostscript
General Public License.  A copy of this license is supposed to have been
given to you along with Ghostscript so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies.  */

/* gdevmac.c */
/* (color) Macintosh driver for Ghostscript library */

#include <stdio.h>
#include <stddef.h>
#include <Icons.h>
#include "gdevmac.h"
#include "gserrors.h"
#include "gp_macui.h"


extern Boolean		gHasColorQD;
extern GUIProcRec	macGUIProcs;


#define DEFAULT_DPI		72	/* Macintosh standard monitor */
#define DEFAULT_WIDTH  ( 9 * DEFAULT_DPI)
#define DEFAULT_HEIGHT (12 * DEFAULT_DPI)


/* The device descriptor */

private gx_device_procs mac_procs =
{
	mac_open,
	gx_default_get_initial_matrix,
	mac_sync,
	mac_output_page,
	mac_close,
	mac_map_rgb_color,
	mac_map_color_rgb,
	mac_fill_rectangle,
	gx_default_tile_rectangle,
	mac_copy_mono,
	mac_copy_color,
	mac_draw_line,
	gx_default_get_bits,
	gx_default_get_props,
	gx_default_put_props
};


/* The instance is public. */

gx_device_mac gs_mac_device =
{
	sizeof (gx_device_mac),
	&mac_procs,
	"mac",
 	DEFAULT_WIDTH, DEFAULT_HEIGHT,	/* x and y extent */
 	DEFAULT_DPI  , DEFAULT_DPI   ,	/* x and y density */
	no_margins,
	dci_color (24, 255, 256),		/* # of bits per pixel,
									 * # of distinct color levels - 1,
									 * size of color cube for dithering */
 	0,			/* connection not initialized */

	GUI_MAGIC_NUMBER,
	&macGUIProcs,

	(CWindowPtr) NULL
};

/* Macros to extract colors */

#define RED(x)		((x & 0x00FF0000) >> 8)
#define GREEN(x)	((x & 0x0000FF00))
#define BLUE(x)		((x & 0x000000FF) << 8)

/* Macro to validate arguments */

#define check_rect()							\
{												\
	if (w <= 0 || h <= 0)						\
		return 0;								\
												\
	if (x < 0 || x > xdev->width  - w ||		\
		y < 0 || y > xdev->height - h)			\
		return_error (gs_error_rangecheck);		\
}


#define BEGIN_DRAW_PRIVATE(theEnd)				\
{												\
	short	i;									\
	short	iEnd = (theEnd);					\
	CWindowPtr	pictWin = macPictWindow (dev);	\
	void 	macBeginDraw	 (gx_device *dev);	\
	void 	macEndDraw		 (gx_device *dev);	\
	void 	macBeginPictDraw (gx_device *dev);	\
	short 	macEndPictDraw   (gx_device *dev);	\
												\
												\
	for (i = 0; i < iEnd; i++)					\
	{											\
		CWindowPtr windowPtr = (i == 0)			\
							   ? xdev->windowPtr\
							   : pictWin;		\
							   					\
							   					\
		if (i == 0)								\
			macBeginDraw (dev);					\
		else									\
			macBeginPictDraw (dev);				\
												\
		{

#define END_DRAW_PRIVATE						\
		}										\
												\
		if (i == 0)								\
			macEndDraw (dev);					\
		else									\
			retVal = macEndPictDraw (dev);		\
	}											\
}


#define BEGIN_DRAW								\
	BEGIN_DRAW_PRIVATE (2)

#define END_DRAW								\
	END_DRAW_PRIVATE


#define BEGIN_DRAW_SPECIAL(fClearPicture)		\
	BEGIN_DRAW_PRIVATE (fClearPicture ? 1 : 2)


#define END_DRAW_SPECIAL						\
	END_DRAW


short	macCreateWindow (gx_device *dev, short defaultWidth, short defaultHeight);
short	macCloseWindow  (gx_device *dev);
void	macClosePicture (gx_device *dev);
void	macClearPicture (gx_device *dev);


	private int
mac_open (register gx_device *dev)

{
	return macCreateWindow (dev, gs_mac_device.width, gs_mac_device.height);
}


/* Close the device. */

	private int
mac_close (register gx_device *dev)

{
	int		retVal = macCloseWindow (dev);


	if (retVal == 0)
	{
	}

	return retVal;
}


/* Synchronize the display with the commands already given */

	private int
mac_sync (register gx_device *dev)

{
	return 0;
}


/* Fill a rectangle with a color. */

	private int
mac_fill_rectangle (register gx_device *dev,
					int x, int y, int w, int h, gx_color_index color)

{
	int	retVal = 0;


	check_rect ();

	if (color != gx_no_color_index)
	{
		CWindowPtr	macPictWindow (gx_device *dev);
		CWindowPtr	pictWin		  = macPictWindow (dev);
		Boolean		fClearPicture =
						((CWindowPeek) pictWin)->port.picSave == (Handle) NULL &&
						x == 0 && y == 0 &&
						w == xdev->width && h == xdev->height;


		BEGIN_DRAW_SPECIAL (fClearPicture)

			Rect		theRect;


			if (gHasColorQD)
			{
			    RGBColor	rgbColor;


				rgbColor.red   = RED   (color);
				rgbColor.green = GREEN (color);
				rgbColor.blue  = BLUE  (color);
				RGBForeColor (&rgbColor);
			}

	    	SetRect (&theRect, x, y, x + w, y + h);
	    	PaintRect (&theRect);
	    	ValidRect (&theRect);

		END_DRAW_SPECIAL

		/*...Handle page erasures...*/

		if (fClearPicture)
			macClearPicture (dev);
	}

	return retVal;
}


/* Copy a monochrome bitmap. */

	private int
mac_copy_mono (register gx_device *dev,
			   const unsigned char *base, int sourcex, int raster, gx_bitmap_id id,
			   int x, int y, int w, int h, gx_color_index zero, gx_color_index one)

{
	int	retVal = 0;


	check_rect ();

	BEGIN_DRAW

		Rect		srcRect, dstRect;
		BitMap		srcBits, *dstBits;


		if (gHasColorQD)
		{
		    RGBColor	zeroColor;
		    RGBColor	oneColor;
	

			zeroColor.red	= RED	(zero);
			zeroColor.green = GREEN	(zero);
			zeroColor.blue	= BLUE	(zero);
			RGBBackColor (&zeroColor);
	
			oneColor.red	= RED	(one);
			oneColor.green	= GREEN	(one);
			oneColor.blue	= BLUE	(one);
			RGBForeColor (&oneColor);
		}
	
	    SetRect (&srcRect, 0, 0, w, h);
	    SetRect (&dstRect, x, y, x + w, y + h);
	    srcBits.baseAddr	  = (QDPtr) base;
	    srcBits.rowBytes	  = raster;
	    srcBits.bounds.left	  = srcRect.left;
	    srcBits.bounds.top    = srcRect.top;
	    srcBits.bounds.right  = srcRect.right;
	    srcBits.bounds.bottom = srcRect.bottom;
	    dstBits = &(((GrafPtr) windowPtr)->portBits);
		CopyBits (&srcBits, dstBits, &srcRect, &dstRect, srcCopy, NULL);

	END_DRAW

	return retVal;
}


/* Copy a color bitmap. */

	private int
mac_copy_color (register gx_device *dev,
				const unsigned char *base, int sourcex, int raster,  gx_bitmap_id id,
				int x, int y, int w, int h)

{
	int	retVal = 0;


	check_rect ();

	return retVal;
}


/* Draw a line */

	private int
mac_draw_line (register gx_device *dev,
			   int x0, int y0, int x1, int y1, gx_color_index color)

{
	int	retVal = 0;


	if (x0 == x1 && y0 == y1)
		return 0;

	if (y1 < y0)
	{
		register int	tmp;


		tmp = y1; y1 = y0; y0 = tmp;
		tmp = x1; x1 = x0; x0 = tmp;
	}

	if (color != gx_no_color_index)
	{
		BEGIN_DRAW

			if (gHasColorQD)
			{
	 			RGBColor	rgbColor;


				rgbColor.red   = RED   (color);
				rgbColor.green = GREEN (color);
				rgbColor.blue  = BLUE  (color);
				RGBForeColor (&rgbColor);
			}

			MoveTo (x0, y0);
			LineTo (x1, y1);

		END_DRAW
	}

	return retVal;
}


#define COLOR_TO_CHANNEL(s)		(s & 0xFF) * (ulong) gx_max_color_value / 255


	private gx_color_index
mac_map_rgb_color(gx_device *dev, ushort r, ushort g, ushort b)

{
	register gx_color_index index = (gx_color_index) 0;


	index  = (r * 255L) / gx_max_color_value;
	index <<= 8;
	index |= (g * 255L) / gx_max_color_value;
	index <<= 8;
	index |= (b * 255L) / gx_max_color_value;

	return (index);
}


	private int
mac_map_color_rgb (gx_device *dev,
				   register gx_color_index color, ushort prgb[3])

{
	prgb[2] = COLOR_TO_CHANNEL (color);
	color >>= 8;
	prgb[1] = COLOR_TO_CHANNEL (color);
	color >>= 8;
	prgb[0] = COLOR_TO_CHANNEL (color);
	color >>= 8;

	return 0;
}


	private int
mac_output_page (gx_device *dev, int num_copies, int flush)

{
	macClosePicture (dev);

	return 0;	/* the page is "output" as it is drawn */
}
