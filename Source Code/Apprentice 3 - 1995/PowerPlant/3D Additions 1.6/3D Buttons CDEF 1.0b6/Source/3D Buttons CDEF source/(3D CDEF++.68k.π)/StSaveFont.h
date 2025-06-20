/*
	Public domain by Zig Zichterman.
*/
/*
	StSaveFont
	
	A class that saves the current port's font, size, style and mode on
	construction and restores on destruction
	
	12/27/94	zz	h	initial write
*/
#pragma once

class StSaveFont
{
	public :
		StSaveFont(void);
		
		~StSaveFont();
	
		void
		Save(void);
		
		void
		Restore(void) const;
		
		static void
		SystemFont(void);
		
		short	mFont;
		short	mSize;
		short	mStyle;
		short	mMode;
};
