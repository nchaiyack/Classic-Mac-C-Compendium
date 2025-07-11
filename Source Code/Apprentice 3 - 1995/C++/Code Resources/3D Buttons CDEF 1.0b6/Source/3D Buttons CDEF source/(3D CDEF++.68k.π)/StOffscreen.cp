/*
	Public domain by Zig Zichterman.
*/
/*
	StOffscreen
	
	An offscreen drawing environment that sets up for offscreen
	drawing on construction and copies to the screen and tears
	down on destruction
	
	Does nothing on non-colorQD systems
	
	12/29/94	zz	h	add PreDraw(), PostDraw()
	12/27/94	zz	h	initial write
*/
#include "StOffscreen.h"

#include "IsColorQDPresent.h"

StOffscreen::StOffscreen(
	const Rect &	inLocalRect)
{
	mScreenPort		= NULL;
	mScreenDevice	= NULL;
	mGWorld			= NULL;
	PreDraw(inLocalRect);
}
	
StOffscreen::~StOffscreen(void)
{
	PostDraw();
}

void
StOffscreen::PreDraw(
	const Rect &	inLocalRect)
{
	// throw out anything previous
	PostDraw();
	
	// if no color quickdraw, do nothing
	if (IsColorQDPresent() == false) {
		return;
	}
	
	// start with a blank slate
	mScreenPort		= NULL;
	mScreenDevice	= NULL;
	mGWorld			= NULL;
	
	// save the old settings
	::GetGWorld(&mScreenPort,&mScreenDevice);
	
	// get the original localbounds
	mLocalBounds.Copy(inLocalRect);
	
	// convert them to global
	mGlobalBounds.Copy(inLocalRect);
	mGlobalBounds.LocalToGlobal();
	
	// create an offscreen GWorld in temporary memory with the
	// given bounds. We promise to release the temp memory
	// before the user notices its gone.
	OSErr err = ::NewGWorld(&mGWorld,0,&mGlobalBounds,NULL,NULL,useTempMem);
	// on error
	if ((err != noErr) || (mGWorld == NULL)) {
		// do nothing, and make sure destructor does nothing
		mGWorld	= NULL;
		return;
	}
	
	// switch drawing ober to offworld city
	::SetGWorld(mGWorld, NULL);
	
	// make sure drawing code doesn't notice the difference
	::SetOrigin(mLocalBounds.left,mLocalBounds.top);
	
	// ALWAYS LockPixels() before drawing. If you ever have problems 
	// with your offscreen drawing code, checking LockPixels() is the
	// first thing you should check. At least that's been my experience...
	::LockPixels(::GetGWorldPixMap(mGWorld));
	
	// clean the slate
	::EraseRect(&mLocalBounds);
}

void
StOffscreen::PostDraw(void)
{
	// if no color quickdraw, do nothing
	if (IsColorQDPresent() == false) {
		return;
	}
	
	// if no offscreen GWorld, do nothing
	if (mGWorld == NULL) {
		return;
	}
	
	// restore drawing to the screen
	::SetGWorld(mScreenPort,mScreenDevice);
	
	// copy the gworld to the screen. Oooh. Aaah.
	::ForeColor(blackColor);
	::BackColor(whiteColor);
	::CopyBits(&((GrafPtr) mGWorld)->portBits,
				&((GrafPtr) mScreenPort)->portBits,
				&mLocalBounds,&mLocalBounds,
				srcCopy,NULL);
	
	// release the gworld. I don't think I really need to unlock
	// the pixels before disposing it, but what can it hurt?
	::UnlockPixels(::GetGWorldPixMap(mGWorld));
	::DisposeGWorld(mGWorld);
	mGWorld = NULL;
}


