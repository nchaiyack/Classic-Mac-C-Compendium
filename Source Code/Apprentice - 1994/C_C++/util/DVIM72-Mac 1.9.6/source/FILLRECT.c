/* -*-C-*- fillrect.h */
/*-->fillrect*/
/**********************************************************************/
/****************************** fillrect ******************************/
/**********************************************************************/
#include "dvihead.h"
#include "commands.h"
#include "gendefs.h"
#include "gblprocs.h"
#include "egblvars.h"
#include "m72.h"
#include "mac-specific.h"
#include "Scale_rect.h"


void
fillrect(x,y,width,height)
COORDINATE x,y,width,height;		/* lower left corner, size */

/***********************************************************************
With the page origin (0,0) at the lower-left corner of the bitmap,  draw
a filled rectangle at (x,y).
***********************************************************************/

{
	Rect	dest, print_dest, preview_dest;
	
	SetRect( &dest, x, YSIZE - y - height, x + width, YSIZE - y );
	if (g_draw_offscreen)
	{
		SetPort( &g_offscreen_GrafPort );
		PaintRect( &dest );
	}
	else
	{
		Scale_rect( &dest, &print_dest, &preview_dest );
		if (g_preview)
		{
			SetPort( g_page_window );
			PaintRect( &preview_dest );
		}
		SetPort( (GrafPtr) g_print_port_p );
		PaintRect( &print_dest );
	}
}

