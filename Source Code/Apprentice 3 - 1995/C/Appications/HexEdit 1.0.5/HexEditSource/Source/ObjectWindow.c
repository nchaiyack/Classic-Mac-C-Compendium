/*********************************************************************
 * ObjectWindow.c
 *
 * A pre-c++ set of routines for implementing object-like windows.
 *
 * HexEdit, a simple hex editor
 * copyright 1993, Jim Bumgardner
 *********************************************************************/
#include "ObjectWindow.h"

WindowPtr InitObjectWindow(short resID, ObjectWindowPtr theWindow, Boolean isFloating)
{
	WindowPtr wp;
	if (theWindow == NULL) {
		theWindow = (ObjectWindowPtr) NewPtrClear(sizeof(ObjectWindowRecord));
		if (theWindow == NULL)
			return NULL;
		theWindow->ownStorage = true;
	}
	else
		theWindow->ownStorage =false;
	theWindow->floating = false;
	wp = GetNewWindow(resID, (WindowPtr) theWindow, (WindowPtr) -1L);
	((WindowPeek) wp)->refCon = MyWindowID;
	theWindow->Update = DefaultUpdate;
	theWindow->Activate = DefaultActivate;
	theWindow->HandleClick = DefaultHandleClick;
	theWindow->Dispose = DefaultDispose;
	theWindow->Draw = DefaultDraw;
	theWindow->Idle = NULL;
	((ObjectWindowPtr) theWindow)->floating = isFloating;
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

void DefaultActivate(WindowPtr theWin, Boolean active)
{
	GrafPtr	savePort;
	GetPort(&savePort);
	SetPort(theWin);
	InvalRect(&theWin->portRect);
	SetPort(savePort);
	((ObjectWindowPtr) theWin)->active = active;
}

void DefaultHandleClick(WindowPtr theWin, Point where, EventRecord *er)
{
}

void DefaultDraw(WindowPtr theWin)
{
}

/************************************************************************************
 * Routines for maintaining floating windows.						
 *
 ************************************************************************************/

