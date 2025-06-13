#define	_H_RectUtils
#ifndef	__RectUtils__
#define	__RectUtils__

/* RectUtils.h
**	Macros to cut out making expensive traps for things like SetRect and SetPt!
**
**
**	History
**	=======
**	00	<tur 24-Feb-92>	Basic Version
**	01	<tur 01-May-94> Jazzed up for C++/wt
*/


#ifdef __cplusplus

	inline void QSetPt(Point *p, short x, short y)	{ p->v = y; p->h = x; }
	inline void ZeroPt(Point *p)					{ p->h = p->v = 0; }
	inline void QSetRect(Rect *rP, short l, short t, short r, short b)
		{ rP->top = t; rP->left = l; rP->bottom = b; rP->right = r; }

	inline void QInsetRect(Rect *rP, short dx, short dy)
		{ rP->top += dy; rP->left += dx; rP->bottom -= dy; rP->right -= dx; }

	inline short rectHeight(Rect *rp)	{ return rp->bottom - rp->top; }
	inline short rectWidth(Rect *rp)	{ return rp->right - rp->left; }

	inline int RectInRect(Rect *r1p, Rect *r2p)
		{ return (r1p->left >= r2p->left && r1p->right <= r2p->right &&
				  r1p->top >= r2p->top && r1p->bottom <= r2p->bottom); }

#else	/* __cplusplus */

	#define	ZeroPt(pointPtr)		*((long *)(pointPtr)) = 0
	
	#define	rectHeight(r)			((r)->bottom - (r)->top)
	#define	rectWidth(r)			((r)->right - (r)->left)

	/* Is "r1" completely enclosed in "r2"? */
	#define	RectInRect(r1p, r2p)	((r1p)->left >= (r2p)->left && (r1p)->right <= (r2p)->right && \
									 (r1p)->top >= (r2p)->top && (r1p)->bottom <= (r2p)->bottom)
 
	#define	QSetPt		SetPt
	#define	QSetRect	SetRect
	#define	QInsetRect	InsetRect

#endif	/* __cplusplus */

#define	SetPt(pointPtr, x, y)	(pointPtr)->v = (y), (pointPtr)->h = (x)
#define	SetRect(rP, l,t,r,b)	(rP)->top = t, (rP)->left = l,				\
								(rP)->bottom = b, (rP)->right = r

#define	InsetRect(rP, dx, dy)	(rP)->top += (dy), (rP)->left += (dx),		\
								(rP)->bottom -= (dy), (rP)->right -= (dx)
	

#define	ZeroRect(rP)			((long *)(rP))[0] = ((long *)(rP))[1] = 0


#ifndef InsetRectByN
	#define	InsetRectByN(rP, dxy)	InsetRect(rp, dxy, dxy)
#endif  /* InsetRectByN */


#define	EqualRect(rp1, rp2)		(((long *)(rp1))[0] == ((long *)(rp2))[0] && \
								((long *)(rp1))[1] == ((long *)(rp2))[1])

#ifndef	topLeft
	#define topLeft(rekt)		(((Point *) &(rekt))[0])
	#define botRight(rekt)		(((Point *) &(rekt))[1])	/* assume this also... */
#endif	/* topLeft */

#endif	/* __RectUtils__ */
