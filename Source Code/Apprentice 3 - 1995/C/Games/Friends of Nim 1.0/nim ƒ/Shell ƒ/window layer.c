#include "window layer.h"
#include "program globals.h"
#if USE_WINDOWS_MENU
#include "windows menu.h"
#endif
#include "graphics dispatch.h"
#include "environment.h"
#include "offscreen layer.h"
#include "memory layer.h"
#include "text layer.h"
#if USE_DRAG
#include "drag layer.h"
#endif
#include <QDOffscreen.h>

#define kMagicNumber		0x16435934

static	WindowRef GetLastDocumentWindow(void);
static	void ActivateWindow(WindowRef theWindow);
static	void DeactivateWindow(WindowRef theWindow);
static	WindowRef GetWindowList(void);
static	void SetWindowList(WindowRef windowReference);
static	void MyHiliteWindow(WindowRef theWindow, Boolean fHilite);
static	void GetMainScreenBounds(Rect *screenRect);
static	GDHandle GetBiggestDevice(WindowRef theWindow);
static	short GetBiggestDeviceDepth(WindowRef theWindow);
static	short GetWindowRealDepth(WindowRef theWindow);
static	void ArrangeWindowsRecurse(WindowRef theWindow, short *h, short *v);

WindowRef MyNewWindow(void *wStorage, const Rect *boundsRect, ConstStr255Param title,
	Boolean visible, WindowAttributes attributes, WindowRef behind, long refCon, Boolean isFloat,
	Boolean isDummy)
{
	WindowAttributes	titleBarType;
	WindowRef			newWindowReference;
	WindowRef			lastFloater;
	short				procID=0;
	Boolean				hasGoAway=FALSE;
	
	titleBarType=attributes & kWindowTitlebarMask;
	if (titleBarType!=0)
	{
		switch (titleBarType)
		{
			case kHasRoundedTitlebarMask:
			case kHasDocumentTitlebarMask:
				if ((behind==(WindowRef)-1) &&
					((lastFloater=GetLastFloatingWindow())!=0L))
				{
					behind=lastFloater;
				}
											
				if (titleBarType==kHasRoundedTitlebarMask)
					procID=kRoundedWindowProc;
				else
					procID=kStandardDocumentWindowProc;
				break;
			case kHasPaletteTitlebarMask:
				if (((behind==0L) && (GetFrontDocumentWindow()!=0L)) ||
					((behind!=(WindowRef)-1) && (GetWindowKind(behind)!=kApplicationFloaterKind)))
				{
					return 0L;
				}
				procID=kFloatingWindowProc;
				break;
			default:
				return 0L;
		}
		
		if (attributes & kHasModalBorderMask)
			procID +=movableDBoxProc;
		else
		{
			if (attributes & kHasCloseBoxMask)
				hasGoAway=TRUE;
			if (attributes & kHasZoomBoxMask)
				procID +=zoomDocProc;
			if (!(attributes & kHasGrowBoxMask))
				procID +=noGrowDocProc;
		}
	}
	else
	{
		procID=dBoxProc;
		if (!(attributes & kHasModalBorderMask))
		{
			procID=plainDBox;
			if (attributes & kHasThickDropShadowMask)
				procID=altDBoxProc;
		}
	}
	
	/* wStorage needs to be allocated _before_ calling MyNewWindow */
	newWindowReference=(WindowRef)wStorage;
	
	if (!gHasColorQD)
		NewWindow(newWindowReference, boundsRect, title, FALSE, procID, (WindowRef)behind,
			hasGoAway, refCon);
	else
		NewCWindow(newWindowReference, boundsRect, title, FALSE, procID, (WindowRef)behind,
			hasGoAway, refCon);
	
	SetWindowMagic(newWindowReference, kMagicNumber);
	
	if (isFloat)
		SetWindowKind(newWindowReference, kApplicationFloaterKind);
	if (isDummy)
		SetWindowKind(newWindowReference, kApplicationDummyKind);
	
	if (visible)
		MyShowWindow(newWindowReference);
	
	return newWindowReference;
}

void MyDisposeWindow(WindowRef windowReference)
{
	if (GetWindowVisible(windowReference))
		MyHideWindow(windowReference);
	CloseWindow((WindowRef) windowReference);
	DisposePtr((Ptr) windowReference);
}

