/*
	Public domain by Zig Zichterman.
*/
/*
	StSaveFont
	
	A class that saves the current port's font, size, style and mode on
	construction and restores on destruction
	
	12/27/94	zz	h	initial write
*/
#include "StSaveFont.h"

#include <QuickDraw.h>

StSaveFont::StSaveFont(void)
{
	// init to something boring
	mFont = mSize = mStyle = mMode = 0;
	
	// save the current port's font settings
	Save();
}

StSaveFont::~StSaveFont()
{
	// restore the current port to what it was when we last Save()d
	Restore();
}

void
StSaveFont::Save(void)
{
	// get the current port
	GrafPtr	currentPort	= NULL;
	::GetPort(&currentPort);
	if (currentPort == NULL) {
		return;
	}
	
	// get the text settings
	mFont	= currentPort->txFont;
	mSize	= currentPort->txSize;
	mStyle	= currentPort->txFace;
	mMode	= currentPort->txMode;
	
}

void
StSaveFont::Restore(void) const
{
	// restore 'em
	::TextFont(mFont);
	::TextSize(mSize);
	::TextFace(mStyle);
	::TextMode(mMode);
}

/**************************************************************************
	SystemFont()									[public, static]
	
	Force the current port's font to system font, default size,
	plain, srcCopy. 
**************************************************************************/
void
StSaveFont::SystemFont(void)
{
	::TextFont(0);
	::TextSize(0);
	::TextFace(0);
	::TextMode(srcCopy);
}
