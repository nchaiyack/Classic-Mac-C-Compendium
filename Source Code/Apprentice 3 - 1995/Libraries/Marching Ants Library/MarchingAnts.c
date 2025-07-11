//=====================================================================================
// MarchingAnts.c -- written by Aaron Giles
// Last update: 7/7/94 (version 1.1)
//=====================================================================================
// A source code library for handling rectangular graphical selections involving the
// famous "marching ants".  See the header file for a brief description of each
// function; see the prelude to each function definition below for a full description
// of its operation and expected use.
//=====================================================================================
// This code has been compiled successfully under MPW C, MPW PPCC, THINK C, and
// Metrowerks C/C++, both 68k and PowerPC.  This code has been compiled under both the
// Universal Headers and the old 7.1 headers; if it works earlier than that I cannot
// say for sure.
//=====================================================================================
// If you find any bugs in this source code, please email me and I will attempt to fix
// them.  If you have any additions/modifications that you think would be generally
// useful, email those to me as well, and I will consider incorporating them into the
// next release.  My email address is giles@med.cornell.edu.
//=====================================================================================
// This source code is copyright � 1994, Aaron Giles.  Permission to use this code in
// your product is freely granted, provided that you credit me appropriately in your
// application's About box/credits *and* documentation.  If you ship an application
// which uses this code, I would also like to request that you provide me with one
// complimentary copy of the application.
//=====================================================================================

//=====================================================================================
// Include standard MacOS definitions.
//=====================================================================================

#include <Errors.h>
#include <Events.h>
#include <Memory.h>
#include <QuickDraw.h>
#include <Types.h>
#include <Windows.h>

//=====================================================================================
// Include our own external definitions.
//=====================================================================================

#include "MarchingAnts.h"

//=====================================================================================
// Prototypes for internal functions local to this module.
//=====================================================================================

static OSErr DrawAnts(AntsReference theAnts);
static OSErr EraseAnts(AntsReference theAnts);
static OSErr MarchAnts(AntsReference theAnts);
static void SaveToGWorlds(AntsReference theAnts);
static void RestoreFromGWorlds(AntsReference theAnts);
static void CopyToCurrentPort(GWorldPtr srcGWorld, Rect *srcRect, Rect *dstRect);
static void CopyFromCurrentPort(GWorldPtr dstGWorld, Rect *srcRect, Rect *dstRect);

//=====================================================================================
// Internal global variable declarations.
//=====================================================================================

static Pattern gAnimatedAntsPatternTop =
			{ 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0 };
static Pattern gAnimatedAntsPatternBottom =
			{ 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0 };
static Pattern gAnimatedAntsPatternLeft =
			{ 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00 };
static Pattern gAnimatedAntsPatternRight =
			{ 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00 };
static Pattern gInactiveAntsPattern =
			{ 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55 };

//=====================================================================================
// AntsReference NewAnts(void)
//=====================================================================================
// Allocates memory for a new marching ants record.  The newly-created ants are by
// default active but not visible, and have no selection.  Following this call, you
// will need to set the ants' port (SetAntsPort()) and the limit rectangle
// (SetAntsLimitRect()) before doing anything really useful.  This function returns nil
// if memory is incredibly tight.
//=====================================================================================

extern AntsReference NewAnts(void)
{
	AntsReference theAnts = (AntsReference)NewHandleClear(sizeof(AntsRecord));
	if (theAnts) (*theAnts)->active = true;			// active by default
	return theAnts;
}

//=====================================================================================
// void DisposeAnts(AntsReference theAnts)
//=====================================================================================
// Disposes of all memory allocated for drawing the ants specified by theAnts.  After
// this function is called, the AntsReference is no longer valid.
//=====================================================================================

extern void DisposeAnts(AntsReference theAnts)
{
	if ((*theAnts)->verticalWorld) DisposeGWorld((*theAnts)->verticalWorld);
	if ((*theAnts)->horizontalWorld) DisposeGWorld((*theAnts)->horizontalWorld);
	DisposeHandle((Handle)theAnts);
}

