
//============================================================================
//----------------------------------------------------------------------------
//									Utilities.c
//----------------------------------------------------------------------------
//============================================================================

// These functions are sort of universal utility functions.  They aren't specific�
// to Glypha per se.  I use these (and others) in many, many games.  Many of them�
// as well are useful for any app you might write for the Mac.

#include "Externs.h"


#define kActive						0
#define kInactive					255


GDHandle	thisGDevice;
long		tickNext;


//==============================================================  Functions
//--------------------------------------------------------------  RandomInt

// Takes a short (range) and returns a random number from zero to range - 1.

short RandomInt (short range)
{
	register long	rawResult;
	
	rawResult = Random();
	if (rawResult < 0L)
		rawResult *= -1L;
	rawResult = (rawResult * (long)range) / 32768L;
	
	return ((short)rawResult);
}

//--------------------------------------------------------------  RedAlert

// Generic error function.  This is called when there is no hope of recovering
// from the error.  A simple alert is brought up and the text passed in (theStr)
// is displayed.  When the user clicks the Okay button, we quit to the Finder.

void RedAlert (StringPtr theStr)
{
	#define		kRedAlertID		128
	short		whoCares;
	
	ParamText(theStr, "\p", "\p", "\p");		// Replace ^0 in alert with error mssg.
	whoCares = Alert(kRedAlertID, 0L);			// Bring up alert.
	ExitToShell();								// Quit to Finder.
}

//--------------------------------------------------------------  FindOurDevice

// Get a handle to the MainDevice (monitor with the Menubar).

void FindOurDevice (void)
{
	thisGDevice = GetMainDevice();
	if (thisGDevice == 0L)						// If a nil handle is returned...
		RedAlert("\pCouldn't Find Our Device");	// call our universal error alert.
}

//--------------------------------------------------------------  LoadGraphic

// Handy function that loads a PICT graphic, get's its bounds and draws it.
// The port drawn to is assumed the current port.  No scaling is done.

void LoadGraphic (short resID)
{
	Rect		bounds;
	PicHandle	thePicture;
	
	thePicture = GetPicture(resID);				// Load graphic from resource fork.
	if (thePicture == 0L)						// Check to see if nil (did it load?)
		RedAlert("\pA Graphic Couldn't Be Loaded");
	
	HLock((Handle)thePicture);					// If we made it this far, lock handle.
	bounds = (*thePicture)->picFrame;			// Get a copy of the picture's bounds.
	HUnlock((Handle)thePicture);				// We can unlock the picture now.
	OffsetRect(&bounds, -bounds.left, -bounds.top);	// Offset bounds rect to (0, 0).
	DrawPicture(thePicture, &bounds);			// Draw picture to current port.
	
	ReleaseResource((Handle)thePicture);		// Dispose of picture from heap.
}

//--------------------------------------------------------------  CreateOffScreenPixMap

// Handles the creation of an offscreen pixmap.  Depth is assumed to be that of the�
// current gDevice.  If the allocation fails (low memory, etc.) we quit to Finder.

void CreateOffScreenPixMap (Rect *theRect, CGrafPtr *offScreen)
{
	CTabHandle	thisColorTable;
	GDHandle	oldDevice;
	CGrafPtr	newCGrafPtr;
	Ptr			theseBits;
	long		sizeOfOff, offRowBytes;
	OSErr		theErr;
	short		thisDepth;
	
	oldDevice = GetGDevice();
	SetGDevice(thisGDevice);
	newCGrafPtr = 0L;
	newCGrafPtr = (CGrafPtr)NewPtrClear(sizeof(CGrafPort));
	if (newCGrafPtr != 0L)
	{
		OpenCPort(newCGrafPtr);
		thisDepth = (**(*newCGrafPtr).portPixMap).pixelSize;
		offRowBytes = ((((long)thisDepth * 
				(long)(theRect->right - theRect->left)) + 15L) >> 4L) << 1L;
		sizeOfOff = (long)(theRect->bottom - theRect->top) * offRowBytes;
		OffsetRect(theRect, -theRect->left, -theRect->top);
		theseBits = NewPtr(sizeOfOff);
		if (theseBits != 0L)
		{
			(**(*newCGrafPtr).portPixMap).baseAddr = theseBits;
			(**(*newCGrafPtr).portPixMap).rowBytes = (short)offRowBytes + 0x8000;
			(**(*newCGrafPtr).portPixMap).bounds = *theRect;
			thisColorTable = (**(**thisGDevice).gdPMap).pmTable;
			theErr = HandToHand((Handle *)&thisColorTable);
			(**(*newCGrafPtr).portPixMap).pmTable = thisColorTable;
			ClipRect(theRect);
			RectRgn(newCGrafPtr->visRgn, theRect);
			ForeColor(blackColor);
			BackColor(whiteColor);
			EraseRect(theRect);
		}
		else
		{
			CloseCPort(newCGrafPtr);		
			DisposePtr((Ptr)newCGrafPtr);
			newCGrafPtr = 0L;
			RedAlert("\pCouldn't Allocate Enough Memory");
		}
	}
	else
		RedAlert("\pCouldn't Allocate Enough Memory");
	
	*offScreen = newCGrafPtr;
	SetGDevice(oldDevice);
}

