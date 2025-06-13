/*********************************************************************

	Buggy MiniEdit.c
	
	The sample application from Inside Macintosh (RoadMap p.15-17)
	beefed up a bit by Stephen Z. Stein, Symantec Corp.
	Use this file with the “MiniEdit” chapter of your manual.
	
	The resources used in this program are in the file MiniEdit.π.rsrc.
	
	In order for THINK C to find the resource file for this
	project, be sure you’ve named the project MiniEdit.π
	
	
*********************************************************************/

#include <Windows.h>
#include <Menus.h>
#include <Resources.h>
#include <Events.h>
#include <Dialogs.h>
#include <Desk.h>
#include <Scrap.h>
#include <OSEvents.h>
#include <Memory.h>
#include <Fonts.h>
#include <TextEdit.h>
#include <ToolUtils.h>
#include <Errors.h>
#include <Files.h>
#include "mini.file.h"
#include "mini.windows.h"
#include "MiniEdit.h"



WindowRecord	wRecord;
WindowPtr		myWindow;
TEHandle		TEH;
int				linesInFolder;
Rect			dragRect = { 0, 0, 1024, 1024 };
MenuHandle		myMenus[3];
ControlHandle 	vScroll;
Cursor			editCursor;
Cursor			waitCursor;
char			dirty;

extern Str255 	theFileName;

#define	ours(w)		((myWindow != NULL) && (w == myWindow))


main() 
{
	int		myRsrc;
	
	InitGraf(&(qd.thePort));
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
	MaxApplZone();

/*
/*  The following statement is included as a check to see if we can
/*	access our program's resources.  When the project is run from
/*	THINK C, the resource file <project name>.rsrc is automatically
/*  opened.  When an application is built, these resources are 
/* 	automatically merged with the application.
/*
*/
	
	if (GetResource('MENU', fileID)==0) {
		SysBeep(20);
		CantOpen();
		return 0;
	}
	
	SetUpFiles();
	SetUpCursors();
	SetUpMenus();
	SetUpWindows();
	while (MainEvent()) ;
}



int DoMouseDown (int windowPart, WindowPtr whichWindow, EventRecord *myEvent)

{
	switch (windowPart) {
		case inGoAway:
			if (ours(whichWindow))
				if (TrackGoAway(myWindow, myEvent->where))
					DoFile(fmClose);
			break;

		case inMenuBar:
			return(DoCommand(MenuSelect(myEvent->where)));

		case inSysWindow:
			SystemClick(myEvent, whichWindow);
			break;

		case inDrag:
			if (ours(whichWindow))
				DragWindow(whichWindow, myEvent->where, &dragRect);
			break;

		case inGrow:
			if (ours(whichWindow))
				MyGrowWindow(whichWindow, myEvent->where);
			break;

		case inContent:
			if (whichWindow != FrontWindow())
				SelectWindow(whichWindow);
			else if (ours(whichWindow))
				DoContent(whichWindow, myEvent);
			break;
	}
}






int MainEvent(void) 
{
	EventRecord		myEvent;
	WindowPtr		whichWindow;
	short			windowPart;
	Rect			r;
	
	MaintainCursor();
	MaintainMenus();
	SystemTask();
	TEIdle(TEH);
	if (GetNextEvent(everyEvent, &myEvent)) {
		switch (myEvent.what) {
		case mouseDown:
			windowPart = FindWindow(myEvent.where, &whichWindow);
			DoMouseDown(windowPart, whichWindow, &myEvent);
			break;

		case keyDown:
		case autoKey: 
			{
			register char	theChar;
			
			theChar = myEvent.message & charCodeMask;
			if ((myEvent.modifiers & cmdKey) != 0) 
				return(DoCommand(MenuKey( theChar)));
			else {
				TEKey(theChar, TEH);
				ShowSelect();
				dirty = 1;
				}
			}
			break;

		case activateEvt:
			if (ours((WindowPtr)myEvent.message)) {
				if (myEvent.modifiers & activeFlag) {
					TEActivate(TEH);
					ShowControl(vScroll);
					DisableItem(myMenus[editM], undoCommand);
					TEFromScrap();
				}
				else {
					TEDeactivate(TEH);
					HideControl(vScroll);
					ZeroScrap();
					TEToScrap();
				}
			}
			break;

		case updateEvt: 
			if (ours((WindowPtr) myEvent.message))
					UpdateWindow(myWindow);
			break;
		} /* end of case myEvent.what */
	} /* if */
	return(1);
}

