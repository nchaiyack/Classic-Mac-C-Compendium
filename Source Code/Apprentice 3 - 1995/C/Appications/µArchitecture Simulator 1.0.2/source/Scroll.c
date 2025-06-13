/*
Copyright й 1993,1994,1995 Fabrizio Oddone
еее еее еее еее еее еее еее еее еее еее
This source code is distributed as freeware:
you may copy, exchange, modify this code.
You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/

//#pragma load "MacDump"

#include	"UtilsSys7.h"
#include	"Scroll.h"

#if defined(FabSystem7orlater)

//#pragma segment Main

/* VScrollRect: ultra-fast vertical ScrollRect which does not fill
with the background pattern; useful when drawing in srcCopy */

void VScrollRect(RectPtr r, short dv)
{
GrafPtr	port;
Rect	src,dst;
register short	left,top,right,bottom;

if (dv != 0) {
	GetPort(&port);
	left	= (*port->visRgn)->rgnBBox.left;
	top		= (*port->visRgn)->rgnBBox.top;
	right	= (*port->visRgn)->rgnBBox.right;
	bottom	= (*port->visRgn)->rgnBBox.bottom;
	if (r->left < left)		r->left = left;
	if (r->top < top)		r->top = top;
	if (r->right > right)	r->right = right;
	if (r->bottom > bottom)	r->bottom = bottom;
	
	if (dv > 0) {
		topLeft(src) = topLeft(*r);
		src.right	= r->right;
		src.bottom	= r->bottom - dv;
		
		dst.left	= r->left;
		dst.top		= r->top + dv;
		botRight(dst) = botRight(*r);
		}
	else {
		src.left	= r->left;
		src.top		= r->top - dv;
		botRight(src) = botRight(*r);
		
		topLeft(dst) = topLeft(*r);
		dst.right	= r->right;
		dst.bottom	= r->bottom + dv;
		}
	CopyBits(&port->portBits, &port->portBits, &src, &dst, srcCopy, (RgnHandle)nil);
	}
}

#endif

