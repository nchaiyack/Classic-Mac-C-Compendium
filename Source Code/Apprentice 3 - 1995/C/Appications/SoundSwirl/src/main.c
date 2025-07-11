/*
** main.c
** 
** Main control center for the application skeleton
*/

#include "main.h"		/* general application header */

static pascal void restartProc(void);
static void Init(void);
static void MakeWindows(void);
static void SetupMenus(void);


void main(void)
{
	EventRecord		theEvent;				/* the current event   */
	int				eventMask = everyEvent;	/* current event mask  */
	int				haveEvent;				/* do we have an event */
	long			sleepTime = 0;			/* time to sleep/WNE   */
	Boolean			weHaveWNE=0;			/* do we have MF or 7  */
	
	gMainWindow = NULL;


	Init();
	InitSound();
	SetupMenus();
	MakeWindows();
	weHaveWNE = TrapAvailable( 0xA860);		/* WaitNextEvent() */

	/************* main event loop ****************/

	/* HideCursor();    /* */
	while (1) {
		if (weHaveWNE)
		{
			if (WaitNextEvent( eventMask, &theEvent, sleepTime, NULL))
				DoEvent(&theEvent);
		}
		else
		{
			SystemTask();
			if ( GetNextEvent(eventMask, &theEvent) ) DoEvent(&theEvent);
		}
		
		DrawStep();
	} /* while */
} /* main() */


/***********************
** Init()
**
** This initializes the toolbox
************************/

static void Init()
{
	register int i;
	GrafPtr	windManPort;				/* window manager port */

	
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(restartProc);		
	InitCursor();
	
	FlushEvents(everyEvent, 0);

	for (i=0; i<6; i++) (void)MoreMasters();
	
    GetWMgrPort(&windManPort);
    
    SetPort(windManPort);
    
	SetEventMask(everyEvent);
} /* Init() */



/**********************
** restartProc()
**
** This is used for the system error/bomb dialog boxes.
** Just in case it crashes, we may have a way out.
***********************/

static pascal void restartProc(void)
{
	ExitToShell();
} /* restartProc() */



/***********************
** SetupMenus()
**
** loads the menubar and menus.  Adds DAs to apple menu.
************************/
static void SetupMenus(void)
{
	Handle mbarH;
	
	mbarH = (Handle)GetNewMBar(MENU_BAR);
	if (mbarH == NULL) {
		SysBeep(1); SysBeep(1);
		ExitToShell();
	}
	SetMenuBar( mbarH);
	gAppleM= GetMHandle(APPLE_MENU);
	AddResMenu( gAppleM, 'DRVR');
	gFileM = GetMHandle(FILE_MENU);
	gEditM = GetMHandle(EDIT_MENU);
	gListenM = GetMHandle(LISTEN_MENU);

	DisableItem( gListenM, ListenStop);

	DrawMenuBar();
	DisposHandle(mbarH);  /* don't need it anymore */
} /* SetupMenus() */



static void MakeWindows(void)
{
	gMainWindow= GetNewWindow( MainWindID, NULL, (WindowPtr)-1L);
	SelectWindow(gMainWindow);
	SetPort( gMainWindow);
} /* MakeWindows() */


/****************************
** Cleanup()
**
** The main program exiting procedure.  It cleans up after
** ourselves
*****************************/
void Cleanup(void)
{
	DoFileClose();	/* dostuff.c - dispose of any dynamic stuff */
	KillSound();
	ExitToShell();
} /* Cleanup() */








