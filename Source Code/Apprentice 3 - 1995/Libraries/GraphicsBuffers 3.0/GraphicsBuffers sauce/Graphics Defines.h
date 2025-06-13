
/*
	Some useful (and others not so useful) #defines...
*/

#define MIN_UTIL(a, b) ((a) < (b) ? (a) : (b))
#define MAX_UTIL(a, b) ((a) > (b) ? (a) : (b))

// ---------------------------------------------------------------------------

/*
	Use FastOffsetRect instead of OffsetRect; this is
	faster (it doesn't incur as much overhead to a trap call).
	This macro, and the ones like it below, will however
	bloat your code slightly.
	Unlike OffsetRect, don't "pass" the address of the rect;
	pass the rect itself...
*/
#define FastOffsetRect(r, x, y)	\
	r.left += x;				\
	r.right += x;				\
	r.top += y;					\
	r.bottom += y

// ---------------------------------------------------------------------------

/*
	Again, like FastOffsetRect, use this for
	speedier results. Much better than
	UnionRect.
*/
#define FastUnionRect(s, d, u)				\
	u.top    = MIN_UTIL(s.top, d.top);		\
	u.left   = MIN_UTIL(s.left, d.left);	\
	u.bottom = MAX_UTIL(s.bottom, d.bottom);\
	u.right  = MAX_UTIL(s.right, d.right)

// ---------------------------------------------------------------------------

#define FastOffsetPt(pt, x, y)	\
	pt.h += x;					\
	pt.v += y

// ---------------------------------------------------------------------------

/*
	Well, not so much faster. More convenient, actually...
	Argument "rgn" in the macro should be a RgnHandle.
*/
#define FastMoveRgnTo(rgn, x, y)	\
	OffsetRgn(rgn, x - (**rgn).rgnBBox.left, y - (**rgn).rgnBBox.top)
