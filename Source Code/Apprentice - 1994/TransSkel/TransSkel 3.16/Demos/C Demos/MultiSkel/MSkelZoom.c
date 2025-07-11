/*
 * TransSkel multiple-window demonstration: ZoomRect module
 * 
 * This module handles a window in which successive randomly generated
 * rectangles are smoothly interpolated into one another.  The display
 * is white on black, which results in some interesting problems (see
 * ZDrawGrowBox, for instance).  The display adjusts itself to the size
 * of the window, so that the zoom series always lie entirely within
 * the window.  Clicking the mouse in the window pauses the display until
 * the button is released.
 * 
 * 21 Apr 88 Paul DuBois
 * 29 Jan 89
 * - Conversion for TransSkel 2.0.
 * 12 Jan 91
 * - Conversion to TransSkel 3.0.  Turning on prototyping uncovered
 * a couple of (harmless) argument-passing mistakes.  Ahem.
 * 20 Dec 93
 * - Fixed grow box drawing so it draws correctly in white-on-black
 * even when system normally draws color-tinged grow boxes.
 */

# include	"TransSkel.h"

# include	"MultiSkel.h"


# define	zoomSteps	15		/* # rects in interpolative series */


static void ZDrawGrowBox (WindowPtr wind);


static Rect		zRect[zoomSteps];	/* set of interpolated rectangles */
static Rect		zSrcRect;
static short	sizeX;				/* size of window in pixels */
static short	sizeY;


static void
SetZoomSize (void)
{
Rect	r;

	r = zoomWind->portRect;
	r.right -= 15;				/* don't use right edge */
	sizeX = r.right;
	sizeY = r.bottom;
}


/*
 * return integer between zero and max (inclusive).  assumes max is
 * non-negative.
 */

static short
Rand (short max)
{
short	t;

	t = Random ();
	if (t < 0) t = -t;
	return (t % (max + 1));
}


/*
 * Interpolate one rectangle smoothly into another.  Erase the previous
 * series as the new one is drawn.
 */

static void
ZoomRect (Rect *r1, Rect *r2)
{
short	r1left, r1top;
short	l, t;
short	j;
short	hDiff, vDiff, widDiff, htDiff;
short	r, b;
short	rWid, rHt;

	r1left = r1->left;
	r1top = r1->top;
	hDiff = r2->left - r1left;	/* positive if moving to right */
	vDiff = r2->top - r1top;		/* positive if moving down */
	rWid = r1->right - r1left;
	rHt = r1->bottom - r1top;
	widDiff = (r2->right - r2->left) - rWid;
	htDiff = (r2->bottom - r2->top) - rHt;
	/*
	 * order of evaluation is important in the rect coordinate calculations.
	 * since all arithmetic is integer, you can't save time by calculating
	 * j/zoomSteps and using that - it'll usually be zero.
	 */
	for (j = 1; j <= zoomSteps; j++)
	{
		FrameRect (&zRect[j-1]);				/* erase a rectangle */
		l = r1left + (hDiff * j) / zoomSteps;
		t = r1top + (vDiff * j) / zoomSteps;
		r = l + rWid + (widDiff * j) / zoomSteps;
		b = t + rHt + (htDiff * j) / zoomSteps;
		SetRect (&zRect[j-1], l, t, r, b);
		FrameRect (&zRect[j-1]);
	}
}


/*
 * Draw the grow box in white on black.  I used to do this by drawing the box and
 * inverting it, but that doesn't work under System 7 which uses color tinges.
 * Now I just draw the squares directly.  Ugh.
 */

static void
ZDrawGrowBox (WindowPtr wind)
{
Rect	r;

	r = wind->portRect;
	r.left = r.right - 15;
	r.top = r.bottom - 15;
	++r.right;
	++r.bottom;
	EraseRect (&r);
	FrameRect (&r);
	if (((WindowPeek) wind)->hilited)
	{
		r.right -= 6;
		r.bottom -= 6;
		OffsetRect (&r, 4, 4);
		FrameRect (&r);
		r.right -= 3;
		r.bottom -= 3;
		OffsetRect (&r, -1, -1);
		EraseRect (&r);
		FrameRect (&r);
	}
}


/*
 * just pause zoom display while mouse down
 */

static pascal void
Mouse (Point pt, long t, short mods)
{
	while (StillDown ()) {  /* wait until mouse button released */ }
}


static pascal void
Update (Boolean resized)
{
short	i;

	EraseRect (&zoomWind->portRect);
	ZDrawGrowBox (zoomWind);
	SetWindClip (zoomWind);
	for (i = 0; i < zoomSteps; ++i)
		FrameRect (&zRect[i]);
	ResetWindClip ();
	if (resized)
		SetZoomSize ();		/* adjust to new window size */
}


static pascal void
Activate (Boolean active)
{

	ZDrawGrowBox (zoomWind);
	if (active)
		DisableItem (editMenu, 0);
	else
		EnableItem (editMenu, 0);
	DrawMenuBar ();
}


static pascal void
Clobber (void)
{
	DisposeWindow (zoomWind);
}


static pascal void
Idle (void)
{
short	i;
Point	pt1, pt2;
Rect	dstRect;

	SetPt (&pt1, Rand (sizeX), Rand (sizeY));	/* generate new rect */
	SetPt (&pt2, Rand (sizeX), Rand (sizeY));	/* and zoom to it */
	Pt2Rect (pt1, pt2, &dstRect);
	SetWindClip (zoomWind);			/* don't draw in right edge */
	ZoomRect (&zSrcRect, &dstRect);
	ResetWindClip ();
	zSrcRect = dstRect;
}


void
ZoomWindInit (void)
{
short	i;

	if (SkelQuery (skelQHasColorQD))
		zoomWind = GetNewCWindow (zoomWindRes, nil, (WindowPtr) -1L);
	else
		zoomWind = GetNewWindow (zoomWindRes, nil, (WindowPtr) -1L);
	if (zoomWind == (WindowPtr) nil)
		return;
	(void) SkelWindow (zoomWind,
				Mouse,		/* pause while button down */
				nil,		/* ignore key clicks */
				Update,
				Activate,
				nil,		/* no close proc */
				Clobber,	/* disposal proc */
				Idle,		/* draw a new series */
				false);

	SetZoomSize ();
	BackPat ((ConstPatternParam) &black);
	PenMode (patXor);
	SetRect (&zSrcRect, 0, 0, 0, 0);
	for (i = 0; i < zoomSteps; ++i)		/* initialize rect array */
		zRect[i] = zSrcRect;
}
