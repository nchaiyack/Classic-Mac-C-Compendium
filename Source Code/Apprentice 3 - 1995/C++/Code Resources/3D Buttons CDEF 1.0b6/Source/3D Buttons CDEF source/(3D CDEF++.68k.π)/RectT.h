/*
	Public domain by Zig Zichterman.
*/
/*
	RectT
	
	C++ wrappers for Toolbox structures
	
	12/29/94	zz	h	rename from Wrappers
	12/20/94	zz	h	initial write
*/
#pragma once

#include <Quickdraw.h>

/**************************************************************************
	RectT
	
	A wrapper for Rect structures. Default constructor creates a rect
	with bounds {0,0,0,0}, parameter constructor initializes the rect
	with the supplied bounds.
**************************************************************************/
struct RectT :
	public Rect
{
	RectT(void);
	
	RectT(
		short	inLeft,
		short	inTop,
		short	inRight,
		short	inBottom);
	
	RectT(	
		const Rect &	inRect)
		{	Copy(inRect);	}
		
	void
	Copy(
		const Rect &	inRect);
		
	void
	LocalToGlobal(void);
	
	void
	GlobalToLocal(void);
};

