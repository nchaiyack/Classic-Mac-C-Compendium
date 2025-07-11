/*************
  appkiller.c
  
  This file contains main() which in turn initializes all the different
  parts of the program and Operating System.  The main event loop
  is in main().
*************/

#ifndef THINK_C
 #include <AppleEvents.h>
 #include <Desk.h>
 #include <Dialogs.h>
 #include <Events.h>
 #include <Fonts.h>
 #include <GestaltEqu.h>
 #include <Menus.h>
 #include <OSEvents.h>
 #include <QuickDraw.h>
 #include <ToolUtils.h>
 #include <Types.h>
 #include <Windows.h>
#else
 #include "ak_headers"
#endif
#include "aevent.h"
#include "appkiller.h"
#include "ak_ppc.h"
#include "trapavail.h"

static void Init(void);
static void Check4Sys7(void);
static void SetupMenus(void);
static void DoEvent(void);
static void DoMouseDown(void);
static void DoKeyDown(void);
static void DoCommand(long);
static void DoHLEvent(EventRecord *);
       void DoAlert(Str255, Str255, Str255, Str255);
static void DoAbout(void);

DialogPtr		gAboutDlog;					/* About box...        */
WindowPtr		gWhichWindow;				/* window of event     */
MenuHandle		gAppleM;					/* handles to menus    */
EventRecord		gTheEvent;					/* the current event   */
int				gHaveEvent;					/* do we have an event */
long			gSleepTime = 15;			/* time to sleep/WNE   */
int				gEventMask = everyEvent;	/* current event mask  */
RgnHandle		gMouseRgn = NULL;			/* current mouse rgn   */


void main()
{
	Init();
	Check4Sys7();	/* this exits if Sys 7 isn't running */
	InitAEs();		/* initialize my Apple Event handlers */
	SetupMenus();
	MyPPCInit();

	PostWithPPCBrowser();
	while (1) {
		gHaveEvent = WaitNextEvent( gEventMask, &gTheEvent, 
			gSleepTime, gMouseRgn);
		if (gHaveEvent)
			DoEvent();
	} /* while */
} /* main() */


static void Init()
{
	register int i;
	InitGraf(&thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NULL);
	InitCursor();
	for (i=0; i<3; i++) (void)MoreMasters();	
	FlushEvents(everyEvent, 0);
	SetEventMask(everyEvent);
} /* Init() */


static void Check4Sys7(void)
{
#define _Gestalt 0xa1ad   /* Gestalt trap number (from IM) */
	if (TrapAvailable(_Gestalt)) {
		long result, mask=0x0700; /* mask = system 7.x */

		if ( (Gestalt( gestaltSystemVersion, &result) == noErr) &&
			( mask == (result & mask)) )
			return;
	}
	/* Let user know we need sys 7, */
	/* if Gestalt isn't on the system, don't even check the others */
	DoAlert("\pThe Application Killer requires",
		    "\pSystem 7 in order to work","\pproperly.","\p");
	ExitToShell();

} /* Check4Sys7() */


static void SetupMenus(void)
{
	Handle mbarH;
	
	mbarH = (Handle)GetNewMBar(MENU_BAR);
	if (mbarH == NULL) {
		SysBeep(1); SysBeep(1);
		ExitToShell();
	}
	SetMenuBar( mbarH);
	gAppleM= GetMenu(APPLE_MENU);
	AddResMenu( gAppleM, 'DRVR');
	DrawMenuBar();
} /* SetupMenus() */


void Cleanup(void)
{
	ExitToShell();
} /* Cleanup() */


static void DoEvent(void)
{
	switch (gTheEvent.what) {
		case mouseUp:
		case mouseDown:
			DoMouseDown();
			break;
		case keyDown:
		case autoKey:
			DoKeyDown();
			break;
		case activateEvt:
		case updateEvt:
		case osEvt:
			break;
		case kHighLevelEvent:
			DoHLEvent(&gTheEvent);
			break;
	}
} /* DoEvent() */


static void DoMouseDown(void)
{
	switch (FindWindow( gTheEvent.where, &gWhichWindow)) {
		case inSysWindow:
			SystemClick( &gTheEvent, gWhichWindow);
			break;
		case inMenuBar:
			DoCommand( MenuSelect(gTheEvent.where) );
			break;
		case inDrag:
		case inContent:
		case inGoAway:
			break;
	}
} /* DoMouseDown() */


static void DoKeyDown(void)
{
	char theChar = (char)(gTheEvent.message & charCodeMask);
	if (gTheEvent.modifiers & cmdKey)
		DoCommand( MenuKey(theChar));
} /* DoKeyDown() */


static void DoCommand(long mResult)
{
	int theItem = LoWord(mResult);
	int theMenu = HiWord(mResult);
	Str255 name;
	int temp;
	
	switch (theMenu) {
		case APPLE_MENU:
			if (theItem == ABOUT_ITEM) {
				DoAbout();
			} else {
				GetItem( gAppleM, theItem, name);
				temp = OpenDeskAcc(name);
			}
			break;
			
		case FILE_MENU:
			switch (theItem) {
				case POST_ITEM:
					PostWithPPCBrowser();
					break;
				case LAUNCH_ITEM:
					DoLaunch();
					break;
				case QUIT_ITEM:
					Cleanup();
					break;
			} /* switch theItem */
			break;
			
	} /* switch theMenu */
	HiliteMenu(0);		/* unhilite the selected menu */
} /* DoCommand() */


static void DoHLEvent(EventRecord *theEvent)
{
	OSErr err;

	DoAppleEvent(theEvent);

} /* DoHLEvent() */


void DoAlert(Str255 s0,Str255 s1,Str255 s2,Str255 s3)
{
	ParamText(s0,s1,s2,s3);
	Alert(AlertID, NULL);
} /* DoAlert() */


static void DoAbout(void)
{
	short itemHit = -9;
	if ((gAboutDlog = GetNewDialog(AboutDlogID, NULL, (void*)(-1))) != NULL)
	{
		SelectWindow((WindowPtr)gAboutDlog);
		while (itemHit != AboutOKButt)
			ModalDialog( NULL, &itemHit);
		DisposDialog(gAboutDlog);
		gAboutDlog = NULL;
	} else
		SysBeep(5);       /* beep if we can't get dialog box */
} /* DoAbout() */