//=====================================================================================
// Boolean AntsHaveSelection(AntsReference theAnts)
//=====================================================================================
// Returns true if the specified ants have a non-nil selection.  Use this function to
// determine if there is, in fact, some selected area you should be operating on.
//=====================================================================================

extern Boolean AntsHaveSelection(AntsReference theAnts)
{
	return (((*theAnts)->bounds.right - (*theAnts)->bounds.left) > antsMinimumX &&
				((*theAnts)->bounds.bottom - (*theAnts)->bounds.top) > antsMinimumY);
}

//=====================================================================================
// void GetAntsSelection(AntsReference theAnts, Rect *theBounds)
//=====================================================================================
// Returns, in theBounds, the current selection specified by theAnts.  If there is no
// selection, this function will return the rectangle { 0, 0, 0, 0 }, so be careful if
// you're doing something like calling MapRect() on it.
//=====================================================================================

extern void GetAntsSelection(AntsReference theAnts, Rect *theBounds)
{
	*theBounds = (*theAnts)->bounds;
}

//=====================================================================================
// void SetAntsSelection(AntsReference theAnts, Rect *newBounds)
//=====================================================================================
// Changes the current selection specified by theAnts to the rectangle described by
// newBounds.  If the ants are currently visible, they are hidden and then reshown
// after the operation is complete.  Attempting to set the selection to anything
// smaller than 8 pixels in any direction will create a nil selection, causing any
// visible selection to disappear.  An easier way of doing this latter operation is to
// simply call ResetAntsSelection(), described below.
//=====================================================================================

extern void SetAntsSelection(AntsReference theAnts, Rect *newBounds)
{
	Boolean wasDrawn = (*theAnts)->drawn;

	if (wasDrawn) HideAnts(theAnts);
	if (newBounds->left != (*theAnts)->bounds.left ||
				newBounds->right != (*theAnts)->bounds.right ||
				newBounds->top != (*theAnts)->bounds.top ||
				newBounds->bottom != (*theAnts)->bounds.bottom) {
		if ((*theAnts)->horizontalWorld)
			DisposeGWorld((*theAnts)->horizontalWorld), (*theAnts)->horizontalWorld = nil;
		if ((*theAnts)->verticalWorld)
			DisposeGWorld((*theAnts)->verticalWorld), (*theAnts)->verticalWorld = nil;
	}
	if ((newBounds->right - newBounds->left) > antsMinimumX &&
				(newBounds->bottom - newBounds->top) > antsMinimumY) {
		(*theAnts)->bounds = *newBounds;
		SaveToGWorlds(theAnts);
	} else (*theAnts)->bounds.top = (*theAnts)->bounds.bottom =
				(*theAnts)->bounds.left = (*theAnts)->bounds.right = 0;
	if (wasDrawn) ShowAnts(theAnts);
}

//=====================================================================================
// void ResetAntsSelection(AntsReference theAnts)
//=====================================================================================
// Clears the current selection specified by theAnts to nil.  If the ants are currently
// visible, the visible selection will disappear.
//=====================================================================================

extern void ResetAntsSelection(AntsReference theAnts)
{
	static Rect gNullRect = { 0, 0, 0, 0 };
	
	HideAnts(theAnts);
	SetAntsSelection(theAnts, &gNullRect);
}

//=====================================================================================
// void GetAntsLimitRect(AntsReference theAnts, Rect *limitRect)
//=====================================================================================
// Returns in limitRect the current limit rectangle specified by theAnts.  See also
// the function SetAntsLimitRect() for a full description of what the limit rectangle
// does.
//=====================================================================================

extern void GetAntsLimitRect(AntsReference theAnts, Rect *limitRect)
{
	*limitRect = (*theAnts)->limitRect;
}

