/*
 * Determine height of a window's title bar.  This is determined as the
 * difference between the top of the window's structure and content rects.
 *
 * This function will not necessarily work for windows with strange shapes
 * or that have a title bar on the side.
 */

# include	"TransSkel.h"


pascal short
SkelGetWindTitleHeight (WindowPtr w)
{
Rect	content;
Rect	structure;

	SkelGetWindContentRect (w, &content);
	SkelGetWindStructureRect (w, &structure);
	return (content.top - structure.top);
}