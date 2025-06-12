#include "Copy_banded.h"
#include "mac-specific.h"

/*
	Printing on the ImageWriter with a high font resolution and offscreen
	drawing caused blank pages to be output.  Apparently that driver
	cannot handle large bitmaps.  Hence this routine.
*/

Boolean		g_print_by_bands;

void Copy_banded( BitMap *source_map, BitMap *dest_map,
	Rect *source_rect, Rect *dest_rect )
{
	Rect	source_strip, dest_strip;
	short	source_strip_depth, dest_strip_depth;
	
	if (g_print_by_bands)
	{
		dest_strip_depth = (**g_print_rec_h).prXInfo.iBandV;
		if (dest_strip_depth <= 0)
			dest_strip_depth = 32;
		dest_strip = *dest_rect;
		dest_strip.bottom = dest_strip.top + dest_strip_depth;
		source_strip = dest_strip;
		MapRect( &source_strip, dest_rect, source_rect );
		source_strip_depth = source_strip.bottom - source_strip.top;
		while (dest_strip.top < dest_rect->bottom)
		{
			if (dest_strip.bottom > dest_rect->bottom)
			{
				dest_strip.bottom = dest_rect->bottom;
				source_strip = dest_strip;
				MapRect( &source_strip, dest_rect, source_rect );
			}
			CopyBits( source_map, dest_map,
				&source_strip, &dest_strip, srcCopy, nil );
			OffsetRect( &source_strip, 0, source_strip_depth );
			OffsetRect( &dest_strip, 0, dest_strip_depth );
		}
	}
	else
		CopyBits( source_map, dest_map,
			source_rect, dest_rect, srcCopy, nil );
}