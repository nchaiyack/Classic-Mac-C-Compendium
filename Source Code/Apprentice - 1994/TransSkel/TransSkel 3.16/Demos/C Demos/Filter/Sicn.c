/*
 * SICN stuff
 */

# include	"Sicn.h"


/*
 * Plot the (i)th SICN item in the SICN resource.  Have to lock the thing,
 * then pass the correct item to the item plotter.
 */

void
PlotSicn (Rect *r, SicnHandle s, short i)
{
SignedByte	state;

	state = HGetState ((Handle) s);
	HLock ((Handle) s);
    PlotSitm (r, &((**s)[i]));
	HSetState ((Handle) s, state);
}


/*
 * Plot sicn item in given rectangle.  This is exactly analogous to
 * PlotIcon.  The item must not belong to a movable hunk of memory
 * (like an unlocked handle).
 */

void
PlotSitm (Rect *r, Sitm *s)
{
GrafPtr	port;
BitMap	bm;

    /*  create a small bitmap */
    bm.baseAddr = (Ptr) s;
    bm.rowBytes = sitmSize / 8;       /* items are 2 bytes wide */
    SetRect (&bm.bounds, 0, 0, sitmSize, sitmSize);
    GetPort (&port);
    CopyBits (&bm, &port->portBits, &bm.bounds, r, srcCopy, nil);
}
