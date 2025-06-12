#include "Draw_arrow.h"

extern short	line_height;

void Draw_arrow( Rect *menu_rect, short scroll_sign )
{
	register short		max_dots, scan_h, scan_v, v_gap;
	Rect			munge_rect;
	
	munge_rect = *menu_rect;
	max_dots = line_height & 0xFFFE; // round down to an even number
	scan_h = menu_rect->left + line_height;
	v_gap = (line_height - (max_dots >> 1)) >> 1;
	if (scroll_sign > 0)	// scroll triangle at bottom
	{
		munge_rect.top = munge_rect.bottom - line_height;
		scan_v = munge_rect.top + v_gap;
	}
	else					// scroll triangle at top
	{
		munge_rect.bottom = munge_rect.top + line_height;
		scan_v = munge_rect.bottom - v_gap;
	}
	EraseRect( &munge_rect );
	MoveTo( scan_h, scan_v );
	while (max_dots >= 0)
	{
		Line( max_dots, 0 );
		Move( 1-max_dots, scroll_sign );
		max_dots -= 2;
	}
}