Boolean MySelectWindow(WindowRef windowToSelect)
/* returns TRUE if window is selected, FALSE if some error or if window is already frontmost */
{
	WindowRef			currentFrontWindow;
	WindowRef			lastFloatingWindow;
	Boolean				isFloatingWindow;

	if (GetWindowKind(windowToSelect)==kApplicationFloaterKind)
	{
		isFloatingWindow=TRUE;
		currentFrontWindow=(WindowRef) FrontWindow();
	}
	else
	{
		isFloatingWindow=FALSE;
		currentFrontWindow=GetFrontDocumentWindow();
		lastFloatingWindow=GetLastFloatingWindow();
	}

	if (currentFrontWindow!=windowToSelect)
	{
		if (isFloatingWindow)
			BringToFront((WindowRef) windowToSelect);
		else
		{
			if (lastFloatingWindow==0L)
				SelectWindow((WindowRef) windowToSelect);
			else
			{
				DeactivateWindow(currentFrontWindow);
				SendBehind((WindowRef) windowToSelect, (WindowRef) lastFloatingWindow);
				ActivateWindow(windowToSelect);
			}
		}
		return TRUE;
	}
	
	return FALSE;
}

void MyShowWindow(WindowRef windowToShow)
{
	WindowRef			windowBehind;
	WindowRef			frontNonFloatingWindow;
	short				windowClass;
	Boolean				windowIsInFront=FALSE;
	
	if (GetWindowVisible(windowToShow)!=FALSE)
		return;
		
	windowClass=GetWindowKind(windowToShow);
	
	if (windowClass!=kApplicationFloaterKind)
	{
		windowBehind=(WindowRef)GetNextWindow(windowToShow);
		if (windowBehind==GetFrontDocumentWindow())
		{
			if (windowBehind!=0L)
				DeactivateWindow(windowBehind);
			SetWindowHilite(windowToShow, TRUE);
			windowIsInFront=TRUE;
		}
	}
	else
	{
		ValidateWindowList();
		frontNonFloatingWindow=GetFrontDocumentWindow();
		if ((frontNonFloatingWindow!=0L) &&
			(frontNonFloatingWindow==(WindowRef) FrontWindow()) &&
			(WindowIsModal(frontNonFloatingWindow)))
		{
			SetWindowHilite(windowToShow, FALSE);
		}
		else
		{
			SetWindowHilite(windowToShow, TRUE);
			windowIsInFront=TRUE;
		}
	}
	
	ShowHide((WindowRef) windowToShow, TRUE);
	
	if (windowIsInFront)
	{
		ActivateWindow(windowToShow);
	}
	
#if USE_DRAG
	ResetHiliteRgn(windowToShow);
#endif
}

