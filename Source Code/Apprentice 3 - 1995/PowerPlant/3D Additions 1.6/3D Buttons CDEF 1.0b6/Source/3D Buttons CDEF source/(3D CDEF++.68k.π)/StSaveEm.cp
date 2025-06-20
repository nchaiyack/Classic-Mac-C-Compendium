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
#include "StSaveEm.h"

void
StSaveEm::Save(void)
{
	mClip.Save();
	mText.Save();
	mPen.Save();
	mColor.Save();
}

void
StSaveEm::Restore(void)
{
	mClip.Restore();
	mText.Restore();
	mPen.Restore();
	mColor.Restore();
}
