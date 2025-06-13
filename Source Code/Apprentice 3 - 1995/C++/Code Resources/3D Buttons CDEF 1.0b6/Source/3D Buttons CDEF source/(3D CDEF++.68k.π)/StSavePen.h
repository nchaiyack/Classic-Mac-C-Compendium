/*
	Public domain by Zig Zichterman.
*/
/*
	StSavePen
	
	A class that saves the pen state on construction and
	restores on destruction
	
	12/27/94	zz	h	initial write
*/
#pragma once

#include <Quickdraw.h>

class StSavePen
{
	public :
		StSavePen(void);
		
		~StSavePen();
	
		void
		Save(void);
		
		void
		Restore(void);
		
		PenState	mPenState;
};
