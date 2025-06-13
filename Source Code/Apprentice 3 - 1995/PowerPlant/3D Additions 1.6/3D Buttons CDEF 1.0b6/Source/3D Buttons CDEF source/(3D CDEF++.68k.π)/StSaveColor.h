/*
	Public domain by Zig Zichterman.
*/
/*
	StSaveColor
	
	A class that save the current grafport's foreground and background
	colors on construction and restores them on destruction.
	
	Does nothing if color quickdraw is absent
	
	12/27/94	zz	h	initial write
*/
#pragma once

#include <Palettes.h>

class StSaveColor
{
	public :
		StSaveColor(void);
		
		~StSaveColor();
		
		void
		Save(void);
		
		void
		Restore(void);
		
		ColorSpec	mForeColor;
		ColorSpec	mBackColor;
};
