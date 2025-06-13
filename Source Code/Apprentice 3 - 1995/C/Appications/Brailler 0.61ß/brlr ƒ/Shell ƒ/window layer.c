/*
	These routines are derived from WindowExtensions.c from develop #15, written by Dean Yu,
		copyright ©1993 Apple Computer, Inc.
*/

#include "window layer.h"
#include "graphics dispatch.h"

extern	Boolean		gHasColorQD;		/* see environment.c */

#define kMagicNumber		0x16435934

static void ActivateWindow(WindowRef theWindow);
static void DeactivateWindow(WindowRef theWindow);
static WindowRef GetWindowList(void);
static void SetWindowList(WindowRef windowReference);

pascal WindowRef MyNewWindow(void *wStorage, const Rect *boundsRect, ConstStr255Param title,
	Boolean visible, WindowAttributes attributes, WindowRef behind, long refCon, Boolean isFloat)
{
	WindowAttributes	titleBarType;
	WindowRef			newWindowReference;
	WindowRef			lastFloater;
	short				procID;
	Boolean				hasGoAway = false;
	
	titleBarType = attributes & kWindowTitlebarMask;
	if (titleBarType != 0)
	{
		switch (titleBarType)
		{
			case kHasRoundedTitlebarMask:
			case kHasDocumentTitlebarMask:
				if ((behind == (WindowRef) -1) &&
					((lastFloater = GetLastFloatingWindow()) != nil))
				{
					behind = lastFloater;
				}
											
				if (titleBarType == kHasRoundedTitlebarMask)
					procID=kRoundedWindowProc;
				else
					procID=kStandardDocumentWindowProc;
				break;
			case kHasPaletteTitlebarMask:
				if (((behind == nil) && (GetFrontDocumentWindow() != nil)) ||
					((behind != (WindowRef) -1) && (GetWindowKind(behind) != kApplicationFloaterKind)))
				{
					return 0L;
				}
				procID = kFloatingWindowProc;
				break;
			default:
				return 0L;
		}
		
		if (attributes & kHasModalBorderMask)
			procID += movableDBoxProc;
		else
		{
			if (attributes & kHasCloseBoxMask)
				hasGoAway = true;
			if (attributes & kHasZoomBoxMask)
				procID += zoomDocProc;
			if (!(attributes & kHasGrowBoxMask))
				procID += noGrowDocProc;
		}
	}
	else
	{
		/* dialog of some kind; only allow them to be frontmost */
		if (behind != (WindowRef) -1)
		{
			return 0L;
		}
		
		procID = dBoxProc;
		if (!(attributes & kHasModalBorderMask))
		{
			procID = plainDBox;
			if (attributes & kHasThickDropShadowMask)
				procID = altDBoxProc;
		}
	}
	
	/* wStorage needs to be allocated _before_ calling MyNewWindow */
	newWindowReference = (WindowRef)wStorage;
	
	if (!gHasColorQD)
		NewWindow(newWindowReference, boundsRect, title, false, procID, (WindowPtr)behind,
			hasGoAway, refCon);
	else
		NewCWindow(newWindowReference, boundsRect, title, false, procID, (WindowPtr)behind,
			hasGoAway, refCon);
	
	SetWindowMagic(newWindowReference, kMagicNumber);
	
	if (isFloat)
		SetWindowKind(newWindowReference, kApplicationFloaterKind);
	
	if (visible)
		MyShowWindow(newWindowReference);
	
	return newWindowReference;
}

pascal void MyDisposeWindow(WindowRef windowReference)
{
	if (GetWindowVisible(windowReference))
		MyHideWindow(windowReference);
	CloseWindow((WindowPtr) windowReference);
	DisposePtr((Ptr) windowReference);
}

pascal Boolean MySelectWindow(WindowRef windowToSelect)
/* returns TRUE if window is selected, FALSE if some error or if window is already frontmost */
{
	WindowRef			currentFrontWindow;
	WindowRef			lastFloatingWindow;
	Boolean				isFloatingWindow;

	if (GetWindowKind(windowToSelect) == kApplicationFloaterKind)
	{
		isFloatingWindow = true;
		currentFrontWindow = (WindowRef) FrontWindow();
	}
	else
	{
		isFloatingWindow = false;
		currentFrontWindow = GetFrontDocumentWindow();
		lastFloatingWindow = GetLastFloatingWindow();
	}

	if (currentFrontWindow != windowToSelect)
	{
		if (isFloatingWindow)
			BringToFront((WindowPtr) windowToSelect);
		else
		{
			if (lastFloatingWindow == nil)
				SelectWindow((WindowPtr) windowToSelect);
			else
			{
				DeactivateWindow(currentFrontWindow);
				SendBehind((WindowPtr) windowToSelect, (WindowPtr) lastFloatingWindow);
				ActivateWindow(windowToSelect);
			}
		}
		return TRUE;
	}
	
	return FALSE;
}

