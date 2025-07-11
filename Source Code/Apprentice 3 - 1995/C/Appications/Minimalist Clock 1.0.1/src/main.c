/* ----------------------------------------------------------------------

	Minimalist Clock
	version 1.0.0
	7 December 1994
	
	Written by: Paul Celestin
	
	This simple application does nothing but display a simple clock in
	a rectangle on your display. You can move the clock around, and you
	can also check the date by clicking once on the clock.
	
	941207 - 1.0.0 initial release

---------------------------------------------------------------------- */


/* ----------------------------------------------------------------------
includes
---------------------------------------------------------------------- */

#include	"the_defines.h"
#include	"the_globals.h"
#include	"the_prototypes.h"

Boolean			gDone,
				gWNEImplemented,
				gInBackground,
				gDrawDate = FALSE;
long			gTicksOld = 0, gTicksNew = 0;
EventRecord		gTheEvent;
MenuHandle		gAppleMenu,
				gFileMenu,
				gEditMenu;
WindowPtr		gClock;

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

	gEditMenu = GetMHandle(MENU_EDIT_ID);
	if (gEditMenu == NIL)
		SysBeep(1);
	
	DrawMenuBar();
}

/* ----------------------------------------------------------------------
MainLoop
---------------------------------------------------------------------- */
void MainLoop()
{
	RgnHandle	cursorRgn;
	Boolean		gotEvent;
	
	gDone = false;
	gInBackground = false;
	
	cursorRgn = NewRgn();
	
	gWNEImplemented = (NGetTrapAddress(WNE_TRAP_NUM,ToolTrap) !=
		NGetTrapAddress(UNIMPL_TRAP_NUM,ToolTrap));
	
	CreateClock();
	
	while (gDone == false)
	{
		if (gWNEImplemented)
		gotEvent =
			WaitNextEvent(everyEvent,&gTheEvent,MIN_SLEEP,cursorRgn);
		else
		{
			SystemTask();
			gotEvent = GetNextEvent(everyEvent,&gTheEvent);
		}
		
		if (gotEvent)
			Do();
		else
			DoIdle();
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
			DoIdle();
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
			break;
		case updateEvt:
			break;
		case app4Evt:
			if ((gTheEvent.message & SUSPEND_RESUME_BIT) == RESUMING)
			{
				gInBackground = (gTheEvent.message & 0x01) == 0;
			}
			else
				DoIdle();
			break;
	}
}

/* ----------------------------------------------------------------------
DoIdle
---------------------------------------------------------------------- */
void DoIdle()
{
	gTicksNew = TickCount();
	if ((gTicksNew - gTicksOld) > UPDATE_INTERVAL)
	{
		gTicksOld = gTicksNew;
		UpdateClock();
	}
}

/* ----------------------------------------------------------------------
DoUpdate
---------------------------------------------------------------------- */
void DoUpdate(window)
WindowPtr window;
{
	GrafPtr	savedPort;
	
	GetPort(&savedPort);
	SetPort(window);
	BeginUpdate(window);
	UpdateClock();
	EndUpdate(window);
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
		case inGrow:
			newSize = GrowWindow(window,gTheEvent.where,&(qd.screenBits.bounds));
			SizeWindow(window,LoWord(newSize),HiWord(newSize),false);
			DoUpdate(window);
			break;
		case inDrag:
			gDrawDate = TRUE;
			DragWindow(window,gTheEvent.where,&(qd.screenBits.bounds));
			DoUpdate(window);
			break;
		case inZoomIn:
			if (doZoom = TrackBox(window,gTheEvent.where,inZoomIn))
			{
				SizeWindow(window,200,200,false);
				DoUpdate(window);
			}
			break;
		case inZoomOut:
			if (doZoom = TrackBox(window,gTheEvent.where,inZoomOut))
			{
				short winHeight,winWidth;

				winHeight = qd.screenBits.bounds.bottom - 43;
				winWidth = qd.screenBits.bounds.right - 5;
				SizeWindow(window,winWidth,winHeight,false);
				MoveWindow(window,2,LMGetMBarHeight() + 20,false);
				DoUpdate(window);
			}
			break;
			break;
		case inGoAway:
			if (doGoAway = TrackGoAway(window,gTheEvent.where))
				CloseWindow(window);
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
			case MENU_EDIT_ID:
				DoMenuEdit(theItem);
				break;
			default:
				break;
		}
		HiliteMenu(0);
	}
}