void MyHideWindow(WindowRef windowToHide)
{
	WindowRef			frontFloater;
	WindowRef			frontNonFloater;
	WindowRef			lastFloater;
	WindowRef			windowBehind;
	
	if (GetWindowVisible(windowToHide)==FALSE)
		return;
	
	frontFloater=(WindowRef) FrontWindow();
	if (GetWindowKind(frontFloater)!=kApplicationFloaterKind)
		frontFloater=0L;
	frontNonFloater=FrontNonFloatingWindow();
	
	ShowHide((WindowRef) windowToHide, FALSE);
	
	if (windowToHide==frontFloater)
	{
		windowBehind=GetNextWindow(windowToHide);
		
		if ((windowBehind!=0L) &&
			(GetWindowKind(windowBehind)==kApplicationFloaterKind))
		{
			SetNextWindow(windowToHide, GetNextWindow(windowBehind));
			SetNextWindow(windowBehind, windowToHide);
			SetWindowList(windowBehind);
		}
	}
	else
	{
		if (windowToHide==frontNonFloater)
		{
			windowBehind=GetNextWindow(windowToHide);
			
			if (windowBehind!=0L)
			{
				SetNextWindow(windowToHide, GetNextWindow(windowBehind));
				SetNextWindow(windowBehind, windowToHide);
				lastFloater=LastFloatingWindow();
				if (lastFloater!=0L)
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

void MyDragWindow(WindowRef windowToDrag, Point startPoint, const Rect *draggingBounds)
{
	Rect			dragRect;
	KeyMap			keyMap;
	GrafPtr			savePort;
	GrafPtr			windowManagerPort;
	RgnHandle		dragRegion;
	RgnHandle		windowContentRegion;
	long			dragResult;
	short			topLimit;
	short			newHorizontalWindowPosition;
	short			newVerticalWindowPosition;
	short			horizontalOffset;
	short			verticalOffset;
	Boolean			commandKeyDown=FALSE;
	
	if (WaitMouseUp())
	{
		topLimit=GetMBarHeight() + 4;
		dragRect=*draggingBounds;
		if (dragRect.top < topLimit)
			dragRect.top=topLimit;
	
		GetPort(&savePort);
		GetWMgrPort(&windowManagerPort);
		SetPort(windowManagerPort);
		SetClip(GetGrayRgn());
		
		GetKeys(keyMap);
		if (keyMap[1] & 0x8000)
			commandKeyDown=TRUE;
	
		if ((commandKeyDown==TRUE) ||
			(GetWindowKind(windowToDrag)!=kApplicationFloaterKind))
		{
			if (commandKeyDown==FALSE)
				ClipAbove(FrontNonFloatingWindow());
			else
				ClipAbove(windowToDrag);
		}
			
		dragRegion=NewRgn();
		CopyRgn(GetStructureRegion(windowToDrag), dragRegion);
		
		dragResult=DragGrayRgn(dragRegion, startPoint, &dragRect, &dragRect, noConstraint, 0L);
	
		SetPort(savePort);

		if (dragResult!=0)
		{
			horizontalOffset=dragResult & 0xFFFF;
			verticalOffset=dragResult >> 16;
			if (verticalOffset!=-32768)
			{
				windowContentRegion=GetContentRegion(windowToDrag);
				newHorizontalWindowPosition=(**windowContentRegion).rgnBBox.left + horizontalOffset;
				newVerticalWindowPosition=(**windowContentRegion).rgnBBox.top + verticalOffset;
				MyMoveWindow(windowToDrag, newHorizontalWindowPosition, newVerticalWindowPosition, FALSE);
			}
		}
	
		if (commandKeyDown==FALSE)
			MySelectWindow(windowToDrag);
	
		DisposeRgn(dragRegion);
	}
}

void MyMoveWindow(WindowRef theWindow, short h, short v, Boolean selectNow)
{
	Point			theLocalPoint;
	
	MoveWindow(theWindow, h, v, FALSE);
	SetWindowBounds(theWindow,
		(*(((WindowPeek)theWindow)->contRgn))->rgnBBox);
	theLocalPoint.v=GetWindowBounds(theWindow).top;
	theLocalPoint.h=GetWindowBounds(theWindow).left;
	SetWindowTopLeft(theWindow, theLocalPoint);
	if (selectNow)
		MySelectWindow(theWindow);
#if USE_DRAG
	ResetHiliteRgn(theWindow);
#endif
}

void MyGrowWindow(WindowRef theWindow, Point theGlobalPoint)
{
	long			windSize;
	GrafPtr			oldPort;
	Rect			sizeRect;
	Point			theLocalPoint;
	
	if (GetGrowSizeDispatch(theWindow, &sizeRect)==kFailure)
		sizeRect=qd.screenBits.bounds;
	
	windSize=GrowWindow(theWindow, theGlobalPoint, &sizeRect);
	if (windSize!=0)
	{
		GetPort(&oldPort);
		SetPort(theWindow);
		SizeWindow(theWindow, LoWord(windSize), HiWord(windSize), TRUE);
		EraseRect(&(theWindow->portRect));
		InvalRect(&(theWindow->portRect));
		SetPort(oldPort);
		
		SetWindowWidth(theWindow, theWindow->portRect.right-theWindow->portRect.left);
		SetWindowHeight(theWindow, theWindow->portRect.bottom-theWindow->portRect.top);
		GrowWindowDispatch(theWindow);
		SetWindowBounds(theWindow,
			(*(((WindowPeek)theWindow)->contRgn))->rgnBBox);
		theLocalPoint.v=GetWindowBounds(theWindow).top;
		theLocalPoint.h=GetWindowBounds(theWindow).left;
		SetWindowTopLeft(theWindow, theLocalPoint);
#if USE_DRAG
		ResetHiliteRgn(theWindow);
#endif
	}
}

void MySizeWindow(WindowRef theWindow, short width, short height, Boolean updateNow)
{
	Point			theLocalPoint;
	
	SizeWindow(theWindow, width, height, updateNow);
	SetWindowWidth(theWindow, theWindow->portRect.right-theWindow->portRect.left);
	SetWindowHeight(theWindow, theWindow->portRect.bottom-theWindow->portRect.top);
	GrowWindowDispatch(theWindow);
	SetWindowBounds(theWindow,
		(*(((WindowPeek)theWindow)->contRgn))->rgnBBox);
	theLocalPoint.v=GetWindowBounds(theWindow).top;
	theLocalPoint.h=GetWindowBounds(theWindow).left;
	SetWindowTopLeft(theWindow, theLocalPoint);
#if USE_DRAG
	ResetHiliteRgn(theWindow);
#endif
}

void MyZoomWindow(WindowRef theWindow, short windowCode)
{
	GrafPtr			oldPort;
	Point			theLocalPoint;
	
	if (theWindow==0L)
		return;
	
	if (windowCode==-1)
		windowCode=(WindowIsZoomedQQ(theWindow)) ? inZoomIn : inZoomOut;
	
	GetPort(&oldPort);
	SetPort(theWindow);
	ZoomWindow(theWindow, windowCode, FALSE);
	EraseRect(&(theWindow->portRect));
	InvalRect(&(theWindow->portRect));
	SetPort(oldPort);

	SetWindowWidth(theWindow, theWindow->portRect.right-theWindow->portRect.left);
	SetWindowHeight(theWindow, theWindow->portRect.bottom-theWindow->portRect.top);
		
	ZoomWindowDispatch(theWindow);
	SetWindowBounds(theWindow,
		(*(((WindowPeek)theWindow)->contRgn))->rgnBBox);
	theLocalPoint.v=GetWindowBounds(theWindow).top;
	theLocalPoint.h=GetWindowBounds(theWindow).left;
	SetWindowTopLeft(theWindow, theLocalPoint);
	SetWindowIsZoomed(theWindow, (windowCode==inZoomOut));
#if USE_DRAG
	ResetHiliteRgn(theWindow);
#endif
}

Boolean MySendBehind(WindowRef window, WindowRef behind)
{
	if (window==0L)
		return FALSE;
	
	if (window==behind)
		return FALSE;
	
	if ((WindowHasLayer(window)) && ((WindowHasLayer(behind)) || (behind==0L)))
	{
		if (behind==0L)
		{
			if (GetWindowKind(window)==kApplicationFloaterKind)
				behind=GetLastFloatingWindow();
			else
				behind=GetLastDocumentWindow();
		}
		if (behind==window)
			return FALSE;
		
		if ((GetWindowKind(window)==kApplicationFloaterKind) &&
			(GetWindowKind(behind)!=kApplicationFloaterKind))
			return FALSE;
		
		if (GetWindowKind(behind)==kApplicationDummyKind)
			return FALSE;
	}
	
	SendBehind(window, behind);
	
#if USE_DRAG
	ResetHiliteRgn(window);
#endif
	
	return TRUE;
}

WindowRef GetFrontDocumentWindow(void)
{
	WindowRef	theWindow;
	
	theWindow=(WindowRef) FrontWindow();
	
	while ((theWindow!=0L) && (GetWindowKind(theWindow)==kApplicationFloaterKind))
	{
		do
		{
			theWindow=GetNextWindow(theWindow);
		} while ((theWindow!=0L) && (GetWindowVisible(theWindow)==FALSE));
	}

	return theWindow;
}

static	WindowRef GetLastDocumentWindow(void)
{
	WindowRef	theWindow, nextWindow;
	
	theWindow=(WindowRef)GetWindowList();
	
	while ((theWindow!=0L) && (GetWindowKind(theWindow)==kApplicationFloaterKind))
		theWindow=GetNextWindow(theWindow);
	
	while ((theWindow!=0L) && ((nextWindow=GetNextWindow(theWindow))!=0L) &&
		(GetWindowKind(theWindow)!=kApplicationFloaterKind) &&
		(GetWindowKind(nextWindow)!=kApplicationDummyKind))
	{
		theWindow=nextWindow;
	}
	
	return theWindow;
}

WindowRef GetLastFloatingWindow(void)
{
	WindowRef	theWindow;
	WindowRef	lastFloatingWindow;
	
	theWindow=GetWindowList();
	lastFloatingWindow=0L;
	
	while (theWindow!=0L)
	{
		if (GetWindowKind(theWindow)==kApplicationFloaterKind)
			lastFloatingWindow=theWindow;
		theWindow=GetNextWindow(theWindow);
	}
	
	return lastFloatingWindow;
}

Boolean WindowIsModalQQ(WindowRef windowReference)
{
	short	windowVariant;
	
	windowVariant=GetWVariant((WindowRef) windowReference);
	if ((GetWindowKind(windowReference)==dialogKind) &&
		((windowVariant==dBoxProc) ||
		(windowVariant==movableDBoxProc)))
		return TRUE;
	else
		return FALSE;
}

void DeactivateFloatersAndFirstDocumentWindow(void)
{
	WindowRef			firstWindow;
	WindowRef			secondDocumentWindow;
	WindowRef			currentWindow;
	
	ValidateWindowList();
	
	firstWindow=(WindowRef) FrontWindow();
	secondDocumentWindow=FrontNonFloatingWindow();
	if (secondDocumentWindow!=0L)
		secondDocumentWindow=GetNextWindow(secondDocumentWindow);
		
	currentWindow=firstWindow;
	while (currentWindow!=secondDocumentWindow)
	{
		if (GetWindowVisible(currentWindow))
			DeactivateWindow(currentWindow);
		currentWindow=GetNextWindow(currentWindow);
	}
}

void ActivateFloatersAndFirstDocumentWindow(void)
{
	WindowRef			firstWindow;
	WindowRef			secondDocumentWindow;
	WindowRef			currentWindow;
	
	if (gIsInBackground)
		SuspendFloatingWindows();
	else
	{
		firstWindow=(WindowRef) FrontWindow();
		secondDocumentWindow=FrontNonFloatingWindow();
		if (secondDocumentWindow!=0L)
			secondDocumentWindow=GetNextWindow(secondDocumentWindow);
		
		currentWindow=firstWindow;
		while (currentWindow!=secondDocumentWindow)
		{
			if (GetWindowVisible(currentWindow))
				ActivateWindow(currentWindow);
			currentWindow=GetNextWindow(currentWindow);
		}
	}
}

void HideOneFloatingWindow(WindowRef floatingWindow)
{
	if (GetWindowKind(floatingWindow)!=kApplicationFloaterKind)
		return;
	
	SetWasVisible(floatingWindow, FALSE);
	ShowHide((WindowRef)floatingWindow, FALSE);
	DeactivateWindow(floatingWindow);
}

void ShowOneFloatingWindow(WindowRef floatingWindow)
{
	if (GetWindowKind(floatingWindow)!=kApplicationFloaterKind)
		return;
	
	SetWasVisible(floatingWindow, TRUE);
	ShowHide((WindowRef)floatingWindow, TRUE);
	ActivateWindow(floatingWindow);
}

void SuspendFloatingWindows(void)
{
	WindowRef	currentWindow;
	Boolean		windowIsVisible;
	
	currentWindow=(WindowRef) GetWindowList();
	if (GetWindowKind(currentWindow)==kApplicationFloaterKind)
	{
		do
		{
			windowIsVisible=GetWindowVisible(currentWindow);
			SetWasVisible(currentWindow, windowIsVisible);
			if (windowIsVisible)
				ShowHide((WindowRef) currentWindow, FALSE);
			currentWindow=GetNextWindow(currentWindow);
		} while ((currentWindow!=0L) &&
				 (GetWindowKind(currentWindow)==kApplicationFloaterKind));
	}
	
	currentWindow=FrontNonFloatingWindow();
	if (currentWindow!=0L)
		DeactivateWindow(currentWindow);
}

void ResumeFloatingWindows(void)
{
	WindowRef	currentWindow;
	Boolean		windowWasVisible;
	
	currentWindow=GetWindowList();
	if (GetWindowKind(currentWindow)==kApplicationFloaterKind)
	{
		do
		{
			windowWasVisible=GetWasVisible(currentWindow);
			if (windowWasVisible)
			{
				ShowHide((WindowRef) currentWindow, TRUE);
				ActivateWindow(currentWindow);
			}
			currentWindow=GetNextWindow(currentWindow);
		} while ((currentWindow!=0L) &&
				 (GetWindowKind(currentWindow)==kApplicationFloaterKind));
	}
	
	currentWindow=FrontNonFloatingWindow();
	if (currentWindow!=0L)
		ActivateWindow(currentWindow);
}

static	void ActivateWindow(WindowRef theWindow)
{
	HiliteWindow(theWindow, TRUE);
	ActivateWindowDispatch(theWindow);
}

static	void DeactivateWindow(WindowRef theWindow)
{
	HiliteWindow(theWindow, FALSE);
	DeactivateWindowDispatch(theWindow);
}

void ValidateWindowList(void)
{
	WindowRef	currentWindow=GetWindowList();
	WindowRef	lastFloatingWindow=LastFloatingWindow();
	WindowRef	firstFloatingWindow=0L;
	WindowRef	documentWindowsToMove=0L;
	WindowRef	lastDocumentWindowAdded=0L;
	WindowRef	previousWindow=0L;
	
	if (currentWindow)
	{
		do
		{
			if (GetWindowKind(currentWindow)==kApplicationFloaterKind)
			{
				firstFloatingWindow=currentWindow;
				break;
			}
			else
			{
				SetNextWindow(previousWindow, GetNextWindow(currentWindow));

				if (documentWindowsToMove==0L)
					documentWindowsToMove=currentWindow;
				else
					SetNextWindow(lastDocumentWindowAdded, currentWindow);
				lastDocumentWindowAdded=currentWindow;
				
				previousWindow=currentWindow;
				currentWindow=GetNextWindow(currentWindow);
			}
		} while (currentWindow);
		
		if (documentWindowsToMove && firstFloatingWindow)
		{
			SetNextWindow(lastDocumentWindowAdded, GetNextWindow(lastFloatingWindow));
			SetNextWindow(lastFloatingWindow, documentWindowsToMove);
			
			if (documentWindowsToMove==GetWindowList())
				SetWindowList(firstFloatingWindow);
		}
	}
}

void GetWindowPortRect(WindowRef windowReference, Rect *portRect)
{
	*portRect=((WindowRef) windowReference)->portRect;
}

static	WindowRef GetWindowList(void)
{
	return LMGetWindowList();
}

static	void SetWindowList(WindowRef windowReference)
{
	LMSetWindowList(windowReference);
}

Boolean WindowHasLayer(WindowRef theWindow)
{
	if (theWindow==0L)
		return FALSE;
	return GetWindowMagic(theWindow)==kMagicNumber;
}

Boolean WindowIsFloat(WindowRef theWindow)
{
	if (theWindow==0L)
		return FALSE;
	return WindowIsFloatQQ(theWindow);
}

WindowRef GetIndWindowRef(short index)
{
	WindowRef		currentWindow;
	
	currentWindow=GetWindowList();
		
	while (currentWindow!=0L)
	{
		if ((WindowHasLayer(currentWindow)) && (GetWindowKind(currentWindow)!=kApplicationDummyKind) &&
			(GetWindowIndex(currentWindow)==index))
		{
			return currentWindow;
		}
		currentWindow=GetNextWindow(currentWindow);
	}
	
	return 0L;
}

OSErr OpenTheIndWindow(short index, Boolean newStatus)
{
	WindowRef		theWindow;
	Point			topLeft;
	ExtendedWindowRef	storage;
	Rect			screenRect;
	Rect			boundsRect;
	WindowAttributes	windowAttrs;
	
	theWindow=0L;
	if ((newStatus==kAlwaysOpenNew) || (!IndWindowExistsQQ(index)))
	{
		storage=(ExtendedWindowRef)NewPtr(sizeof(ExtendedWindowStruct));
		if (storage==0L)
			return MemError();
		
		theWindow=(WindowRef)storage;
		SetupWindowDispatch(index, theWindow);
		windowAttrs=GetWindowAttributes(theWindow);
		
		if (WindowAutoCenterQQ(theWindow))
		{
			GetMainScreenBounds(&screenRect);
			topLeft.h=screenRect.left+
				(((screenRect.right-screenRect.left)-GetWindowWidth(theWindow))/2);
			topLeft.v=screenRect.top+
				(((screenRect.bottom-screenRect.top)-GetWindowHeight(theWindow))/2);
			SetWindowTopLeft(theWindow, topLeft);

			if ((windowAttrs&kHasDocumentTitlebarMask) || (windowAttrs&kHasRoundedTitlebarMask) ||
				(windowAttrs&kHasModalBorderMask))
			{
				topLeft.v+=9;
			}
		}
		else
		{
			topLeft=GetWindowTopLeft(theWindow);
		}
		
		if (topLeft.v<GetMBarHeight()+1)
			topLeft.v=GetMBarHeight()+1;
		
		SetRect(&boundsRect, topLeft.h, topLeft.v, topLeft.h+GetWindowWidth(theWindow),
			topLeft.v+GetWindowHeight(theWindow));
		SetWindowBounds(theWindow, boundsRect);
		
		theWindow=MyNewWindow(storage, &(GetWindowBounds(theWindow)), GetWindowTitle(theWindow),
			FALSE, GetWindowAttributes(theWindow), (WindowRef)-1L, 0L, WindowIsFloatQQ(theWindow),
			FALSE);
		
#if USE_WINDOWS_MENU
		if (theWindow!=0L)
			AddToWindowsMenu(theWindow);
#endif
	}
	
	if (theWindow!=0L)
	{
		SetPort(theWindow);				/* important! for TE info to stick*/
		OpenWindowDispatch(theWindow);
		MyShowWindow(theWindow);
		MySelectWindow(theWindow);		/* immediately select this new window */
		InvalRect(&(theWindow->portRect));
	}
	else return -1;
	
	return noErr;
}

#define theGWorld		offscreenWorldPtr->offscreenPtr
#define thePixMapHandle	offscreenWorldPtr->pixMapHandle
#define bwBitMap		offscreenWorldPtr->bwBitMap
#define bwGrafPort		offscreenWorldPtr->bwGrafPort
#define bwGrafPtr		offscreenWorldPtr->offscreenPtr
#define currentGWorld	offscreenWorldPtr->currentGWorld
#define currentGDHandle	offscreenWorldPtr->currentGDHandle

OSErr UpdateTheWindow(WindowRef theWindow)
{
	Rect			boundsRect;
	TEHandle		hTE;
	short			realWindowDepth, maxWindowDepth;
	MyOffscreenPtr	offscreenWorldPtr;
	short			index;
	Boolean			depthChanged, goon;
	Boolean			wasColor, isColor;
	
	if (theWindow==0L) return memWZErr;
	index=GetWindowIndex(theWindow);
	realWindowDepth=GetWindowRealDepth(theWindow);
	maxWindowDepth=GetWindowMaxDepth(theWindow);
	wasColor=WindowIsColorQQ(theWindow);
	isColor=GetWindowReallyIsColor(theWindow);
	
	boundsRect=theWindow->portRect;
	OffsetRect(&boundsRect, -boundsRect.left, -boundsRect.top);
	
	if (WindowHasPermanentOffscreenWorldQQ(theWindow))
	{
		offscreenWorldPtr=GetWindowPermanentOffscreenWorld(theWindow);
		SetPortToOffscreenWindow(offscreenWorldPtr);
		ChangeDepthOffscreenWindow(theWindow, &offscreenWorldPtr, &depthChanged);
		SetWindowPermanentOffscreenWorld(theWindow, offscreenWorldPtr);
	}
	else
	{
		if ((offscreenWorldPtr=AllocateOffscreenWorld(theWindow, &depthChanged))==0L)
			return MemError();
	}
	
	if (wasColor!=isColor)
		depthChanged=TRUE;
	
	SetWindowDepth(theWindow, (realWindowDepth>=maxWindowDepth) ? maxWindowDepth : realWindowDepth);
	SetWindowIsColor(theWindow, isColor);
	
	if (depthChanged)
	{
		SetWindowOffscreenNeedsUpdate(theWindow, TRUE);
		goon=(ChangeDepthDispatch(theWindow)==kFailure);
	}
	else
	{
		goon=TRUE;
	}
	
	if (goon)
	{
		if (!WindowHasPermanentOffscreenWorldQQ(theWindow) || WindowOffscreenNeedsUpdateQQ(theWindow))
		{
			if (DrawWindowDispatch(theWindow, GetWindowDepth(theWindow))==kFailure)
			{
				EraseRect(&(theWindow->portRect));
			}
			SetWindowOffscreenNeedsUpdate(theWindow, FALSE);
		}
		
		if ((hTE=GetWindowTE(theWindow))!=0L)
		{
			(**hTE).inPort=gHasColorQD ? (GrafPtr)theGWorld : bwGrafPtr;
			EraseRect(&((**hTE).viewRect));
			TEUpdate(&(theWindow->portRect), hTE);
			(**hTE).inPort=theWindow;
		}
		
		if (gHasColorQD)
			SetGWorld(currentGWorld, currentGDHandle);
		
		SetPort(theWindow);
		
		if (CopybitsDispatch(theWindow, gHasColorQD ? (WindowRef)theGWorld : (WindowRef)bwGrafPtr)==kFailure)
		{
			CopyBits(gHasColorQD ?
						&(((WindowRef)theGWorld)->portBits) :
						&(((WindowRef)bwGrafPtr)->portBits),
					&(theWindow->portBits),
					&boundsRect,
					&boundsRect,
					0,
					0L);
			if ((GetWindowVScrollBar(theWindow)!=0L) || (GetWindowHScrollBar(theWindow)!=0L))
				UpdateControls(theWindow, theWindow->visRgn);
		}
		
		ValidRect(&(theWindow->portRect));
	}
	
	if (!WindowHasPermanentOffscreenWorldQQ(theWindow))
		DisposeOffscreenWorld(offscreenWorldPtr);
	
	CompactMem(maxSize);
	
	return noErr;
}

Boolean CloseTheWindow(WindowRef theWindow)
{
	short			index;
	
	index=GetWindowIndex(theWindow);
	
	if (CloseWindowDispatch(theWindow)==kCancel)
		return FALSE;
	
	DisposeWindowDispatch(theWindow);
	
#if USE_WINDOWS_MENU
	RemoveFromWindowsMenu(theWindow);
#endif
	MyDisposeWindow(theWindow);
	
	return TRUE;	/* successful close */
}

void GetMainScreenBounds(Rect *screenRect)
{
	*screenRect = qd.screenBits.bounds;		/* low-mem global */
	(*screenRect).top += GetMBarHeight();	/* don't include menu bar */
}

static	GDHandle GetBiggestDevice(WindowRef theWindow)
{
	Rect			tempRect;
	long			biggestSize;
	long			tempSize;
	GDHandle		thisHandle, gBiggestDevice;
	
	if (!gHasColorQD)
		return 0L;
	
	if (!theWindow)
		return GetMainDevice();
	
	thisHandle = GetDeviceList();
	gBiggestDevice = 0L;
	biggestSize = 0L;
	
	while (thisHandle)
	{
		if (TestDeviceAttribute(thisHandle, screenDevice) &&
			TestDeviceAttribute(thisHandle, screenActive))
		{
			if (SectRect(&(theWindow->portRect),
					&((**thisHandle).gdRect), &tempRect))
			{
				if (biggestSize < (tempSize = ((long)(tempRect.bottom - tempRect.top))*
					((long)(tempRect.right - tempRect.left))))
				{
					biggestSize = tempSize;
					gBiggestDevice = thisHandle;
				}
			}
		}
		thisHandle = GetNextDevice(thisHandle);
	}
	
	return gBiggestDevice;
}

short GetWindowRealDepth(WindowRef theWindow)
{
	GDHandle		theDevice;
	
	theDevice=GetBiggestDevice(theWindow);
	if (theDevice==0L)
		return 1;
	return (**(**theDevice).gdPMap).pixelSize;
}

Boolean GetWindowReallyIsColor(WindowRef theWindow)
{
	GDHandle		theDevice;
	
	theDevice=GetBiggestDevice(theWindow);
	if (theDevice==0L)
		return 1;
	return (GetWindowRealDepth(theWindow)>8) ? TRUE : TestDeviceAttribute(theDevice, gdDevType);
}

#if USE_WINDOWS_MENU
#define kDefaultHPosition		(qd.screenBits.bounds.left+10)
#define kDefaultVPosition		(qd.screenBits.bounds.top+LMGetMBarHeight()+20)

void ArrangeWindows(void)
{
	WindowRef		theWindow;
	short			h, v;
	
	if ((theWindow=GetFrontDocumentWindow())==0L)
		return;
	
	h=kDefaultHPosition;
	v=kDefaultVPosition;
	ArrangeWindowsRecurse(theWindow, &h, &v);
}

static	void ArrangeWindowsRecurse(WindowRef theWindow, short *h, short *v)
{
	short			height, width;
	
	if (theWindow!=GetLastDocumentWindow())
		ArrangeWindowsRecurse(GetNextWindow(theWindow), h, v);
	
	height=GetWindowHeight(theWindow);
	width=GetWindowWidth(theWindow);
	if (((*h)>qd.screenBits.bounds.right-width) || ((*v)>qd.screenBits.bounds.bottom-height))
	{
		*h=kDefaultHPosition;
		*v=kDefaultVPosition;
	}
	MyMoveWindow(theWindow, *h, *v, FALSE);
	(*h)+=20;
	(*v)+=20;
}
#endif