//--------------------------------------------------------------  CreateOffScreenBitMap

// Creates an offscreen bitmap.  Depth is of course 1 (b & w).  If this function�
// fails to create the bitmap, we post an alert and quit to the Finder.

void CreateOffScreenBitMap (Rect *theRect, GrafPtr *offScreen)
{
	GrafPtr		theBWPort;
	BitMap		theBitMap;	
	long		theRowBytes;
	
	theBWPort = (GrafPtr)(NewPtr(sizeof(GrafPort)));
	OpenPort(theBWPort);
	theRowBytes = (long)((theRect->right - theRect->left + 15L) / 16L) * 2L;
	theBitMap.rowBytes = (short)theRowBytes;
	theBitMap.baseAddr = NewPtr((long)theBitMap.rowBytes * 
		(theRect->bottom - theRect->top));
	if (theBitMap.baseAddr == 0L)
		RedAlert("\pCouldn't Create Bitmaps");
	theBitMap.bounds = *theRect;
	if (MemError() != noErr)
		RedAlert("\pCouldn't Create Bitmaps");
	SetPortBits(&theBitMap);
	ClipRect(theRect);
	RectRgn(theBWPort->visRgn, theRect);
	EraseRect(theRect);
	*offScreen = theBWPort;
}

//--------------------------------------------------------------  ZeroRectCorner

// Offset rect to (0, 0).  This means the upper left corner of the rect is 
// moved to the origin - to (0, 0) - to the upperleft corner of the port.

void ZeroRectCorner (Rect *theRect)
{
	theRect->right -= theRect->left;	// Move right edge by amount of left.
	theRect->bottom -= theRect->top;	// Move bottom edge by amount of top.
	theRect->left = 0;					// Can now set left to zero.
	theRect->top = 0;					// Can set top edge to zero as well.
}

//--------------------------------------------------------------  FlashShort

// This is a simple debugging function that will display the short passed to it�
// in the upper left corner of the screen.  It's a handy way to watch the value�
// of a variable while the program is running.

void FlashShort (short theValue)
{
	GrafPtr			wasPort, tempPort;
	Str255			tempStr;
	Rect			tempRect;
	
	GetPort(&wasPort);						// Remember old grafPort.
	
	tempPort = (GrafPtr)NewPtrClear(sizeof(GrafPort));
	OpenPort(tempPort);						// Create a new empty port.
	
	NumToString((long)theValue, tempStr);	// Convert value passed in to a string.
	MoveTo(20, 40);							// Move the pen to the upperleft corner.
	SetRect(&tempRect, 18, 20, 122, 42);	// Create a rect up there as well.
	EraseRect(&tempRect);					// Erase the rect (to make a white hole).
	DrawString(tempStr);					// And draw our text into that hole.
	
	ClosePort(tempPort);					// Get rid of out temp port.
	SetPort((GrafPtr)wasPort);				// And set port back to the old one.
}

//--------------------------------------------------------------  LogNextTick

// Simple function to set a global (tickNext) to the current TickCount() plus�
// some offset.  We'll then wait for TickCount() to exceed that global.  We use�
// this function and the function below to regulate animation speeds (remember�
// your game may be run on a slow Mac or a fast one - we need a way to keep the�
// motion consistent.  I love when the comments are longer than the function.
// (Not really.)

void LogNextTick (long howMany)
{
	tickNext = TickCount() + howMany;		// Get machine's TickCount() and add to it.
}

//--------------------------------------------------------------  WaitForNextTick

// This is the companion function to the above function (LogNextTick()).
// We do nothing but loop until TickCount() catches up with (or passes) our�
// global variable tickNext.

void WaitForNextTick (void)
{
	do
	{
	}
	while (TickCount() < tickNext);			// Loop until TickCount() catches up.
}

//--------------------------------------------------------------  TrapExists  

// A nice "test function" that test for the existence of some ToolBox trap.
// Returns TRUE if the function exists, FALSE if it doesn't.

Boolean TrapExists (short trapNumber)
{
	#define		kUnimpTrap		0x9F
	
				// Test trap number against unimplemented trap number.
	return ((NGetTrapAddress(trapNumber, ToolTrap) !=
			NGetTrapAddress(kUnimpTrap, ToolTrap)));
}

