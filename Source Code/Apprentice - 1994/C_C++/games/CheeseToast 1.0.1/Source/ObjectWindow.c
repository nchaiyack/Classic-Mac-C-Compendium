/************************************************************************************
 * ObjectWindow.c			- Jim Bumgardner			
 *
 * Pseudo-Object Oriented Stuff
 ************************************************************************************/
#include "ObjectWindow.h"

WindowPtr InitObjectWindow(short resID, ObjectWindowPtr theWindow, Boolean isFloating)
{
	WindowPtr wp;
	if (theWindow == NULL) {
		theWindow = (ObjectWindowPtr) NewPtrClear(sizeof(ObjectWindowRecord));
		theWindow->ownStorage = true;
	}
	else
		theWindow->ownStorage =false;
	theWindow->floating = false;
	wp = GetNewCWindow(resID, (WindowPtr) theWindow, (WindowPtr) -1L);
	((WindowPeek) wp)->refCon = MyWindowID;
	theWindow->Update = DefaultUpdate;
	theWindow->Activate = DefaultActivate;
	theWindow->HandleClick = DefaultHandleClick;
	theWindow->Dispose = DefaultDispose;
	theWindow->Draw = DefaultDraw;
	theWindow->Idle = NULL;

	((ObjectWindowPtr) theWindow)->floating = isFloating;

	LocateWindows();
	return wp;
}

void DefaultDispose(WindowPtr theWin)
{
	CloseWindow(theWin);
	if (((ObjectWindowPtr) theWin)->ownStorage)
		DisposPtr((Ptr) theWin);
}

void DefaultUpdate(WindowPtr theWin)
{
	GrafPtr	savePort;
	GetPort(&savePort);
	SetPort(theWin);
	BeginUpdate(theWin);
	((ObjectWindowPtr) theWin)->Draw(theWin);
	EndUpdate(theWin);
	SetPort(savePort);
}

void DefaultActivate(WindowPtr theWin)
{
	GrafPtr	savePort;
	GetPort(&savePort);
	SetPort(theWin);
	InvalRect(&theWin->portRect);
	SetPort(savePort);
}

void DefaultHandleClick(WindowPtr theWin, Point where)
{
}

void DefaultDraw(WindowPtr theWin)
{
}


/************************************************************************************
 * Routines for maintaining floating windows.						
 *
 ************************************************************************************/

WindowPtr FrontDocument(void)
{
	WindowPeek	theWin;
	theWin = (WindowPeek) FrontWindow();
	while (theWin && ((ObjectWindowPtr) theWin)->floating)
		theWin = theWin->nextWindow;
	if (theWin && !((ObjectWindowPtr) theWin)->floating)
		return (WindowPtr) theWin;
	else
		return NULL;
}

WindowPtr FrontFloating(void)
{
	WindowPeek	theWin;
	theWin = (WindowPeek) FrontWindow();
	while (theWin && !((ObjectWindowPtr) theWin)->floating)
		theWin =  theWin->nextWindow;
	if (theWin && ((ObjectWindowPtr) theWin)->floating)
		return (WindowPtr) theWin;
	else
		return NULL;
}

void SelectDocument(WindowPtr theWindow)
{
	WindowPeek	frontFloat;
	frontFloat = (WindowPeek) FrontFloating();
	if (frontFloat == NULL)
		SelectWindow(theWindow);
	else {
		while ( frontFloat->nextWindow &&
				((ObjectWindowPtr) frontFloat->nextWindow)->floating)
			frontFloat = frontFloat->nextWindow;
		SendBehind(theWindow, (WindowPtr) frontFloat);
		HiliteWindow(theWindow,true);
		CalcVisBehind((WindowPeek) FrontWindow(),((WindowPeek) theWindow)->strucRgn);
		PaintOne((WindowPeek) theWindow,((WindowPeek) theWindow)->strucRgn);
	}
}

void SelectFloating(WindowPtr theWindow)
{
	SelectWindow(theWindow);
}

void LocateWindows(void)
{
//	WindowPtr	frontDoc;
//	if ((frontDoc = FrontDocument()) != NULL) {
//		SelectDocument(frontDoc);
//	}
}