//=====================================================================================
// void SetAntsLimitRect(AntsReference theAnts, Rect *newLimit)
//=====================================================================================
// Sets the limit rectangle for theAnts to the rectangle newLimit.  The limit rectangle
// is used in two places, and it is important that you update this value whenever your
// window changes size.  The first and most common place the limit rectangle is used is
// when drawing the ants; in this case, the limit rectangle is used as the clipping
// rectangle for drawing.  The limit rectangle is also used when tracking the mouse
// during a selection, to specify the outermost boundaries of the area the user may
// select.
//=====================================================================================

extern void SetAntsLimitRect(AntsReference theAnts, Rect *newLimit)
{
	(*theAnts)->limitRect = *newLimit;
}

//=====================================================================================
// GrafPtr GetAntsPort(AntsReference theAnts)
//=====================================================================================
// Returns the port the ants are currently drawn into.  Use the SetAntsPort() function,
// below, to change the associated GrafPort.
//=====================================================================================

extern GrafPtr GetAntsPort(AntsReference theAnts)
{
	return (*theAnts)->port;
}

//=====================================================================================
// void SetAntsPort(AntsReference theAnts, GrafPtr thePort)
//=====================================================================================
// Use this function to set the ants' destination port.  The destination port
// determines where the ants will be drawn and tracked.
//=====================================================================================

extern void SetAntsPort(AntsReference theAnts, GrafPtr thePort)
{
	(*theAnts)->port = thePort;
}

//=====================================================================================
// Boolean AreAntsVisible(AntsReference theAnts)
//=====================================================================================
// Returns true if the ants are currently visible.  Note that this merely reflects the
// state of the internal visible flag, which can be set by the HideAnts() and
// ShowAnts() functions.
//=====================================================================================

extern Boolean AreAntsVisible(AntsReference theAnts)
{
	return (*theAnts)->visible;
}

//=====================================================================================
// void TrackAntsSelection(AntsReference theAnts, Point localStart,
//			AntsScrollProc scroll, long refCon)
//=====================================================================================
// Call this function to track the mouse as a new selection is made.  The point
// localStart is the location of mouse down event, in local coordinates.  scroll is a
// pointer to a callback function which gets called whenever the mouse pointer moves
// outside of the ants' limit rectangle.  It gets passed two pointers to shorts,
// indicating how much we want to scroll by; your function should scroll as much as
// you can and store the amount actually scrolled back into the two shorts.  You can
// also pass nil for scroll if you don't want to do autoscrolling.  The refCon
// parameter gets passed along to your callback function, for storing a pointer to
// information which might be needed.  Once this function returns, you can use the
// AntsHaveSelection() function to determine if a selection was, in fact, made.
//=====================================================================================

