/*
	Public domain by Zig Zichterman.
*/
/*
	StSavePen
	
	A class that saves the pen state on construction and
	restores on destruction
	
	12/27/94	zz	h	initial write
*/
#include "StSavePen.h"

StSavePen::StSavePen(void)
{
	Save();
}

StSavePen::~StSavePen()
{
	Restore();
}

void
StSavePen::Save(void)
{
	::GetPenState(&mPenState);
}

void
StSavePen::Restore(void)
{
	::SetPenState(&mPenState);
}
