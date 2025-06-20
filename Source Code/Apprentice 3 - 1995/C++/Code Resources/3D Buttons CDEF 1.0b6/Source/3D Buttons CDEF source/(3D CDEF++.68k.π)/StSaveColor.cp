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
#include "StSaveColor.h"

#include "IsColorQDPresent.h"

StSaveColor::StSaveColor(void)
{
	Save();
}

StSaveColor::~StSaveColor()
{
	Restore();
}

void
StSaveColor::Save(void)
{
	if (::IsColorQDPresent()) {
		::SaveFore(&mForeColor);
		::SaveBack(&mBackColor);	
	}
}

void
StSaveColor::Restore(void)
{
	if (::IsColorQDPresent()) {
		::RestoreFore(&mForeColor);
		::RestoreBack(&mBackColor);	
	}
}

