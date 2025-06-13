/*
	Public domain by Zig Zichterman.
*/
/*
	StOffscreen
	
	An offscreen drawing environment that sets up for offscreen
	drawing on construction and copies to the screen and tears
	down on destruction
	
	Does nothing on non-colorQD systems
	
	12/29/94	zz	h	add PreDraw(), PostDraw()
	12/27/94	zz	h	initial write
*/
#pragma once

#include <QDOffscreen.h>
#include "RectT.h"

class StOffscreen
{
	public :
		StOffscreen(
			const Rect &	inLocalRect);
		
		~StOffscreen(void);
	
		void
		PreDraw(
			const Rect &	inLocalRect);
		
		void
		PostDraw(void);
		
	protected :
		CGrafPtr	mScreenPort;
		GDHandle	mScreenDevice;
		RectT		mGlobalBounds;
		RectT		mLocalBounds;
		GWorldPtr	mGWorld;
};
