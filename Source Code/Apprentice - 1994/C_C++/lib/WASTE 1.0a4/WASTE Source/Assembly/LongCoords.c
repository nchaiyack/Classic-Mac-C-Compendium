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

enum {
	kQDMin = -32767,
	kQDMax = +32767
};

typedef struct LongPoint {
	long v;
	long h;
} LongPoint;

typedef struct LongRect {
	long top;
	long left;
	long bottom;
	long right;
} LongRect;

pascal long _WEPinInRange(long value, long rangeStart, long rangeEnd);
pascal void WELongPointToPoint(LongPoint *lp, Point *p);
pascal void WEPointToLongPoint(Point p, LongPoint *lp);
pascal void WESetLongRect(LongRect *lr, long left, long top, long right, long bottom);
pascal void WELongRectToRect(LongRect *lr, Rect *r);
pascal void WERectToLongRect(Rect *r, LongRect *lr);
pascal void WEOffsetLongRect(LongRect *lr, long hOffset, long vOffset);
pascal Boolean WELongPointInLongRect(LongPoint *lp, LongRect *lr);

pascal long _WEPinInRange(long value, long rangeStart, long rangeEnd)
{
	asm {
		movem.l rangeEnd, d0-d2	; d0 = rangeEnd, d1 = rangeStart, d2 = value
		cmp.l d0, d2			; value > rangeEnd?
		bgt.s @exit				; yes, return rangeEnd
		move.l d1, d0			; 
		cmp.l d0, d2			; value < rangeStart?
		blt.s @exit				; yes, return rangeStart
		move.l d2, d0			; return value
@exit
	}
}

pascal void WELongPointToPoint(register LongPoint *lp, register Point *p)
{
	p->v = (short) _WEPinInRange(lp->v, kQDMin, kQDMax);
	p->h = (short) _WEPinInRange(lp->h, kQDMin, kQDMax);
}

pascal void WEPointToLongPoint(Point p, register LongPoint *lp)
{
	lp->v = (long) p.v;
	lp->h = (long) p.h;
}

pascal void WESetLongRect(LongRect *lr, long left, long top, long right, long bottom)
{
	asm {
		movea.l lr, a0
		move.l top, (a0)+
		move.l left, (a0)+
		move.l bottom, (a0)+
		move.l right, (a0)+
	}
}

pascal void WELongRectToRect(register LongRect *lr, register Rect *r)
{
	WELongPointToPoint((LongPoint *) lr, (Point *) r);
	WELongPointToPoint((LongPoint *) lr + 1, (Point *) r + 1);
}

pascal void WERectToLongRect(register Rect *r, register LongRect *lr)
{
	WEPointToLongPoint(*((Point *) r), (LongPoint *) lr);
	WEPointToLongPoint(*((Point *) r + 1), (LongPoint *) lr + 1);
}

pascal void WEOffsetLongRect(LongRect *lr, long hOffset, long vOffset)
{
	asm {
		movea.l lr, a0
		movem.l vOffset, d0/d1	; d0 = vOffset, d1 = hOffset
		add.l d0, (a0)+			; lr->top += vOffset
		add.l d1, (a0)+			; lr->left += hOffset
		add.l d0, (a0)+			; lr->bottom += vOffset
		add.l d1, (a0)+			; lr->right += hOffset
	}
}

pascal Boolean WELongPointInLongRect(LongPoint *lp, LongRect *lr)
{
	asm {
		moveq #0, d0			; assume function result is FALSE
		move.l lp, a0			; 
		movem.l (a0)+, d1/d2	; d1 = lp.v, d2 = lp.h
		move.l lr, a0			; 
		cmp.l (a0)+, d1			; lp.v < lr.top ?
		blt.s @exit				; 
		cmp.l (a0)+, d2			; lp.h < lr.left ?
		blt.s @exit				; 
		cmp.l (a0)+, d1			; lp.v >= lr.bottom ?
		bge.s @exit				; 
		cmp.l (a0)+, d2			; lp.h >= lr.right ?
		bge.s @exit				; 
		moveq #1, d0			; set function result to TRUE
@exit
	}
}
