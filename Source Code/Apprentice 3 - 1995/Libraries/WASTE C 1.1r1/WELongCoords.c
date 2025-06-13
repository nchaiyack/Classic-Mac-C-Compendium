/*
 *	LongCoords.c
 *
 *	WASTE PROJECT
 *	Long Coordinates
 *
 *	Copyright (c) 1993-1994 Marco Piovanelli
 *	All Rights Reserved
 *
 */

#include <Types.h>
#include "WASTEIntf.h"

enum {
	kQDMin = -32767L,
	kQDMax = +32767L
};

pascal long _WEPinInRange(long value, long rangeStart, long rangeEnd)
{
	return ((value > rangeEnd) ? rangeEnd : ((value < rangeStart) ? rangeStart : value));
}

pascal void WELongPointToPoint(const LongPt *lp, Point *p)
{
	p->v = (short) _WEPinInRange(lp->v, kQDMin, kQDMax);
	p->h = (short) _WEPinInRange(lp->h, kQDMin, kQDMax);
}

pascal void WEPointToLongPoint(Point p, LongPt *lp)
{
	lp->v = (long) p.v;
	lp->h = (long) p.h;
}

pascal void WESetLongRect(LongRect *lr, long left, long top, long right, long bottom)
{
	lr->top = top;
	lr->left = left;
	lr->bottom = bottom;
	lr->right = right;
}

pascal void WELongRectToRect(const LongRect *lr, Rect *r)
{
	WELongPointToPoint((const LongPt *) lr, (Point *) r);
	WELongPointToPoint((const LongPt *) lr + 1, (Point *) r + 1);
}

pascal void WERectToLongRect(const Rect *r, LongRect *lr)
{
	WEPointToLongPoint(*((const Point *) r), (LongPt *) lr);
	WEPointToLongPoint(*((const Point *) r + 1), (LongPt *) lr + 1);
}

pascal void WEOffsetLongRect(LongRect *lr, long hOffset, long vOffset)
{
	lr->top += vOffset;
	lr->left += hOffset;
	lr->bottom += vOffset;
	lr->right += hOffset;
}

pascal Boolean WELongPointInLongRect(const LongPt *lp, const LongRect *lr)
{
	return ((lp->v >= lr->top) && (lp->h >= lr->left) && (lp->v < lr->bottom) && (lp->h < lr->right));
}
