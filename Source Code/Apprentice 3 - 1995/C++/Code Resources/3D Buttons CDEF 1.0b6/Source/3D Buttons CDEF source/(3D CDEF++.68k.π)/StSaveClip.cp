/*
	Public domain by Zig Zichterman.
*/
/*
	StSaveClip
	
	A class that saves the clip region on construction and restores
	it on destruction
	
	12/27/94	zz	h	initial write
*/
#include "StSaveClip.h"

StSaveClip::StSaveClip(void)
{
	Save();
}

StSaveClip::~StSaveClip()
{
	Restore();
}

void
StSaveClip::Save(void)
{
	mRgn.CopyFromClip();
}


void
StSaveClip::Restore(void)
{
	mRgn.CopyToClip();
}