//--------------------------------------------------------------  DoWeHaveGestalt  

// This function specifically tests for the availablity of the Gestalt() function.
// It returns TRUE if Gestalt() exists, FALSE if it doesn't.

Boolean DoWeHaveGestalt (void)
{
	#define		kGestaltTrap	0xAD
	
				// Call above function (TrapExists()) with the Gestalt() trap number.
	return (TrapExists(kGestaltTrap));
}

//--------------------------------------------------------------  CenterAlert

// Handy function to center any alert within the main monitor.

void CenterAlert (short alertID)
{
	AlertTHndl	alertHandle;
	Rect		theScreen, alertRect;
	short		horiOff, vertOff;
	Byte		wasState;
	
	theScreen = qd.screenBits.bounds;		// Get main monitor's bounds.
	theScreen.top += LMGetMBarHeight();		// Account for menubar height.
											// Get handle to alert resource.
	alertHandle = (AlertTHndl)GetResource('ALRT', alertID);
	if (alertHandle != 0L)					// Make sure we got it!
	{										// Remember its "state" (locked, etc.)
		wasState = HGetState((Handle)alertHandle);
		HLock((Handle)alertHandle);			// We'll lock it.
											// Get a copy of it's bounds and zero.
		alertRect = (**alertHandle).boundsRect;
		OffsetRect(&alertRect, -alertRect.left, -alertRect.top);
											// Calculate offsets for centering bounds.
		horiOff = ((theScreen.right - theScreen.left) - alertRect.right) / 2;	
		vertOff = ((theScreen.bottom - theScreen.top) - alertRect.bottom) / 3;
											// And offset the bounds copy.
		OffsetRect(&alertRect, horiOff, vertOff + LMGetMBarHeight());
											// Set alerts bounds to our centered rect.
		(**alertHandle).boundsRect = alertRect;
		HSetState((Handle)alertHandle, wasState);
	}
}

//--------------------------------------------------------------  RectWide

// Handy function for returning the absolute width of a rectangle.

short RectWide (Rect *theRect)
{
	return (theRect->right - theRect->left);
}

//--------------------------------------------------------------  RectTall

// Handy function for returning the absolute height of a rectangle.

short RectTall (Rect *theRect)
{
	return (theRect->bottom - theRect->top);
}

//--------------------------------------------------------------  CenterRectInRect

// Nice utility function that takes two rectangles and centers the first�
// rectangle within the second.

void CenterRectInRect (Rect *rectA, Rect *rectB)
{
	short	widthA, tallA;
	
	widthA = RectWide(rectA);				// Get width of 1st rect.
	tallA = RectTall(rectA);				// Get height of 1st rect.
											// Do the math (center horizontally).
	rectA->left = rectB->left + (RectWide(rectB) - widthA) / 2;
	rectA->right = rectA->left + widthA;
											// Do the math (center vertically).
	rectA->top = rectB->top + (RectTall(rectB) - tallA) / 2;
	rectA->bottom = rectA->top + tallA;
}

//--------------------------------------------------------------  PasStringCopy

// This is a nice function that helps to free you from dealing with C strings.
// It takes one Pascal-style string and copies it to a second.

void PasStringCopy (StringPtr p1, StringPtr p2)
{
	register short		stringLength;
	
	stringLength = *p2++ = *p1++;	// Get 1st string's length.
	while (--stringLength >= 0)		// Loop through each character in 1st string.
		*p2++ = *p1++;				// And copy to 2nd string.
}

//--------------------------------------------------------------  CenterDialog

// Like CenterAlert(), this function centers a Dialog on the main monitor.

void CenterDialog (short dialogID)
{
	DialogTHndl	dlogHandle;
	Rect		theScreen, dlogBounds;
	short		hPos, vPos;
	Byte		wasState;
	
	theScreen = qd.screenBits.bounds;			// Get main monitor's bounds.
	theScreen.top += LMGetMBarHeight();			// Add menuBar's height.
												// Load up dialog from resource.
	dlogHandle = (DialogTHndl)GetResource('DLOG', dialogID);
	if (dlogHandle != 0L)						// If it loaded....!
	{											// Remember handle state.
		wasState = HGetState((Handle)dlogHandle);
		HLock((Handle)dlogHandle);				// We're going to lock it.
												// Get a copy of the dialog's bounds.
		dlogBounds = (**dlogHandle).boundsRect;
		OffsetRect(&dlogBounds, -dlogBounds.left, -dlogBounds.top);
												// Calculate how much to offset.
		hPos = ((theScreen.right - theScreen.left) - dlogBounds.right) / 2;
		vPos = ((theScreen.bottom - theScreen.top) - dlogBounds.bottom) / 3;
												// Offset ourt copy of the bounds.
		OffsetRect(&dlogBounds, hPos, vPos + LMGetMBarHeight());
												// Set dlg's bounds to centered rect.
		(**dlogHandle).boundsRect = dlogBounds;
		HSetState((Handle)dlogHandle, wasState);// Restore handle's state.
	}
}

