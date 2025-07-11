/*
 * move or draw ball on screen
 */

#include <qd.h>
#include <qdvars.h>
#include "ball.h"

/*
 * There are three off-screen bitmaps.  The first two hold the images of
 * a slow ball and a fast ball.  The third is used for combining the
 * before and after pictures so that a screen update of a moving ball
 * whose new postion overlaps its old position can be done in one
 * screen operation.  This is intended to reduce flicker, by cutting
 * number of copybits calls to the screen from two to one per slow
 * moving ball.  However, it increases the total number of copybits
 * calls, which makes things run slower.
 *
 * The use of this third bitmap is enabled by defining SMOOTH.  It will
 * run ~25% slower with SMOOTH defined.
 */

char sbits[ 2*(SDIM)*((SDIM+15)/16) ];		/* slow bits */
char fbits[ 2*(SDIM)*((SDIM+15)/16) ];		/* fast bits */
#ifdef SMOOTH
char dbits[ 2*(2*SDIM)*((2*SDIM+15)/16) ];	/* combined bits */
#endif

BitMap sm, fm							/* bitmaps for slow and fast bits */
#ifdef SMOOTH
	,dm									/* and combined bits */
#endif
;

/*
 * movebits() draws two balls.  The first is in the square with corner
 * at Sx, Sy, and side SDIM.  The second is at Dx and Dy, and has the
 * same size.  Which drawing to use for each is determined by op and np.
 */
movebits( win, Sx, Sy, Dx, Dy, op, np )
	GrafPtr win;
	long Sx, Sy, Dx, Dy, op, np;
{
	register int sx, sy, dx, dy;
	int tx,ty;
	int rt, rl;
	Rect S,D;

	/*
	 * The pointers to the bits in the bitmaps may have changed if we
	 * have been moved on the heap since we were initialized, so we shall
	 * fix them.
	 */
	sm.baseAddr = sbits;
	fm.baseAddr = fbits;
#ifdef SMOOTH
	dm.baseAddr = dbits;
#endif
	
	sx = Sx; sy = Sy; dx = Dx; dy = Dy; /* convert to integers */
	tx = dx - sx;						/* relative x positions */
	ty = dy - sy;						/* relative y positions */
	
	
#define pict(v) ((v) == 0 ? &sm : &fm )

	SetPort( win );

/*
 * If the balls do not overlap, or if SMOOTH is not defined, then we
 * simply want to erase the first ball and draw the second.
 */
#ifdef SMOOTH
	/*
	 * If the balls can't possibly overlap, then just draw them directly
	 * on the screen
	 */
	if ( tx < -GRAD || tx > GRAD || ty < -GRAD || ty > GRAD ) {
#endif
		SetRect( &S, 0, 0, SDIM, SDIM );
		SetRect( &D, sx, sy, sx + SDIM, sy + SDIM );
		CopyBits( pict(op), &win->portBits, &S, &D, srcXor, 0L );
		SetRect( &D, dx, dy, dx + SDIM, dy + SDIM );
		CopyBits( pict(np), &win->portBits, &S, &D, srcXor, 0L );
		return;
#ifdef SMOOTH
	}
	
	/*
	 * The balls are close enough that we may combine their updates into
	 * one CopyBits to the screen.
	 */
	
	/*
	 * The rest of this is a pain to explain without a diagram,
	 * so figure it out for yourself!
	 */
	if ( ty > 0 )
		if ( tx > 0 )	{ rt = 0;    rl = 0; }
		else			{ rt = 0;    rl = SDIM; }
	else
		if ( tx > 0 )	{ rt = SDIM; rl = 0; }
		else			{ rt = SDIM; rl = SDIM; }
		
	SetRect( &D, 0, 0, 2*SDIM, 2*SDIM );
	CopyBits( &dm, &dm, &D, &D, srcXor, 0L );
	SetRect( &S, 0, 0, SDIM, SDIM );
	SetRect( &D, rl, rt, rl+SDIM, rt+SDIM );
	CopyBits( pict(op), &dm, &S, &D, srcCopy, 0L );
	SetRect( &D, rl + tx, rt + ty, rl + tx + SDIM, rt + ty + SDIM );
	CopyBits( pict(np), &dm, &S, &D, srcXor, 0L );
	SetRect( &S, 0, 0, SDIM*2, SDIM*2 );
	SetRect( &D, sx - rl, sy - rt, sx - rl + 2*SDIM, sy - rt + 2*SDIM );
	CopyBits( &dm, &win->portBits, &S, &D, srcXor, 0L );
#endif
}

/*
 * Initialize the bitmaps
 */	
initmove() {		
	Rect S,D;
	GrafPort port;

	/*
	 * get a grafport to do our thing in
	 */
	OpenPort( &port );
	PenNormal();
	
	/*
	 * bitmap for slow bits...
	 */
	sm.baseAddr = sbits;
	sm.rowBytes = 2*((SDIM+15)/16);
	sm.bounds.a.top = 0;
	sm.bounds.a.bottom = 2*SDIM;
	sm.bounds.a.left = 0;
	sm.bounds.a.right = 2*SDIM;
	
	/*
	 * bitmap for fast bits...
	 */
	fm.baseAddr = fbits;
	fm.rowBytes = 2*((SDIM+15)/16);
	fm.bounds.a.top = 0;
	fm.bounds.a.bottom = 2*SDIM;
	fm.bounds.a.left = 0;
	fm.bounds.a.right = 2*SDIM;
	
	SetPortBits( &sm );				/* prepare to draw slow ball */
	
	SetRect( &S, 0, 0, SDIM, SDIM );
	FrameOval( &S );
	MoveTo( 3, 7 ); LineTo( 4, 8 );	/* ...risking the taste-police */
	LineTo( 7, 8 ); LineTo( 8, 7 );
	MoveTo( 4, 4 ); LineTo( 4, 4 );
	MoveTo( 7, 4 ); LineTo( 7, 4 );

	SetPortBits( &fm );				/* prepare to draw fast ball */
	InvertOval( &S );
	
#ifdef SMOOTH
	/*
	 * bitmap for combined drawings...
	 */
	dm.baseAddr = dbits;
	dm.rowBytes = 2*((SDIM+SDIM+15)/16);
	dm.bounds.a.top = 0;
	dm.bounds.a.bottom = 2*SDIM;
	dm.bounds.a.left = 0;
	dm.bounds.a.right = 2*SDIM;
#endif	
}
