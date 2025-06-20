/*
CenterRectInRect.c
�1991-1993 Denis G. Pelli
These routines are trivial, but enhance the readability of programs that use them.

HISTORY:
8/24/91	dgp	Made compatible with THINK C 5.0.
1/25/93 dgp removed obsolete support for THINK C 4.
*/
#include "VideoToolbox.h"

void CenterRectInRect(Rect *a,Rect *b)
{
	OffsetRect(a,(b->left + b->right - a->left - a->right)/2
		,(b->top + b->bottom - a->top - a->bottom)/2);
}

void OffsetRectTile(Rect *r,int nx,int ny) /* shift rect by multiples of the rect */
// Shift rect by multiples of the rect
{
	OffsetRect(r,nx*(r->right-r->left),ny*(r->bottom-r->top));
}

Boolean RectInRect(Rect *r,Rect *R)
// Is the first rect entirely inside the second?
// If either rect has zero area then this routine will return false.
{
	Rect t;
	
	if(!SectRect(r,R,&t))return 0;
	return EqualRect(r,&t);
}