extern void TrackAntsSelection(AntsReference theAnts, Point localStart,
			AntsScrollProc scroll, long refCon)
{
	static Pattern gTrackSelectionPattern =
				{ 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55 };
	Rect newBounds, limitRect = (*theAnts)->limitRect;
	Point lastPoint, newPoint;
	RgnHandle saveClip;
	PenState oldState;
	GrafPtr oldPort;
	short dx, dy;
	long pinned;

	GetPort(&oldPort);
	SetPort((*theAnts)->port);
	if (saveClip = NewRgn()) {
		GetClip(saveClip);
		ClipRect(&limitRect);
		GetPenState(&oldState);
		PenNormal();
		PenMode(srcXor);
		PenPat(&gTrackSelectionPattern);
		lastPoint = localStart;
		newBounds.left = newBounds.right = localStart.h;
		newBounds.top = newBounds.bottom = localStart.v;
		FrameRect(&newBounds);
		while (StillDown()) {
			GetMouse(&newPoint);
			if (newPoint.h == lastPoint.h && newPoint.v == lastPoint.v &&
						PtInRect(newPoint, &limitRect)) continue;
			dx = dy = 0;
			if (newPoint.h > limitRect.right) dx = (newPoint.h - limitRect.right > 8) ? -16 : -1;
			else if (newPoint.h < limitRect.left) dx = (limitRect.left - newPoint.h > 8) ? 16 : 1;
			if (newPoint.v > limitRect.bottom) dy = (newPoint.v - limitRect.bottom > 8) ? -16 : -1;
			else if (newPoint.v < limitRect.top) dy = (limitRect.top - newPoint.v > 8) ? 16 : 1;
			FrameRect(&newBounds);
			if (scroll && (dx || dy)) {
				scroll(&dx, &dy, refCon);
				ClipRect(&limitRect);
				PenNormal();
				PenMode(srcXor);
				PenPat(&gTrackSelectionPattern);
				localStart.h += dx;
				localStart.v += dy;
				OffsetRect(&newBounds, dx, dy);
			}
			pinned = PinRect(&limitRect, newPoint);
			newPoint = *(Point *)&pinned;
			if (newPoint.h > localStart.h) {
				newBounds.left = localStart.h;
				newBounds.right = newPoint.h + 1;
			} else {
				newBounds.left = newPoint.h;
				newBounds.right = localStart.h + 1;
			}
			if (newPoint.v > localStart.v) {
				newBounds.top = localStart.v;
				newBounds.bottom = newPoint.v + 1;
			} else {
				newBounds.top = newPoint.v;
				newBounds.bottom = localStart.v + 1;
			}
			FrameRect(&newBounds);
			lastPoint = newPoint;
		}
		FrameRect(&newBounds);
		SetAntsSelection(theAnts, &newBounds);
		SetPenState(&oldState);
		SetClip(saveClip);
		DisposeRgn(saveClip);
	}
	SetPort(oldPort);
}

//=====================================================================================
// void ShowAnts(AntsReference theAnts)
//=====================================================================================
// This function forces the ants to become visible if they weren't already.  At this
// time we also save what is currently behind the selection rectangle in GWorlds so
// that we can do a clean restore.  Note that this is the *only* time we save the area
// behind the ants.  However, this is usually ok, since we need to call HideAnts()
// before doing any drawing anyhow, and then ShowAnts() again after the drawing is
// complete.
//=====================================================================================

extern void ShowAnts(AntsReference theAnts)
{
	(*theAnts)->visible = true;
	if (!(*theAnts)->drawn) {
		SaveToGWorlds(theAnts);
		DrawAnts(theAnts);
	}
}

//=====================================================================================
// void HideAnts(AntsReference theAnts)
//=====================================================================================
// This function hides the ants and restores any background behind them.  This function
// should get called before any drawing occurs, so as not to overwrite the current
// ants.
//=====================================================================================

extern void HideAnts(AntsReference theAnts)
{
	if ((*theAnts)->drawn) EraseAnts(theAnts);
	(*theAnts)->visible = false;
}

//=====================================================================================
// void AnimateAnts(AntsReference theAnts)
//=====================================================================================
// This function rotates the ants forward one notch and redraws them.  Call this
// function in your null event handler, whenever the time is appropriate.
//=====================================================================================

extern void AnimateAnts(AntsReference theAnts)
{
	if (!(*theAnts)->verticalWorld || !(*theAnts)->horizontalWorld) EraseAnts(theAnts);
	MarchAnts(theAnts);
	DrawAnts(theAnts);
}

//=====================================================================================
// void ActivateAnts(AntsReference theAnts)
//=====================================================================================
// Call this function to activate the specified ants selection (i.e., when the window
// the ants are in receives an activate event).  This means that the ants will be
// displayed as an animated dashed rectangle that rotates whenever AnimateAnts() is
// called.
//=====================================================================================

extern void ActivateAnts(AntsReference theAnts)
{
	if (!(*theAnts)->verticalWorld || !(*theAnts)->horizontalWorld) EraseAnts(theAnts);
	(*theAnts)->active = true;
	DrawAnts(theAnts);
}

