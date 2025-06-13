/* ==========================================

	nsh.c
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

#include <Traps.h>
#include <GestaltEqu.h>

#include "multi.h"
#include "nsh.h"

#include "cmd.proto.h"
#include "event.proto.h"
#include "find.proto.h"
#include "memory.proto.h"
#include "menu.proto.h"
#include "multi.proto.h"
#include "nsh.proto.h"
#include "path.proto.h"
#include "utility.proto.h"
#include "wind.proto.h"
#include "str_utl.proto.h"

#if ! FULL_APP
#include "drop.proto.h"
#endif

/* ========================================== */

Rect		gDragRect = { 0, 0, 1024, 1024 };

CursHandle	gEditCursor;		// for convenience cursors are preloaded
CursHandle	gWaitCursor;

Boolean		gHasAppleEvents;	// true if the Apple Events are available
Boolean		gHasFSSpec;			// true if the FSS calls are available
Boolean		gHasFindFolder;		// true if the findfolder call is there
Boolean		gHasNewStdFile;		// true if the new SF calls are there
Boolean		gInBackground;		// maintained by Initialize and DoEvent
Boolean		gEventOpen;			// true if we received an "oapp" Apple Event
Boolean		gEventQuit;			// true if we received a "quit" Apple Event

/* ========================================== */

main(void) 
{
	nsh_init();
		
	if ( !gHasAppleEvents )
#if FULL_APP
		if ( !nsh_app_files() )
			mf_1st_new();
#else
		drop_open();
#endif
			

	while (TRUE) nsh_loop();
}

void nsh_init(void)
{
	InitGraf(&thePort);							// apple routines
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
	MaxApplZone();
	
	gEditCursor = GetCursor(iBeamCursor);		// preload cursors
	gWaitCursor = GetCursor(watchCursor);

	gInBackground = 0;
	gEventOpen = 0;
	gEventQuit = 0;

	gHasAppleEvents = TestGestaltBit( gestaltAppleEventsAttr, gestaltAppleEventsPresent );
	gHasFSSpec = TestGestaltBit( gestaltFSAttr, gestaltHasFSSpecCalls );
	gHasFindFolder = TestGestaltBit( gestaltFindFolderAttr, gestaltFindFolderPresent );
	gHasNewStdFile = TestGestaltBit( gestaltStandardFileAttr, gestaltStandardFile58 );
	
	multi_init();								// init nShell routines
	mem_init();
	menu_init();
	find_init();
	cmd_init();
	path_init();
	
#if ! FULL_APP
	drop_init();
#endif

	event_init();

}

void nsh_quit( void )
{
	if (multi_close_all()) {
		cmd_unload_all();
		ExitToShell();
		}
	else
		gEventQuit = 0;
}

void nsh_loop(void) 
{
	Boolean		gotEvent;
	EventRecord	event;
	Point		mouse;

	mem_test();

	if (gEventOpen) {
	
#if FULL_APP
		mf_1st_new();
#else
		drop_open();
#endif
		gEventOpen = 0;
		}

	if (gEventQuit)
		nsh_quit();

	multi_task();

	gotEvent = WaitNextEvent(everyEvent, &event, nsh_sleep(), NULL);
		
	if ( gotEvent ) {
		nsh_cursor();
		nsh_events(&event);
		}
	else
		nsh_idle_task();
}

long nsh_sleep( void )
{
	return( gInBackground ? 10 : 1 );
}

void nsh_idle_task(void) 
{
	nsh_cursor();
	menu_hilite();
}

void nsh_events( EventRecord *myEvent ) 
{
	char			theChar;
	WindowPtr		whichWindow;
	short			windowPart;
	Rect			r;
	ShellH			shell;
	
	switch (myEvent->what) {

		case mouseDown:
			windowPart = FindWindow(myEvent->where, &whichWindow);
			nsh_mouse(windowPart, whichWindow, myEvent);
			break;

		case keyDown:
		case autoKey: 
			theChar = myEvent->message & charCodeMask;
			if ((myEvent->modifiers & cmdKey) != 0) {
				if (theChar == '.')
					multi_cancel();
				else
					menu_command(MenuKey( theChar));
				}
			else 
				multi_key(theChar);
			break;

		case activateEvt:
			if (shell = multi_find((WindowPtr)myEvent->message)) {
				if (myEvent->modifiers & activeFlag) 
					multi_activate(shell);
				else 
					multi_deactivate(shell);
				}
			break;

		case updateEvt: 
			if (shell = multi_find((WindowPtr)myEvent->message))
				wind_update(shell);
			break;

		case osEvt:
	  	    if ( ( myEvent->message & suspendResumeMessage) == resumeFlag )
	  	    	gInBackground = 0;
			else
				gInBackground = 1;
  	 		 break;
  	 		 
		case kHighLevelEvent:
			event_do(myEvent);
			break;
			
		default:
			break;
		}
}

void nsh_mouse(int windowPart, WindowPtr whichWindow, EventRecord *myEvent)
{
	ShellH	shell;
	
	shell = multi_find(whichWindow);
	
	switch (windowPart) {
		case inMenuBar:
			menu_command(MenuSelect(myEvent->where));
			break;

		case inSysWindow:
			SystemClick(myEvent, whichWindow);
			break;

		case inGoAway:
			if (shell)
				if (TrackGoAway(whichWindow, myEvent->where))
					multi_close( shell );
			break;

		case inDrag:
			if (shell)
				DragWindow(whichWindow, myEvent->where, &gDragRect);
			break;

		case inGrow:
			if (shell)
				wind_grow(shell, myEvent->where);
			break;

		case inZoomIn:
		case inZoomOut:
			if (shell)
				if ( TrackBox(whichWindow, myEvent->where, windowPart) )
					wind_zoom(shell, windowPart);
			break;

		case inContent:
			if (whichWindow != FrontWindow())
				SelectWindow(whichWindow);
			else if (shell)
				wind_action(shell, myEvent);
			break;
	}
}

void nsh_cursor(void)
{
	Point			pt;
	GrafPtr			savePort;
	ShellH			shell;
	Rect			myRect;
	
	shell = multi_front();
	
	if (shell) {
		GetPort(&savePort);
		SetPort((**shell).WindPtr);
		GetMouse(&pt);
		myRect = (**(**shell).Text).viewRect;
		if (PtInRect(pt, &myRect ) )
			SetCursor( *gEditCursor );
		else
			SetCursor(&arrow);
		SetPort(savePort);
		}
}

#if FULL_APP

int nsh_app_files( void )
{
	short	What;
	short	HowMany;
	short	GotSome;
	short	i;
	AppFile	afile;
	FSSpec	fss;
	
	GotSome = 0;

	CountAppFiles(&What,&HowMany);
	
	for (i=1; i<=HowMany; i++) {
		GetAppFiles( i, &afile );
		if ( ( afile.fType == TRANSCRIPT_FILE ) || ( afile.fType == SCRIPT_FILE ) ) {
			pStrCopy( fss.name, afile.fName );
			fss.vRefNum = afile.vRefNum;
			fss.parID = 0;
			if ( afile.fType == TRANSCRIPT_FILE )
				mf_open_file( &fss );
			else
				mf_open_script( &fss );
			GotSome = 1;
			}
		ClrAppFiles( i );
		}
		
	return(GotSome);
}

#endif
