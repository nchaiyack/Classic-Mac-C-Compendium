/*
	Public domain by Zig Zichterman.
*/
/*
	StSaveEm
	
	A class that saves the following on construction and restores on
	destruction:
		�	clip region (StSaveClip)
		�	text state (StSaveText)
		�	pen state (StSavePen)
		�	color (StSaveColor)
	
	12/27/94	zz	h	initial write
*/
#pragma once

#include "StSaveClip.h"
#include "StSaveColor.h"
#include "StSavePen.h"
#include "StSaveFont.h"

class StSaveEm
{
	public :
		void
		Save(void);
		
		void
		Restore(void);
		
		StSaveClip	mClip;
		StSaveFont	mText;
		StSavePen	mPen;
		StSaveColor	mColor;
};