//=====================================================================================
// void DeactivateAnts(AntsReference theAnts)
//=====================================================================================
// Call this function to deactivate the specified ants selection (i.e., when the window
// the ants are in receives an deactivate event).  This means that the ants will be
// displayed as a steady dotted rectangle which does nothing when AnimateAnts() is
// called.
//=====================================================================================

extern void DeactivateAnts(AntsReference theAnts)
{
	if (!(*theAnts)->verticalWorld || !(*theAnts)->horizontalWorld) EraseAnts(theAnts);
	(*theAnts)->active = false;
	DrawAnts(theAnts);
}

/**************************************************************************************
***************************************************************************************
**********                                                                   **********
**********                                                                   **********
**********          E N D   O F   P U B L I C   I N T E R F A C E S          **********
**********                                                                   **********
**********                                                                   **********
***************************************************************************************
**************************************************************************************/

//=====================================================================================
// OSErr DrawAnts(AntsReference theAnts)
//=====================================================================================
// INTERNAL FUNCTION.  Draws the ants if they are supposed to be visible.  If there was
// enough memory for the offscreen GWorlds, we draw them using srcCopy mode to make
// them nice and visible.  If there was not enough memory for the GWorlds, we use
// srcXor mode instead so that things still work, if not as prettily.
//=====================================================================================

static OSErr DrawAnts(AntsReference theAnts)
{
	OSErr theErr = noErr;
	RgnHandle saveClip;
	PenState oldState;
	GrafPtr oldPort;
	Rect theRect;

	if (!(*theAnts)->visible) return noErr;
	theRect = (*theAnts)->bounds;
	if ((theRect.bottom - theRect.top) == 0 || (theRect.right - theRect.left) == 0)
		return noErr;
	GetPort(&oldPort);
	SetPort((*theAnts)->port);
	if (saveClip = NewRgn()) {
		GetClip(saveClip);
		theRect = (*theAnts)->limitRect;
		ClipRect(&theRect);
		GetPenState(&oldState);
		PenNormal();
		if (!(*theAnts)->horizontalWorld || !(*theAnts)->verticalWorld) PenMode(srcXor);
		else PenMode(srcCopy);
		MoveTo((*theAnts)->bounds.left, (*theAnts)->bounds.top);
		if ((*theAnts)->active) PenPat(&gAnimatedAntsPatternTop);
		else PenPat(&gInactiveAntsPattern);
		LineTo((*theAnts)->bounds.right - 1, (*theAnts)->bounds.top);
		if ((*theAnts)->active) PenPat(&gAnimatedAntsPatternRight);
		LineTo((*theAnts)->bounds.right - 1, (*theAnts)->bounds.bottom - 1);
		if ((*theAnts)->active) PenPat(&gAnimatedAntsPatternBottom);
		LineTo((*theAnts)->bounds.left, (*theAnts)->bounds.bottom - 1);
		if ((*theAnts)->active) PenPat(&gAnimatedAntsPatternLeft);
		LineTo((*theAnts)->bounds.left, (*theAnts)->bounds.top);
		SetPenState(&oldState);
		(*theAnts)->drawn = true;
		SetClip(saveClip);
		DisposeRgn(saveClip);
	} else theErr = memFullErr;
	SetPort(oldPort);
	return theErr;
}

//=====================================================================================
// OSErr EraseAnts(AntsReference theAnts)
//=====================================================================================
// INTERNAL FUNCTION.  Erases the ants if they are supposed to be visible.  If there
// was enough memory for the offscreen GWorlds, we copy their contents back into the
// window.  If there was not enough memory for the GWorlds, we just redraw the ants
// using srcXor mode which erases them.
//=====================================================================================