pascal void MyShowWindow(WindowRef windowToShow)
{
	WindowRef			windowBehind;
	WindowRef			frontNonFloatingWindow;
	short				windowClass;
	Boolean				windowIsInFront = false;
	
	if (GetWindowVisible(windowToShow) != false)
		return;
		
	windowClass = GetWindowKind(windowToShow);
	
	if (windowClass != kApplicationFloaterKind)
	{
		windowBehind = (WindowRef)GetNextWindow(windowToShow);
		if (windowBehind == GetFrontDocumentWindow())
		{
			if (windowBehind != nil)
				DeactivateWindow(windowBehind);
			SetWindowHilite(windowToShow, true);
			windowIsInFront = true;
		}
	}
	else
	{
		ValidateWindowList();
		frontNonFloatingWindow = GetFrontDocumentWindow();
		if ((frontNonFloatingWindow != nil) &&
			(frontNonFloatingWindow == (WindowRef) FrontWindow()) &&
			(WindowIsModal(frontNonFloatingWindow)))
		{
			SetWindowHilite(windowToShow, false);
		}
		else
		{
			SetWindowHilite(windowToShow, true);
			windowIsInFront = true;
		}
	}
	
	ShowHide((WindowPtr) windowToShow, true);
	
	if (windowIsInFront)
	{
		ActivateWindow(windowToShow);
	}
}

pascal void MyHideWindow(WindowRef windowToHide)
{
	WindowRef			frontFloater;
	WindowRef			frontNonFloater;
	WindowRef			lastFloater;
	WindowRef			windowBehind;
	
	if (GetWindowVisible(windowToHide) == false)
		return;
	
	frontFloater = (WindowRef) FrontWindow();
	if (GetWindowKind(frontFloater) != kApplicationFloaterKind)
		frontFloater = nil;
	frontNonFloater = FrontNonFloatingWindow();
	
	ShowHide((WindowPtr) windowToHide, false);
	
	if (windowToHide == frontFloater)
	{
		windowBehind = GetNextWindow(windowToHide);
		
		if ((windowBehind != nil) &&
			(GetWindowKind(windowBehind) == kApplicationFloaterKind))
		{
			SetNextWindow(windowToHide, GetNextWindow(windowBehind));
			SetNextWindow(windowBehind, windowToHide);
			SetWindowList(windowBehind);
		}
	}
	else
	{
		if (windowToHide == frontNonFloater)
		{
			windowBehind = GetNextWindow(windowToHide);
			
			if (windowBehind != nil)
			{
				SetNextWindow(windowToHide, GetNextWindow(windowBehind));
				SetNextWindow(windowBehind, windowToHide);
				lastFloater = LastFloatingWindow();
				if (lastFloater != nil)
				{
					SetNextWindow(lastFloater, windowBehind);
				}
				else
				{
					SetWindowList(windowBehind);
				}
				
				ActivateWindow(windowBehind);
			}
		}
	}
}

pascal void MyDragWindow(WindowRef windowToDrag, Point startPoint, const Rect *draggingBounds)
{
	Rect		dragRect;
	KeyMap		keyMap;
	GrafPtr		savePort;
	GrafPtr		windowManagerPort;
	RgnHandle	dragRegion;
	RgnHandle	windowContentRegion;
	long		dragResult;
	short		topLimit;
	short		newHorizontalWindowPosition;
	short		newVerticalWindowPosition;
	short		horizontalOffset;
	short		verticalOffset;
	Boolean		commandKeyDown = false;
	
	if (WaitMouseUp())
	{
		topLimit = GetMBarHeight() + 4;
		dragRect = *draggingBounds;
		if (dragRect.top < topLimit)
			dragRect.top = topLimit;
	
		GetPort(&savePort);
		GetWMgrPort(&windowManagerPort);
		SetPort(windowManagerPort);
		SetClip(GetGrayRgn());
		
		GetKeys(keyMap);
		if (keyMap[1] & 0x8000)
			commandKeyDown = true;
	
		if ((commandKeyDown == true) ||
			(GetWindowKind(windowToDrag) != kApplicationFloaterKind))
		{
			if (commandKeyDown == false)
				ClipAbove(FrontNonFloatingWindow());
			else
				ClipAbove(windowToDrag);
		}
			
		dragRegion = NewRgn();
		CopyRgn(GetStructureRegion(windowToDrag), dragRegion);
		
		dragResult = DragGrayRgn(dragRegion, startPoint, &dragRect, &dragRect, noConstraint, nil);
	
		SetPort(savePort);

		if (dragResult != 0)
		{
			horizontalOffset = dragResult & 0xFFFF;
			verticalOffset = dragResult >> 16;
			if (verticalOffset != -32768)
			{
				windowContentRegion = GetContentRegion(windowToDrag);
				newHorizontalWindowPosition = (**windowContentRegion).rgnBBox.left + horizontalOffset;
				newVerticalWindowPosition = (**windowContentRegion).rgnBBox.top + verticalOffset;
				
				MoveWindow((WindowPtr) windowToDrag, newHorizontalWindowPosition, newVerticalWindowPosition, false);
				
			}
		}
	
		if (commandKeyDown == false)
			MySelectWindow(windowToDrag);
	
		DisposeRgn(dragRegion);
	}
}

