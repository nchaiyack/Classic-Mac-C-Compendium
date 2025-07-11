/* ----------------------------------------------------------------------

	Background ppat
	demonstrates how to create a full-screen background window using a
	ppat. This version uses the System ppat, but you could use any ppat
	you want.
	
	Written by: Paul Celestin
	
	950620 - work begins on the project

---------------------------------------------------------------------- */


/* ----------------------------------------------------------------------
includes
---------------------------------------------------------------------- */

#include	"main.h"

Boolean				gDone,
					gWNEImplemented,
					gInBackground;
EventRecord			gTheEvent;
MenuHandle			gAppleMenu,
					gFileMenu;
WindowPtr			gTheWindow;
PixPatHandle		gPixPat;

/* ----------------------------------------------------------------------
main - here is where it all began...
---------------------------------------------------------------------- */
void main()
{
	InitToolBox();
	InitMenuBar();
	MainLoop();
	ExitToShell();
}


/* ----------------------------------------------------------------------
InitToolBox
---------------------------------------------------------------------- */
void InitToolBox()
{
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
}


/* ----------------------------------------------------------------------
InitMenuBar
---------------------------------------------------------------------- */
void InitMenuBar()
{
	Handle	myMenuBar;
	
	myMenuBar = GetNewMBar(MENU_BASE_ID);
	if (myMenuBar == NIL)
		SysBeep(1);
	else
		SetMenuBar(myMenuBar);
	
	gAppleMenu = GetMHandle(MENU_APPLE_ID);
	if (gAppleMenu == NIL)
		SysBeep(1);
	else
		AddResMenu(gAppleMenu,'DRVR');
	
	gFileMenu = GetMHandle(MENU_FILE_ID);
	if (gFileMenu == NIL)
		SysBeep(1);

	DrawMenuBar();
}


/* ----------------------------------------------------------------------
MainLoop
---------------------------------------------------------------------- */
void MainLoop()
{
	RgnHandle		cursorRgn;
	Boolean			gotEvent;
	WindowRecord	*myWinRecord;
	Rect			myWinRect;
	Str255 			myTitle;
	
	gInBackground = false;
	
	cursorRgn = NewRgn();
	
	gWNEImplemented = (NGetTrapAddress(WNE_TRAP_NUM,ToolTrap) !=
		NGetTrapAddress(UNIMPL_TRAP_NUM,ToolTrap));

	gPixPat = GetPixPat(SYSTEM_PPAT);

	myWinRect = qd.screenBits.bounds;
	myWinRecord = NIL;
	gTheWindow =
		NewCWindow(myWinRecord,&myWinRect,"\p",FALSE,plainDBox,(WindowPtr)-1L,FALSE,0L);
	
	if (gTheWindow)
	{
		ShowWindow(gTheWindow);

		gDone = false;

		while (gDone == false)
		{
			if (gWNEImplemented)
				gotEvent = WaitNextEvent(everyEvent,&gTheEvent,MIN_SLEEP,cursorRgn);
			else
			{
				SystemTask();
				gotEvent = GetNextEvent(everyEvent,&gTheEvent);
			}
		
			if (gotEvent)
				Do();
		}
	}
}


/* ----------------------------------------------------------------------
Do
---------------------------------------------------------------------- */
void Do()
{
	char	c;
	
	switch (gTheEvent.what)
	{
		case nullEvent:
			break;
		case mouseDown:
			DoMouseDown();
			break;
		case keyDown:
		case autoKey:
			c = gTheEvent.message & charCodeMask;
			if ((gTheEvent.modifiers & cmdKey) != 0)
			{
				DoMenu(MenuKey(c));
			}
			break;
		case activateEvt:
			DoUpdate();
			break;
		case updateEvt:
			DoUpdate();
			break;
		case app4Evt:
			if ((gTheEvent.message & SUSPEND_RESUME_BIT) == RESUMING)
				gInBackground = (gTheEvent.message & 0x01) == 0;
			break;
	}
}

/* ----------------------------------------------------------------------
DoUpdate
---------------------------------------------------------------------- */
void DoUpdate()
{
	GrafPtr			savedPort;
	Rect			theRect;
	
	SetCursor(&qd.arrow);
	GetPort(&savedPort);
	SetPort(gTheWindow);
	BeginUpdate(gTheWindow);
		SetRect(&theRect,gTheWindow->portRect.left,
			gTheWindow->portRect.top,
			gTheWindow->portRect.right,
			gTheWindow->portRect.bottom);
		if (gPixPat != NIL)
			FillCRect(&theRect,gPixPat);
	EndUpdate(gTheWindow);
	SetPort(savedPort);
}

/* ----------------------------------------------------------------------
DoMouseDown
---------------------------------------------------------------------- */
void DoMouseDown()
{
	WindowPtr	window;
	short int	thePart;
	long int	menuChoice, windSize, newSize;
	Boolean		doZoom, doGoAway;
	Point		p;
	
	thePart = FindWindow(gTheEvent.where,&window);
	switch (thePart)
	{
		case inMenuBar:
			menuChoice = MenuSelect(gTheEvent.where);
			DoMenu(menuChoice);
			break;
		case inSysWindow:
			SystemClick(&gTheEvent,window);
			break;
		case inContent:
			if (window != FrontWindow())
				SelectWindow(window);
			break;
		default:
			break;
	}
}

/* ----------------------------------------------------------------------
DoMenu
---------------------------------------------------------------------- */
void DoMenu(menuChoice)
long int menuChoice;
{
	int	theMenu;
	int	theItem;
	
	if (menuChoice != 0)
	{
		theMenu = HiWord(menuChoice);
		theItem = LoWord(menuChoice);
		switch (theMenu)
		{
			case MENU_APPLE_ID:
				DoMenuApple(theItem);
				break;
			case MENU_FILE_ID:
				DoMenuFile(theItem);
				break;
			default:
				break;
		}
		HiliteMenu(0);
	}
}