static OSErr EraseAnts(AntsReference theAnts)
{
	OSErr theErr = noErr;
	RgnHandle saveClip;
	PenState oldState;
	GrafPtr oldPort;
	Rect theRect;

	theRect = (*theAnts)->bounds;
	if ((theRect.bottom - theRect.top) == 0 || (theRect.right - theRect.left) == 0)
		return noErr;
	GetPort(&oldPort);
	SetPort((*theAnts)->port);
	if (saveClip = NewRgn()) {
		GetClip(saveClip);
		theRect = (*theAnts)->limitRect;
		ClipRect(&theRect);
		GetPenState(&oldState);
		PenNormal();
		if (!(*theAnts)->horizontalWorld || !(*theAnts)->verticalWorld) {
			PenMode(srcXor);
			MoveTo((*theAnts)->bounds.left, (*theAnts)->bounds.top);
			if ((*theAnts)->active) PenPat(&gAnimatedAntsPatternTop);
			else PenPat(&gInactiveAntsPattern);
			LineTo((*theAnts)->bounds.right - 1, (*theAnts)->bounds.top);
			if ((*theAnts)->active) PenPat(&gAnimatedAntsPatternRight);
			LineTo((*theAnts)->bounds.right - 1, (*theAnts)->bounds.bottom - 1);
			if ((*theAnts)->active) PenPat(&gAnimatedAntsPatternBottom);
			LineTo((*theAnts)->bounds.left, (*theAnts)->bounds.bottom - 1);
			if ((*theAnts)->active) PenPat(&gAnimatedAntsPatternLeft);
			LineTo((*theAnts)->bounds.left, (*theAnts)->bounds.top);
			SetPenState(&oldState);
		} else RestoreFromGWorlds(theAnts);
		(*theAnts)->drawn = false;
		SetClip(saveClip);
		DisposeRgn(saveClip);
	} else theErr = memFullErr;
	SetPort(oldPort);
	return theErr;
}

//=====================================================================================
// OSErr MarchAnts(AntsReference theAnts)
//=====================================================================================
// INTERNAL FUNCTION.  Rotates the QuickDraw patterns used to draw the ants.
//=====================================================================================

static OSErr MarchAnts(AntsReference theAnts)
{
#if applec
#pragma unused(theAnts)
#endif
	char temp;
	int i;
	
#ifdef dangerousPattern
	temp = ((gAnimatedAntsPatternTop[0] & 1) ? 0x80 : 0) + (gAnimatedAntsPatternTop[0] >> 1);
	for (i = 0; i < 8; i++) gAnimatedAntsPatternTop[i] = temp;
	temp = ((gAnimatedAntsPatternBottom[0] & 0x80) ? 1 : 0) + (gAnimatedAntsPatternBottom[0] << 1);
	for (i = 0; i < 8; i++) gAnimatedAntsPatternBottom[i] = temp;
	temp = gAnimatedAntsPatternRight[7];
	for (i = 7; i > 0; i--) gAnimatedAntsPatternRight[i] = gAnimatedAntsPatternRight[i - 1];
	gAnimatedAntsPatternRight[0] = temp;
	temp = gAnimatedAntsPatternLeft[0];
	for (i = 0; i < 7; i++) gAnimatedAntsPatternLeft[i] = gAnimatedAntsPatternLeft[i + 1];
	gAnimatedAntsPatternLeft[7] = temp;
#else
	temp = ((gAnimatedAntsPatternTop.pat[0] & 1) ? 0x80 : 0) + (gAnimatedAntsPatternTop.pat[0] >> 1);
	for (i = 0; i < 8; i++) gAnimatedAntsPatternTop.pat[i] = temp;
	temp = ((gAnimatedAntsPatternBottom.pat[0] & 0x80) ? 1 : 0) + (gAnimatedAntsPatternBottom.pat[0] << 1);
	for (i = 0; i < 8; i++) gAnimatedAntsPatternBottom.pat[i] = temp;
	temp = gAnimatedAntsPatternRight.pat[7];
	for (i = 7; i > 0; i--) gAnimatedAntsPatternRight.pat[i] = gAnimatedAntsPatternRight.pat[i - 1];
	gAnimatedAntsPatternRight.pat[0] = temp;
	temp = gAnimatedAntsPatternLeft.pat[0];
	for (i = 0; i < 7; i++) gAnimatedAntsPatternLeft.pat[i] = gAnimatedAntsPatternLeft.pat[i + 1];
	gAnimatedAntsPatternLeft.pat[7] = temp;
#endif
	return noErr;
}

