/*

		Regions.c++
		by Hiep Dam, 3G Software.
		March 1994.
		Last Update April 1994
		Contact: America Online: Starlabs
		         Delphi        : StarLabs
		         Internet      : starlabs@aol.com
		                      or starlabs@delphi.com

		Contains routines that deal with pictures and regions, and the
		conversion of picts to regions.

*/


#include "Regions.h"
#include "QDUtils.h"

// -----------------------------------------------------------------------------

// Globals, private to this file.
static GrafPtr gOffscreenBuffer = nil;		// Offscreen grafport buffer we'll draw to.
static ResType gRgnType = 'Rgn ';			// Default resource type of our regions.
static Rect gBufferRect = { 0, 0, 0, 0 };	// Rect of the offscreen grafport buffer.

// -----------------------------------------------------------------------------

// Private routine to this file.
OSErr CreateOffscreenBuffer(Rect *bufferRect);

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// Initialize the Region-making routines in this file.
// Allocates an offscreen, black and white grafport buffer (so we can
// draw pictures onto it and then convert it to regions).

// IN-> Size of offscreen grafport; preferably size of largest monitor (or
//      largest region you'll be making.)
//      Also specify the rgn resource type (by default its 'Rgn ').
// OUT-> Error code, if any.

OSErr InitRegionMaker(Rect *screenSize, ResType rgnType) {
	gRgnType = rgnType;
	gBufferRect = *screenSize;
	return(CreateOffscreenBuffer(screenSize));
} // END InitRegionMaker

// -----------------------------------------------------------------------------

// DataFromScrap.
// Given a specified resource type, try to fetch that resource lying in the
// clipboard. If none exists, return nil. Allocates memory for that resource.
// IN-> Specify a resource type, such as 'PICT', 'TEXT', etc.
// OUT-> Returns a handle to specified resource type, if it exists. Else
//		 returns nil.

Handle DataFromScrap(ResType dataType) {
	Handle dataHandle;
	long scrapOffset, rc;

	// First, allocate a valid 0-length handle (required by _GetScrap())
	dataHandle = NewHandle(0);

	// Now, just get size of data; do this by passing a handle with a size of 0.
	rc = GetScrap(dataHandle, dataType, &scrapOffset );
	if (rc < 0) {
		// Data type not in scrap so... 
		return(nil);
	}
	else {
		// Yeah, our resource type is there. Increase handle size to actual
		// size of our requested resource.
		SetHandleSize(dataHandle, rc);
		// Let's get it.
		rc = GetScrap(dataHandle, dataType, &scrapOffset);
		return(dataHandle);
	}
} // END DataFromScrap

// -----------------------------------------------------------------------------

// PictureFromScrap.
// Just calls DataFromScrap(), requesting type 'PICT' and type-casting
// the return value to PicHandle.

PicHandle PictureFromScrap() {
	return((PicHandle)DataFromScrap('PICT'));
} // END PictureFromScrap

// -----------------------------------------------------------------------------

// RegionToScrap.
// Puts the specified region onto the clipboard. Region must be valid.
// Returns an error code if anything went wrong (like not enough memory).
// IN-> A valid region, preferably _HLocked.
// OUT-> Error value, 0 if everything went OK.

OSErr RegionToScrap(RgnHandle theRgn) {
	long scrapErr;

	(void)ZeroScrap();
	scrapErr = PutScrap(GetHandleSize((Handle)theRgn), gRgnType, (Ptr)*theRgn);
	return(scrapErr);
} // END RegionToScrap

// -----------------------------------------------------------------------------

// ConvertPictureToRegion.
// Here's the actual "engine" or "core" of Regions.c++. Given a handle to
// a valid picture, convert that into a region and return that region. This
// routine will allocate the region itself, and calls _BitMapToRegion() to do
// the actual dirty work.
// How it's done: The picture is drawn onto the black and white offscreen
// grafport buffer, and the trap _BitMapToRegion() is called. Thus the picture
// can be color as well, but is converted to black and white when it's drawn
// to the black and white grafport...
// Note that you must allocate the black and white grafport FIRST before calling
// this routine; do that by calling InitRegionMaker().

// IN-> Valid pichandle. Need not be _HLocked by caller.
// OUT-> Valid region. If error occurred, will return nil.