//--------------------------------------------------------------  DrawDefaultButton

// A nice dialog function.  This draws the bold default outline around�
// item #1 in the dialog passed in.

void DrawDefaultButton (DialogPtr theDialog)
{
	Rect		itemRect;
	Handle		itemHandle;
	short		itemType;
										// Get at the item's bounds.
	GetDItem(theDialog, 1, &itemType, &itemHandle, &itemRect);
	InsetRect(&itemRect, -4, -4);		// Inset (outset?) bounds by -4 pixels.
	PenSize(3, 3);						// Set the pen 3 pixels thick.
	FrameRoundRect(&itemRect, 16, 16);	// Draw the button outline.
	PenNormal();						// And restore pen to 1 pixel thick.
}

//--------------------------------------------------------------  PasStringCopyNum

// Another function to keep you from using C strings.  This one copies only a�
// certain number of characters from one Pascal-style string to a second.

void PasStringCopyNum (StringPtr p1, StringPtr p2, short charsToCopy)
{
	short		i;
	
	if (charsToCopy > *p1)		// If trying to copy more chars than there are�
		charsToCopy = *p1;		// Reduce the number of chars to copy to this size
	
	*p2 = charsToCopy;			// Set 2nd string's length to charsToCopy.
	
	*p2++;						// Point to first character in 2nd string.
	*p1++;						// Point to first character in 1st string.
	
	for (i = 0; i < charsToCopy; i++)
		*p2++ = *p1++;			// Copy the specified number of chars over.
}

//--------------------------------------------------------------  GetDialogString

// Handy dialog function that returns a dialog item string.  This will be�
// especially handy for getting the high score name the player enters.

void GetDialogString (DialogPtr theDialog, short item, StringPtr theString)
{
	Rect		itemRect;
	Handle		itemHandle;
	short		itemType;
										// Get handle to dialog item.
	GetDItem(theDialog, item, &itemType, &itemHandle, &itemRect);
	GetIText(itemHandle, theString);	// Extract text from item handle.
}

//--------------------------------------------------------------  SetDialogString

// Like the above function, but this one sets a dialog items string to whatever�
// you pass in.  We'll use this to set a default high score name.

void SetDialogString (DialogPtr theDialog, short item, StringPtr theString)
{
	Rect		itemRect;
	Handle		itemHandle;
	short		itemType;
										// Get handle to dialog item.
	GetDItem(theDialog, item, &itemType, &itemHandle, &itemRect);
	SetIText(itemHandle, theString);	// Set the items text to theString.
}

//--------------------------------------------------------------  SetDialogNumToStr

// This one is like SetDialogString() above, but it takes a number (long)�
// instead of a string (the function will convert the long to a string for us).

void SetDialogNumToStr (DialogPtr theDialog, short item, long theNumber)
{
	Str255		theString;
	Rect		itemRect;
	Handle		itemHandle;
	short		itemType;
	
	NumToString(theNumber, theString);	// Convert long to a string.
	GetDItem(theDialog, item, &itemType, &itemHandle, &itemRect);
	SetIText(itemHandle, theString);	// Set the item's text to this number/string.
}

//--------------------------------------------------------------  GetDialogNumFromStr

// This one is like GetDialogString() above, but returns a long (number)�
// instead of a string (it does this by converting the string to a long).

void GetDialogNumFromStr (DialogPtr theDialog, short item, long *theNumber)
{
	Str255		theString;
	Rect		itemRect;
	Handle		itemHandle;
	short		itemType;
										// Get a handle to the dialog item.
	GetDItem(theDialog, item, &itemType, &itemHandle, &itemRect);
	GetIText(itemHandle, theString);	// Get the item's text.
	StringToNum(theString, theNumber);	// Convert the text to a long.
}

//--------------------------------------------------------------  DisableControl

// Another dialog utility for "graying out" buttons or other controls in a dialog.

void DisableControl (DialogPtr theDialog, short whichItem)
{
	Rect		iRect;
	Handle		iHandle;
	short		iType;
										// Get a handle to the dialog item.
	GetDItem(theDialog, whichItem, &iType, &iHandle, &iRect);
										// Set it's "hilite state" to "grayed out".
	HiliteControl((ControlHandle)iHandle, kInactive);
}

