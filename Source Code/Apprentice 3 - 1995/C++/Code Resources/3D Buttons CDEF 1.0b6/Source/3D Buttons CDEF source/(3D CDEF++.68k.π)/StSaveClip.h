/*
	Public domain by Zig Zichterman.
*/
/*
	StSaveClip
	
	A class that saves the clip region on construction and restores
	it on destruction
	
	12/27/94	zz	h	initial write
*/
#pragma once

#include "RgnHandleT.h"

class StSaveClip
{
	public :
		StSaveClip(void);
		
		~StSaveClip();
	
		void
		Save(void);
		
		void
		Restore(void);

		RgnHandleT	mRgn;		
};