RgnHandle ConvertPictureToRegion(PicHandle thePicture, Boolean doOffset) {
	GrafPtr savePort;
	char picState;
	RgnHandle returnRgn = NewRgn();

	GetPort(&savePort);

	if (gOffscreenBuffer == nil) {
		// Hmm. Caller forgot to allocate offscreen grafport. Exit!
		DisposeRgn(returnRgn);
		return(nil);
	}

	SetPort(gOffscreenBuffer);

	// Erase any previous pictures drawn onto this grafport, if any
	FillRect(&gOffscreenBuffer->portRect, white);

	// Get handle state. (Is it locked? purgeable? etc.)
	picState = HGetState((Handle)thePicture);
	HLockHi((Handle)thePicture);
	DrawPicture(thePicture, &(**thePicture).picFrame);
	// Restore handle state
	HSetState((Handle)thePicture, picState);

	// Let's do it!
	if (BitMapToRegion(returnRgn, &gOffscreenBuffer->portBits) != noErr) {
		DisposeRgn(returnRgn);
		SetPort(savePort);
		return(nil);
	}

	// Just in case the region isn't located starting at 0, 0, we offset the
	// region so it's topleft corner is at 0, 0.
	if (doOffset)
		OffsetRgn(returnRgn, -(**returnRgn).rgnBBox.left, -(**returnRgn).rgnBBox.top);

	SetPort(savePort);

	return(returnRgn);
} // END ConvertPictureToRegion

// -----------------------------------------------------------------------------

// CreateOffscreenBuffer.
// Does the dirty work of creating an offscreen b&w grafport. Actually calls
// NewOffScreenBitMap() to do it, located in QDUtils.c++.

OSErr CreateOffscreenBuffer(Rect *bufferRect) {
	GrafPtr savePort;
	short grafErr;

	GetPort(&savePort);

	if(!NewOffScreenBitMap(&gOffscreenBuffer, bufferRect, 0))
		grafErr = -1;
	else
		grafErr = noErr;

	SetPort(savePort);
	return(grafErr);
} // END CreateOffscreenBuffer

// -----------------------------------------------------------------------------

// SetRegionType and GetRegionType.
// Obvious so I'll not comment them further.

void SetRegionType(ResType rgnType) {
	gRgnType = rgnType;
} // END SetRegionType

ResType GetRegionType() {
	return(gRgnType);
} // END GetRegionType

// -----------------------------------------------------------------------------

// PreviewRgn.
// So you want to see what your region's gonna look like, huh? This
// routine creates its own window, centered on the main screen.

// IN-> Valid region.
// OUT-> None.

void PreviewRgn(RgnHandle theRgn) {
	GrafPtr savePort;
	WindowPtr rgnWindow;
	Rect rgnRect;
	EventRecord dummyEvent;
	
	if (theRgn == nil)
		return;
		
	rgnRect = (**theRgn).rgnBBox;
	
	// Just in case the region isn't flushed topleft at 0, 0, make
	// some adjustments to the window rect...
	if (rgnRect.left != 0) {
		rgnRect.right += rgnRect.left;
		rgnRect.left = 0;
	}
	if (rgnRect.top != 0) {
		rgnRect.bottom += rgnRect.top;
		rgnRect.top = 0;
	}

	GetPort(&savePort);

	// If the region is really tiny, let's not create a really tiny window.
	// Instead make a small window, but large enough NOT to cause eyestrain.
	// We'll arbitrarily set the region size limit to 20 pixels; any smaller
	// and we create our window, else use a window based on the size of
	// that region.
	if ((rgnRect.right - rgnRect.left < 20) || (rgnRect.bottom - rgnRect.top < 20))
		SetRect(&rgnRect, 0, 0, 24, 24);

	// Center the region's rect based on the rect of the offscreen
	// grafport buffer. This call resides in QDUtils.c++
	CenterRect(&rgnRect, &gBufferRect);
	// Make our window.
	rgnWindow = NewWindow(nil, &rgnRect, "\pPreview", false, noGrowDocProc,
		(WindowPtr)-1, false, 0);

	SetPort(rgnWindow);
	ShowWindow(rgnWindow);

	// Fill it.
	FillRgn(theRgn, black);

	// Wait for a mouseDown or a keyDown.
	do {
		if (GetNextEvent(everyEvent, &dummyEvent))
			if (dummyEvent.what == mouseDown || dummyEvent.what == keyDown)
				break;
	} while (!Button());
	// Flush any residual mouse clicks
	FlushEvents(everyEvent, 0);

	// Clean up our act.
	DisposeWindow(rgnWindow);
	SetPort(savePort);
} // END PreviewRgn




// END Regions.c++