/*
	Public domain by Zig Zichterman.
*/
/*
	RectT
	
	C++ wrappers for Rect
	
	12/29/94	zz	h	rename from Wrappers
	12/20/94	zz	h	initial write
*/
#include "RectT.h"

#ifndef	topLeft
#define topLeft(r)	(((Point *) &(r))[0])
#endif	// topLeft

#ifndef	botRight
#define botRight(r)	(((Point *) &(r))[1])
#endif	// botRight

/**************************************************************************
	RectT(void)											[public]
	
	Create a rect with bounds {0,0,0,0}
**************************************************************************/
RectT::RectT(void)
{
	left = top = right = bottom = 0;
}

/**************************************************************************
	RectT(shorts)										[public]
	
	Create a rect with the given bounds. I use "Like The Red Baron" to
	remember the parameter order.
**************************************************************************/
RectT::RectT(
	short	inLeft,
	short	inTop,
	short	inRight,
	short	inBottom)
{
	left	= inLeft;
	top		= inTop;
	right	= inRight;
	bottom	= inBottom;
}

/**************************************************************************
	RectT::Copy()										[public]
	
	Copy a rectangle into this RectT
**************************************************************************/
void
RectT::Copy(
	const Rect &	inRect)
{
	left	= inRect.left;
	top		= inRect.top;
	right	= inRect.right;
	bottom	= inRect.bottom;
}

/**************************************************************************
	RectT::LocalToGlobal()								[public]
	
	Convert a rect from local to global coordinates
**************************************************************************/
void
RectT::LocalToGlobal(void)
{
	::LocalToGlobal(&topLeft(*this));
	::LocalToGlobal(&botRight(*this));
}

/**************************************************************************
	RectT::GlobalToLocal()								[public]
	
	Convert a rect from global to local coordinates
**************************************************************************/
void
RectT::GlobalToLocal(void)
{
	::GlobalToLocal(&topLeft(*this));
	::GlobalToLocal(&botRight(*this));
}

