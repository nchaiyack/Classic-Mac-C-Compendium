/*
 * TransSkel multiple-window demonstration: Region module
 *
 * This module handles a window in which the mouse may be clicked and
 * dragged to draw rectangles.  The rects so drawn are combined into
 * a single region, the outline of which is drawn.  Rects drawn while
 * the shift key is held down are subtracted from the region.
 * Double-clicking the mouse clears the display.  If the window is
 * resized, the region that is drawn is resized as well.
 *
 * BUG: should measure double-click time from mouse-up to next mouse down,
 * not time between mouse-downs.
 *
 * 21 Apr 88 Paul DuBois
 * 08 Jul 88
 * - Changed outline so that it's drawn as a marquee.
 * 29 Jan 89
 * - Conversion for TransSkel 2.0.
 * 12 Jan 91
 * - Conversion for TransSkel 3.0.
 */

# include	"TransSkel.h"

# include	"MultiSkel.h"


static Rect			rgnPortRect;	/* portRect size - for detecting wind grows */
static RgnHandle	selectRgn;		/* current region to be drawn */
static long			selectWhen;		/* time of last click */
static Point		selectWhere;	/* location of last click */

static Pattern		marqueePat = { 0x0f, 0x87, 0xc3, 0xe1, 0xf0, 0x78, 0x3c, 0x1e };


static void
MarqueeRgn (RgnHandle r)
{
PenState	p;
Byte		b;
short		i;

	GetPenState (&p);
	PenPat ((ConstPatternParam) &marqueePat);
	PenMode (patCopy);
	FrameRgn (r);
	SetPenState (&p);
	b = ((char *) &marqueePat)[0];		/* shift pattern for next call */
	for (i = 0; i < 7; ++i)
		((char *) &marqueePat)[i] = ((char *) &marqueePat)[i+1];
	((char *) &marqueePat)[7] = b;
}


/*
 * While mouse is down, draw gray selection rectangle in the current
 * port.  Return the resultant rect in dstRect.  The rect is always
 * clipped to the current portRect.
 */

static void
DoSelectRect (Point startPoint, Rect *dstRect)
{
Point		pt, dragPt;
Rect		rClip;
GrafPtr		port;
Boolean		result;
PenState	ps;
short		i;

	GetPort (&port);
	rClip = port->portRect;
	rClip.right -= 15;
	GetPenState (&ps);
	PenPat ((ConstPatternParam) &qd.gray);
	PenMode (patXor);
	dragPt = startPoint;
	Pt2Rect (dragPt, dragPt, dstRect);
	FrameRect (dstRect);
	for (;;)
	{
		GetMouse (&pt);
		if (!EqualPt (pt, dragPt))	/* mouse has moved, change region */
		{
			FrameRect (dstRect);
			dragPt = pt;
			Pt2Rect (dragPt, startPoint, dstRect);
			result = SectRect (dstRect, &rClip, dstRect);
			FrameRect (dstRect);
			for (i = 0; i < 1000; ++i) { /* empty */ }
		}
		if (!StillDown ()) break;
	}
	FrameRect (dstRect);	/* erase last rect */
	SetPenState (&ps);
}


/*
 * On double-click, clear window.  On single click, draw gray selection
 * rectangle as long as mouse is held down.  If user draws non-empty rect,
 * then add it to the selection region and redraw the region's outline.
 * If the shift-key was down, then subtract the selection region instead
 * and redraw.
 */

static pascal void
Mouse (Point pt, long t, short mods)
{
Rect		r;
RgnHandle	rgn;

	r = rgnWind->portRect;
	if (pt.h >= r.right - 15)		/* must not click in right edge */
		return;
	if (t - selectWhen <= GetDblTime ())	/* it's a double-click */
	{
		selectWhen = 0L;		/* don't take next click as double-click */
		SetWindClip (rgnWind);
		EraseRgn (selectRgn);
		ResetWindClip ();
		SetEmptyRgn (selectRgn);	/* clear region */
	}
	else
	{
		selectWhen = t;				/* update click variables */
		selectWhere = pt;
		DoSelectRect (pt, &r);	/* draw selection rectangle */
		if (!EmptyRect (&r))
		{
			EraseRgn (selectRgn);
			selectWhen = 0L;
			rgn = NewRgn ();
			RectRgn (rgn, &r);
			if ((mods & shiftKey) != 0)		/* test shift key */
				DiffRgn (selectRgn, rgn, selectRgn);
			else
				UnionRgn (selectRgn, rgn, selectRgn);
			DisposeRgn (rgn);
		}
	}
}


static pascal void
Idle (void)
{
short	i;

	SetWindClip (rgnWind);
	MarqueeRgn (selectRgn);	/* draw selection region outline */
	ResetWindClip ();		/* restore previous clipping */
}


/*
 * Redraw the current region.  If the window was resized, resize
 * the region to fit.
 */

static pascal void
Update (Boolean resized)
{
Rect	r;

	r = rgnWind->portRect;
	EraseRect (&r);
	if (resized)
	{
		rgnPortRect.right -= 15;	/* don't use right edge of window */
		r.right -= 15;
		MapRgn (selectRgn, &rgnPortRect, &r);
		rgnPortRect = rgnWind->portRect;
	}
	DrawGrowBox (rgnWind);
	Idle ();
}


static pascal void
Activate (Boolean active)
{
	DrawGrowBox (rgnWind);
	if (active)
		DisableItem (editMenu, 0);
	else
		EnableItem (editMenu, 0);
	DrawMenuBar ();
}


static pascal void
Clobber (void)
{
	DisposeRgn (selectRgn);
	DisposeWindow (rgnWind);
}


void
RgnWindInit (void)
{
	if (SkelQuery (skelQHasColorQD))
		rgnWind = GetNewCWindow (rgnWindRes, nil, (WindowPtr) -1L);
	else
		rgnWind = GetNewWindow (rgnWindRes, nil, (WindowPtr) -1L);
	if (rgnWind == (WindowPtr) nil)
		return;
	(void) SkelWindow (rgnWind,
				Mouse,		/* draw rectangles */
				nil,		/* ignore keyclicks */
				Update,
				Activate,
				nil,		/* no close proc */
				Clobber,	/* disposal proc */
				Idle,		/* idle proc */
				false);

	rgnPortRect = rgnWind->portRect;
	selectRgn = NewRgn ();	/* selected region empty initially */
	selectWhen = 0L;		/* first click can't be taken as dbl-click */
}