//=====================================================================================
// void SaveToGWorlds(AntsReference theAnts)
//=====================================================================================
// INTERNAL FUNCTION.  Attempts to create offscreen GWorlds for saving the area
// behind the ants.  If successful, we go ahead and do CopyBits from the screen(s) to
// the GWorlds; otherwise, we leave the GWorldPtrs set to nil to indicate that we
// should draw using srcXor instead.
//=====================================================================================

static void SaveToGWorlds(AntsReference theAnts)
{
	Rect srcRect, dstRect;
	GWorldPtr theGWorld;
	OSErr theErr;
	Rect theRect;
	
	if (!(*theAnts)->horizontalWorld) {
		theRect = (*theAnts)->bounds;
		theRect.bottom = theRect.top + 2;
		OffsetRect(&theRect, -theRect.left, -theRect.top);
		theErr = NewGWorld(&theGWorld, 32, &theRect, nil, nil, 0);
		if (theErr == noErr && theGWorld) (*theAnts)->horizontalWorld = theGWorld;
		else return;
	}
	if (!(*theAnts)->verticalWorld) {
		theRect = (*theAnts)->bounds;
		theRect.right = theRect.left + 2;
		OffsetRect(&theRect, -theRect.left, -theRect.top);
		theErr = NewGWorld(&theGWorld, 32, &theRect, nil, nil, 0);
		if (theErr == noErr && theGWorld) (*theAnts)->verticalWorld = theGWorld;
		else return;
	}
	srcRect = (*theAnts)->bounds;
	srcRect.bottom = srcRect.top + 1;
	dstRect = srcRect;
	OffsetRect(&dstRect, -dstRect.left, -dstRect.top);
	CopyFromCurrentPort((*theAnts)->horizontalWorld, &srcRect, &dstRect);
	srcRect.bottom = (*theAnts)->bounds.bottom;
	srcRect.top = srcRect.bottom - 1;
	dstRect.top++;
	dstRect.bottom++;
	CopyFromCurrentPort((*theAnts)->horizontalWorld, &srcRect, &dstRect);
	srcRect = (*theAnts)->bounds;
	srcRect.right = srcRect.left + 1;
	dstRect = srcRect;
	OffsetRect(&dstRect, -dstRect.left, -dstRect.top);
	CopyFromCurrentPort((*theAnts)->verticalWorld, &srcRect, &dstRect);
	srcRect.right = (*theAnts)->bounds.right;
	srcRect.left = srcRect.right - 1;
	dstRect.left++;
	dstRect.right++;
	CopyFromCurrentPort((*theAnts)->verticalWorld, &srcRect, &dstRect);
}

//=====================================================================================
// void RestoreFromGWorlds(AntsReference theAnts)
//=====================================================================================
// INTERNAL FUNCTION.  Restores the original pixels from the GWorlds saved in the
// specified ants record.
//=====================================================================================