pascal WindowRef GetFrontDocumentWindow(void)
{
	WindowRef	theWindow;
	
	theWindow = (WindowRef) FrontWindow();
	
	while ((theWindow != nil) && (GetWindowKind(theWindow) == kApplicationFloaterKind))
	{
		do
		{
			theWindow = GetNextWindow(theWindow);
		} while ((theWindow != nil) && (GetWindowVisible(theWindow) == false));
	}

	return theWindow;
}

pascal WindowRef GetLastFloatingWindow(void)
{
	WindowRef	theWindow;
	WindowRef	lastFloatingWindow;
	
	theWindow = GetWindowList();
	lastFloatingWindow = nil;
	
	while (theWindow != nil)
	{
		if (GetWindowKind(theWindow) == kApplicationFloaterKind)
			lastFloatingWindow = theWindow;
		theWindow = GetNextWindow(theWindow);
	}
	
	return lastFloatingWindow;
}

pascal Boolean WindowIsModalQQ(WindowRef windowReference)
{
	short	windowVariant;
	
	windowVariant = GetWVariant((WindowPtr) windowReference);
	if ((GetWindowKind(windowReference) == dialogKind) &&
		((windowVariant == dBoxProc) ||
		(windowVariant == movableDBoxProc)))
		return true;
	else
		return false;
}

pascal void DeactivateFloatersAndFirstDocumentWindow(void)
{
	WindowRef			firstWindow;
	WindowRef			secondDocumentWindow;
	WindowRef			currentWindow;
	
	ValidateWindowList();
	
	firstWindow = (WindowRef) FrontWindow();
	secondDocumentWindow = FrontNonFloatingWindow();
	if (secondDocumentWindow != nil)
		secondDocumentWindow = GetNextWindow(secondDocumentWindow);
		
	currentWindow = firstWindow;
	while (currentWindow != secondDocumentWindow)
	{
		if (GetWindowVisible(currentWindow))
			DeactivateWindow(currentWindow);
		currentWindow = GetNextWindow(currentWindow);
	}
}

pascal void ActivateFloatersAndFirstDocumentWindow(void)
{
	ProcessSerialNumber	currentPSN;
	ProcessSerialNumber	frontPSN;
	WindowRef			firstWindow;
	WindowRef			secondDocumentWindow;
	WindowRef			currentWindow;
	OSErr				getFrontProcessResult;
	OSErr				getCurrentProcessResult;
	OSErr				sameProcessResult;
	Boolean				isSameProcess;
	
	getFrontProcessResult = GetFrontProcess(&frontPSN);
	getCurrentProcessResult = GetCurrentProcess(&currentPSN);
	
	if ((getFrontProcessResult == noErr) && (getCurrentProcessResult == noErr))
		sameProcessResult = SameProcess(&frontPSN, &currentPSN, &isSameProcess);
		
	if ((sameProcessResult == noErr) && (isSameProcess == false))
		SuspendFloatingWindows();
	else
	{
		firstWindow = (WindowRef) FrontWindow();
		secondDocumentWindow = FrontNonFloatingWindow();
		if (secondDocumentWindow != nil)
			secondDocumentWindow = GetNextWindow(secondDocumentWindow);
		
		currentWindow = firstWindow;
		while (currentWindow != secondDocumentWindow)
		{
			if (GetWindowVisible(currentWindow))
				ActivateWindow(currentWindow);
			currentWindow = GetNextWindow(currentWindow);
		}
	}
}

pascal void HideOneFloatingWindow(WindowRef floatingWindow)
{
	if (GetWindowKind(floatingWindow)!=kApplicationFloaterKind)
		return;
	
	SetWasVisible(floatingWindow, FALSE);
	ShowHide((WindowPtr)floatingWindow, FALSE);
	DeactivateWindow(floatingWindow);
}