int SetUpMenus(void)
{
	int		i;
	
	myMenus[appleM] = NewMenu(appleID, "\p\024");
	AddResMenu(myMenus[appleM], 'DRVR');
	myMenus[fileM] = GetMenu(fileID);
	myMenus[editM] = GetMenu(editID);
	for ((i=appleM); (i<=editM); i++)
		InsertMenu(myMenus[i], 0) ;
	DrawMenuBar();
}

int DoCommand(long mResult)

{
	int		theItem;
	Str255	name;
	
	theItem = LoWord(mResult);
	switch (HiWord(mResult)) {
		case appleID:
			GetItem(myMenus[appleM], theItem, &name);
			OpenDeskAcc(name);
			SetPort(myWindow);
			break;

		case fileID: 
			DoFile(theItem);
			break;

		case editID: 
			if (SystemEdit(theItem-1) == 0) {
				switch (theItem) {
					case cutCommand:
						TECut(TEH);
						dirty = 1;
						break;
	
					case copyCommand:
						TECopy(TEH);
						break;
		
					case pasteCommand:
						TEPaste(TEH);
						dirty = 1;
						break;
		
					case clearCommand:
						TEDelete(TEH);
						dirty = 1;
						break;
				}
				ShowSelect();
			}
			break;
	}
	HiliteMenu(0);
	return(1);
}

int MaintainCursor(void)
{
	Point		pt;
	WindowPeek	wPtr;
	GrafPtr		savePort;
	
	if (ours((WindowPtr)(wPtr=(WindowPeek)FrontWindow()))) {
		GetPort(&savePort);
		SetPort((GrafPtr)wPtr);
		GetMouse(&pt);
		if (PtInRect(pt, &(**TEH).viewRect ) )
			SetCursor( &editCursor);
		else SetCursor(&qd.arrow);
		SetPort(savePort);
	}
}

int MaintainMenus(void)
{
	if ( !(*(WindowPeek)myWindow).visible || 
			!ours(FrontWindow()) ) {
		EnableItem(myMenus[fileM], fmNew);
		EnableItem(myMenus[fileM], fmOpen);
		DisableItem(myMenus[fileM], fmClose);
		DisableItem(myMenus[fileM], fmSave);
		DisableItem(myMenus[fileM], fmSaveAs);
		DisableItem(myMenus[fileM], fmRevert);
		DisableItem(myMenus[fileM], fmPrint);
		EnableItem(myMenus[editM], undoCommand);
		EnableItem(myMenus[editM], cutCommand);
		EnableItem(myMenus[editM], copyCommand);
		EnableItem(myMenus[editM], clearCommand);
	}
	else {
		DisableItem(myMenus[fileM], fmNew);
		DisableItem(myMenus[fileM], fmOpen);
		EnableItem(myMenus[fileM], fmClose);
		EnableItem(myMenus[fileM], fmSaveAs);
		EnableItem(myMenus[fileM], fmPrint);
		if (dirty && theFileName[0] != 0) {
			EnableItem(myMenus[fileM], fmRevert);
			EnableItem(myMenus[fileM], fmSave);
		}
		else {
			DisableItem(myMenus[fileM], fmRevert);
			DisableItem(myMenus[fileM], fmSave);
		}
		DisableItem(myMenus[editM], undoCommand);
		if ((**TEH).selStart==(**TEH).selEnd) {
			DisableItem(myMenus[editM], cutCommand);
			DisableItem(myMenus[editM], copyCommand);
			DisableItem(myMenus[editM], clearCommand);
		}
		else {
			EnableItem(myMenus[editM], cutCommand);
			EnableItem(myMenus[editM], copyCommand);
			EnableItem(myMenus[editM], clearCommand);
		}
	}
}

int SetUpCursors(void)
{
	CursHandle	hCurs;
	
	hCurs = GetCursor(1);
	editCursor = **hCurs;
	hCurs = GetCursor(watchCursor);
	waitCursor = **hCurs;
}


int CantOpen(void)
{
	Rect r;

	SetRect(&r, 152, 60, 356, 132);
	SetPort((myWindow = NewWindow( (Ptr) 0L, &r, "\p", true, dBoxProc, (WindowPtr) -1L, false, 0L)));
	TextFont(0);
	MoveTo(4, 20);
	DrawString("\pCan't open resource file.");
	MoveTo(4, 40);
	DrawString("\pClick mouse to exit.");
	do {
	} while (!Button());
}
