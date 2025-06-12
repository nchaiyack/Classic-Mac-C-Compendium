#include "flight.h"



WindowPtr		theWind;	/*the main window*/
WindowRecord	windRecord;	/*storage for the window*/


GrafPtr			windManPort;	/*window manager port*/
EventRecord 	theEvent;	/*the main event*/


MenuHandle		appleMenu, fileMenu, editMenu;


main()
{
	Init();
	MakeMenus();
	MakeWindow();
	AddrsSet();

	MakeGrid();
DebugStr("\PHello");

	while (1)
	{
		SystemTask();
		if (GetNextEvent(everyEvent, &theEvent))
			DoEvent();
	}
}

DoEvent()
{
	switch (theEvent.what) {
	case mouseDown:
		DoMouseDown();
		break;
	case keyDown:
	case autoKey:
		DoKeyDown();
		break;
	case updateEvt:
		DoUpdate();
		break;
	case diskEvt:
		break;
	case activateEvt:
		DoActivate();
		break;
	}
}

DoMouseDown()
{
	int thePart;
	WindowPtr whichWindow;

	thePart = FindWindow( theEvent.where, &whichWindow);

	switch (thePart) {
	case inDesk:
		break;
	case inMenuBar:
		DoMenuClick();
		break;
	case inSysWindow:
		SystemClick(&theEvent, whichWindow);
		break;
	case inContent:
		DoContent(whichWindow);
		break;
	case inDrag:
		DoDrag(whichWindow);
		break;
	case inGrow:
		DoGrow(whichWindow);
		break;
	case inGoAway:
		if (TrackGoAway(whichWindow, theEvent.where))
			DoQuit();
		break;
	case inZoomIn:
	case inZoomOut:
		DoZoom(whichWindow, thePart);
		break;
	}
}

DoMenuClick()
{
	long menuChoice;

	menuChoice = MenuSelect(theEvent.where);
	DoMenuChoice(menuChoice);
}

DoMenuChoice(menuChoice)
long menuChoice;
{
	switch(HiWord(menuChoice)) {
	case MENU_APPLE:
		DoAppleChoice( LoWord(menuChoice) );
		break;
	case MENU_FILE:
		DoFileMenu(LoWord(menuChoice));
		break;
	case MENU_EDIT:
		DoEditMenu(LoWord(menuChoice));
		break;
	}

	HiliteMenu(0); /* unhilight the selected menu */
}

DoAppleChoice(theItem)
int theItem;
{
	char	accName[256];
	int		accNum, i;
	
	if (theItem EQ 1)	
		Alert(ALRT_ABOUT, NIL);
	else
	{
		GetItem(appleMenu, theItem, accName);
		accNum = OpenDeskAcc(accName);
	}
}

DoFileMenu(theItem)
int theItem;
{
	if (theItem EQ 1)
		DoQuit();
}

DoQuit()
{
	ExitToShell();
}

DoEditMenu(theItem)
int theItem;
{
	switch(theItem) {
	case 1: /* Undo */
		SystemEdit(undoCmd);
		break;
	case 3: /* Cut */
		SystemEdit(cutCmd);
		break;
	case 4: /* Copy */
		SystemEdit(copyCmd);
		break;
	case 5: /* Paste */
		SystemEdit(pasteCmd);
		break;
	case 6: /* Clear */
		SystemEdit(clearCmd);
		break;
	}
}

DoContent(whichWindow)
WindowPtr whichWindow;
{
	Point	localPt;

	if (whichWindow NEQ FrontWindow())
		SelectWindow(whichWindow);
	else
	{
		localPt = theEvent.where;
		GlobalToLocal(&localPt);
		DoMouseMove(&localPt);
	}
}

DoDrag(whichWindow)
WindowPtr whichWindow;
{
	Rect limitRect;

	if (whichWindow NEQ FrontWindow())
		SelectWindow(whichWindow);
	else
	{
		SetRect(&limitRect, 0, MENU_BAR_HEIGHT, 5000, 5000);
		DragWindow(whichWindow, theEvent.where, &limitRect);
	}
}

DoGrow(whichWindow)
WindowPtr whichWindow;
{
	Rect limitRect;
	long	newSize;

return;
	SetRect(&limitRect, 50, 50, 2000, 2000);
	newSize = GrowWindow(whichWindow, theEvent.where, &limitRect);
	if (newSize)
	{
		EraseRect(&whichWindow->portRect);
		SizeWindow(whichWindow, LoWord(newSize), HiWord(newSize), TRUE);
	}
}

DoKeyDown()
{
	char	ch;
	long	menuChoice;
	
	ch = (char)(theEvent.message & charCodeMask);
	
	if ((theEvent.modifiers & cmdKey)
		AND (theEvent.what NEQ autoKey)) /* ignore repeats */
	{
		menuChoice = MenuKey(ch);
		if (menuChoice)
			DoMenuChoice(menuChoice);
	}
}

DoUpdate()
{
    GrafPtr savePort;		/* to save and restore the old port */
	WindowPtr whichWindow;

	whichWindow = (WindowPtr) (theEvent.message);

    BeginUpdate(whichWindow);	/* reset ClipRgn etc to only redraw what's
				 			  necessary. */

    	GetPort(&savePort);		/* don't trash the port; we might be
								   updating an inactive window */
    	SetPort(whichWindow);	/* work in the specified window */

		if (whichWindow EQ theWind)
		{
			EraseRect(&(whichWindow->portRect));
			DoForward();
		}
	EndUpdate(whichWindow);

	SetPort(savePort);
}

DoActivate()
{
	WindowPtr whichWindow;

	whichWindow = (WindowPtr) (theEvent.message);
	if (whichWindow EQ theWind)
		/*DrawGrowIcon(whichWindow);*/

	if (theEvent.modifiers & activeFlag)
		SetPort(whichWindow);
}

DoZoom(whichWindow, thePart)
WindowPtr	whichWindow;
int			thePart;
{
return;

	if (TrackBox(whichWindow, theEvent.where, thePart))
		ZoomWindow(whichWindow, thePart, TRUE);
}



Init()
{
	pascal void restartProc();

	InitGraf(&thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(restartProc);		
	InitCursor();
	
	FlushEvents(everyEvent, 0);

    GetWMgrPort(&windManPort);	/* get whole screen port that window mgr
				   uses */
    SetPort(windManPort);	/* and start off with it */

	SetEventMask(everyEvent);
}

/* when the world comes to an end */
pascal void
restartProc()
{
	ExitToShell();
}

MakeMenus()
{
	appleMenu = GetMenu(MENU_APPLE);
	AddResMenu(appleMenu, 'DRVR');
	InsertMenu(appleMenu, 0);
	
	fileMenu = GetMenu(MENU_FILE);
	InsertMenu(fileMenu, 0);
	
	editMenu = GetMenu(MENU_EDIT);
	InsertMenu(editMenu, 0);
	
	DrawMenuBar();
}

MakeWindow()
{
	theWind = GetNewWindow(WIND_MAIN, &windRecord, (WindowPtr)-1L);	
}