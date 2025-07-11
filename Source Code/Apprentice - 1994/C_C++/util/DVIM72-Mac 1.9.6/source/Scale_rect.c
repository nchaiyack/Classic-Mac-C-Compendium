#include <stdio.h>
#include "mac-specific.h"
#include "Scale_rect.h"
#include "dvihead.h"
#include "gendefs.h"
#include "egblvars.h"
#include "m72.h"

void Scale_rect( Rect *what, Rect *for_print, Rect *for_preview )
{
	Rect	virtual_rect, print_rect;
	
	SetRect( &print_rect, 0, 0, g_printer_dpi, g_printer_dpi );
	SetRect( &virtual_rect, 0, 0, g_dpi, g_dpi );
	*for_print = *what;
	MapRect( for_print, &virtual_rect, &print_rect );
	if ((for_print->top == for_print->bottom) &&
		(what->top != what->bottom) )
		for_print->bottom ++;
	if ((for_print->left == for_print->right) &&
		(what->left != what->right) )
		for_print->right ++;
	SetRect( &virtual_rect, 0, 0,  XSIZE, YSIZE );
	*for_preview = *what;
	MapRect( for_preview, &virtual_rect, &g_page_window->portRect );
}