static void RestoreFromGWorlds(AntsReference theAnts)
{
	static RGBColor gBlack = { 0x0000, 0x0000, 0x0000 }, gWhite = { 0xffff, 0xffff, 0xffff };
	Rect srcRect, dstRect;
	
	RGBForeColor(&gBlack);
	RGBBackColor(&gWhite);
	dstRect = (*theAnts)->bounds;
	dstRect.bottom = dstRect.top + 1;
	srcRect = dstRect;
	OffsetRect(&srcRect, -srcRect.left, -srcRect.top);
	CopyToCurrentPort((*theAnts)->horizontalWorld, &srcRect, &dstRect);
	dstRect.bottom = (*theAnts)->bounds.bottom;
	dstRect.top = dstRect.bottom - 1;
	srcRect.top++;
	srcRect.bottom++;
	CopyToCurrentPort((*theAnts)->horizontalWorld, &srcRect, &dstRect);
	dstRect = (*theAnts)->bounds;
	dstRect.right = dstRect.left + 1;
	srcRect = dstRect;
	OffsetRect(&srcRect, -srcRect.left, -srcRect.top);
	CopyToCurrentPort((*theAnts)->verticalWorld, &srcRect, &dstRect);
	dstRect.right = (*theAnts)->bounds.right;
	dstRect.left = dstRect.right - 1;
	srcRect.left++;
	srcRect.right++;
	CopyToCurrentPort((*theAnts)->verticalWorld, &srcRect, &dstRect);
}

//=====================================================================================
// void CopyToCurrentPort(GWorldPtr srcGWorld, Rect *srcRect, Rect *dstRect)
//=====================================================================================
// INTERNAL FUNCTION.  Performs a CopyBits operation from the specified offscreen
// GWorld to the current onscreen port, mapping from srcRect to dstRect.  This makes
// our life easier by taking care of foreground/background colors, destination ports,
// and locked offscreen PixMaps.
//=====================================================================================

static void CopyToCurrentPort(GWorldPtr srcGWorld, Rect *srcRect, Rect *dstRect)
{
	static RGBColor gBlack = { 0x0000, 0x0000, 0x0000 }, gWhite = { 0xffff, 0xffff, 0xffff };
	PixMapHandle srcPixMap, dstPixMap;
	char pixelsState;
	GrafPtr thePort;
	
	RGBForeColor(&gBlack);
	RGBBackColor(&gWhite);
	GetPort(&thePort);
	srcPixMap = GetGWorldPixMap(srcGWorld);
	dstPixMap = GetGWorldPixMap((CGrafPtr)thePort);
	pixelsState = GetPixelsState(srcPixMap);
	LockPixels(srcPixMap);
	CopyBits((BitMap *)*srcPixMap, (BitMap *)*dstPixMap, srcRect, dstRect, srcCopy, nil);
	SetPixelsState(srcPixMap, pixelsState);
}

//=====================================================================================
// void CopyFromCurrentPort(GWorldPtr srcGWorld, Rect *srcRect, Rect *dstRect)
//=====================================================================================
// INTERNAL FUNCTION.  Performs a CopyBits operation from the current onscreen port
// to the specified offscreen GWorld, mapping from srcRect to dstRect.  This makes our
// life easier by taking care of foreground/background colors, destination ports, and
// locked offscreen PixMaps.
//=====================================================================================

static void CopyFromCurrentPort(GWorldPtr dstGWorld, Rect *srcRect, Rect *dstRect)
{
	PixMapHandle srcPixMap, dstPixMap;
	RgnHandle maskRgn;
	char pixelsState;
	GrafPtr thePort;
	
	if (maskRgn = NewRgn()) {
		GetPort(&thePort);
		SetGWorld(dstGWorld, nil);
		RectRgn(maskRgn, srcRect);
		SectRgn(maskRgn, thePort->visRgn, maskRgn);
		OffsetRgn(maskRgn, dstRect->left - srcRect->left, dstRect->top - srcRect->top);
		srcPixMap = GetGWorldPixMap((CGrafPtr)thePort);
		dstPixMap = GetGWorldPixMap(dstGWorld);
		pixelsState = GetPixelsState(dstPixMap);
		LockPixels(dstPixMap);
		CopyBits((BitMap *)*srcPixMap, (BitMap *)*dstPixMap, srcRect, dstRect, srcCopy, maskRgn);
		SetPixelsState(dstPixMap, pixelsState);
		SetGWorld((CGrafPtr)thePort, nil);
		DisposeRgn(maskRgn);
	}
}
