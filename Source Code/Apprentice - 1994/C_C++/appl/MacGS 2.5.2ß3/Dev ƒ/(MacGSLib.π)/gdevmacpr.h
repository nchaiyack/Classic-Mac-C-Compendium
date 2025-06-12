#ifndef _H_gdevmacpr
#define _H_gdevmacpr

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


#include "gx.h"				/* for gx_bitmap; includes std.h */
#include "gxdevice.h"
#include "gp_macui.h"


/* Define the Macintosh device */

typedef struct gx_device_macpr
{
	gx_device_common;

	long		magicNumber;
	GUIProcPtr	pGUIProcs;

	CWindowPtr	windowPtr;				/*	graphics window	*/
} gx_device_macpr;


/* Macro for casting gx_device argument */

#define xdev	((gx_device_macpr *) dev)


/* Procedures */

dev_proc_open_device (macpr_open);
dev_proc_get_initial_matrix (macpr_get_initial_matrix);
dev_proc_sync_output (macpr_sync);
dev_proc_output_page (macpr_output_page);
dev_proc_close_device (macpr_close);
dev_proc_map_rgb_color (macpr_map_rgb_color);
dev_proc_map_color_rgb (macpr_map_color_rgb);
dev_proc_fill_rectangle (macpr_fill_rectangle);
#if 0
dev_proc_tile_rectangle (macpr_tile_rectangle);
#endif
dev_proc_copy_mono (macpr_copy_mono);
dev_proc_copy_color (macpr_copy_color);
dev_proc_draw_line (macpr_draw_line);


#endif /* _H_gdevmacpr */