pascal void ShowOneFloatingWindow(WindowRef floatingWindow)
{
	if (GetWindowKind(floatingWindow)!=kApplicationFloaterKind)
		return;
	
	SetWasVisible(floatingWindow, TRUE);
	ShowHide((WindowPtr)floatingWindow, TRUE);
	ActivateWindow(floatingWindow);
}

pascal void SuspendFloatingWindows(void)
{
	WindowRef	currentWindow;
	Boolean		windowIsVisible;
	
	currentWindow = (WindowRef) GetWindowList();
	if (GetWindowKind(currentWindow) == kApplicationFloaterKind)
	{
		do
		{
			windowIsVisible = GetWindowVisible(currentWindow);
			SetWasVisible(currentWindow, windowIsVisible);
			if (windowIsVisible)
				ShowHide((WindowPtr) currentWindow, false);
			currentWindow = GetNextWindow(currentWindow);
		} while ((currentWindow != nil) &&
				 (GetWindowKind(currentWindow) == kApplicationFloaterKind));
	}
	
	currentWindow = FrontNonFloatingWindow();
	if (currentWindow != nil)
		DeactivateWindow(currentWindow);
}

pascal void ResumeFloatingWindows(void)
{
	WindowRef	currentWindow;
	Boolean		windowWasVisible;
	
	currentWindow = GetWindowList();
	if (GetWindowKind(currentWindow) == kApplicationFloaterKind)
	{
		do
		{
			windowWasVisible = GetWasVisible(currentWindow);
			if (windowWasVisible)
			{
				ShowHide((WindowPtr) currentWindow, true);
				ActivateWindow(currentWindow);
			}
			currentWindow = GetNextWindow(currentWindow);
		} while ((currentWindow != nil) &&
				 (GetWindowKind(currentWindow) == kApplicationFloaterKind));
	}
	
	currentWindow = FrontNonFloatingWindow();
	if (currentWindow != nil)
		ActivateWindow(currentWindow);
}

static	void ActivateWindow(WindowRef theWindow)
{
	HiliteWindow((WindowPtr)theWindow, TRUE);
	ActivateWindowDispatch(GetWindowIndex(theWindow));
}

static	void DeactivateWindow(WindowRef theWindow)
{
	HiliteWindow((WindowPtr)theWindow, FALSE);
	DeactivateWindowDispatch(GetWindowIndex(theWindow));
}

pascal void ValidateWindowList(void)
{
	WindowRef	currentWindow = GetWindowList();
	WindowRef	lastFloatingWindow = LastFloatingWindow();
	WindowRef	firstFloatingWindow = nil;
	WindowRef	documentWindowsToMove = nil;
	WindowRef	lastDocumentWindowAdded = nil;
	WindowRef	previousWindow = nil;
	
	if (currentWindow)
	{
		do
		{
			if (GetWindowKind(currentWindow) == kApplicationFloaterKind)
			{
				firstFloatingWindow = currentWindow;
				break;
			}
			else
			{
				SetNextWindow(previousWindow, GetNextWindow(currentWindow));

				if (documentWindowsToMove == nil)
					documentWindowsToMove = currentWindow;
				else
					SetNextWindow(lastDocumentWindowAdded, currentWindow);
				lastDocumentWindowAdded = currentWindow;
				
				previousWindow = currentWindow;
				currentWindow = GetNextWindow(currentWindow);
			}
		} while (currentWindow);
		
		if (documentWindowsToMove && firstFloatingWindow)
		{
			SetNextWindow(lastDocumentWindowAdded, GetNextWindow(lastFloatingWindow));
			SetNextWindow(lastFloatingWindow, documentWindowsToMove);
			
			if (documentWindowsToMove == GetWindowList())
				SetWindowList(firstFloatingWindow);
		}
	}
}

pascal void GetWindowPortRect(WindowRef windowReference, Rect *portRect)
{
	*portRect = ((WindowPtr) windowReference)->portRect;
}

static	WindowRef GetWindowList(void)
{
	return LMGetWindowList();
}

static	void SetWindowList(WindowRef windowReference)
{
	LMSetWindowList(windowReference);
}

pascal Boolean WindowHasLayer(WindowRef theWindow)
{
	if (theWindow==0L)
		return FALSE;
	return GetWindowMagic(theWindow)==kMagicNumber;
}

pascal Boolean WindowIsFloat(WindowRef theWindow)
{
	if (theWindow==0L)
		return FALSE;
	return WindowIsFloatQQ(theWindow);
}

pascal WindowRef GetIndWindowPtr(short index)
{
	WindowRef		currentWindow;
	
	currentWindow = (WindowRef) GetWindowList();
		
	while (currentWindow!=0L)
	{
		if (GetWindowIndex(currentWindow)==index)
			return currentWindow;
		currentWindow = GetNextWindow(currentWindow);
	}
	
	return 0L;
}
