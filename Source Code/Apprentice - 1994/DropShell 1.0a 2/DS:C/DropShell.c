/******************************************************************************
**
**  Project Name:	DropShell
**     File Name:	DropShell.c
**
**   Description:	Main application code for the QuickShell
**
*******************************************************************************
**                       A U T H O R   I D E N T I T Y
*******************************************************************************
**
**	Initials	Name
**	--------	-----------------------------------------------
**	LDR			Leonard Rosenthol
**	MTC			Marshall Clow
**	SCS			Stephan Somogyi
**
*******************************************************************************
**                      R E V I S I O N   H I S T O R Y
*******************************************************************************
**
**	  Date		Time	Author	Description
**	--------	-----	------	---------------------------------------------
**	12/09/91			LDR		Added support for new "Select File�" menu item
**								Quit now sends AEVT to self to be politically correct
**								Added support for the new gSplashScreen
**	11/24/91			LDR		Added support for the Apple Menu (duh!)
**	10/29/91			SCS		Changes for THINK C 5
**	10/28/91			LDR		Officially renamed DropShell (from QuickShell)
**								Added a bunch of comments for clarification
**	10/06/91	00:02	MTC		Converted to MPW C
**	04/09/91	00:02	LDR		Added to Projector
**
******************************************************************************/


#include "DSGlobals.h"
#include "DSUserProcs.h"
#include "DSAppleEvents.h"

#include "DropShell.h"

#ifndef THINK_C
	#include <Fonts.h>
	#include <OSEvents.h>
	#include <Resources.h>
	#include <Desk.h>
#endif


Boolean		gDone, gOApped, gHasAppleEvents, gWasEvent;
EventRecord	gEvent;
MenuHandle	gAppleMenu, gFileMenu;
WindowPtr	gSplashScreen;

extern void _DataInit();	

/*
	Simple routine installed by InitDialogs for the Resume button in Bomb boxes
*/
#pragma segment Main

void Panic () {
	ExitToShell ();
}


#pragma segment Initialize
void InitToolbox (void) 
{

#ifndef THINK_C
	UnloadSeg ((Ptr) _DataInit );
#endif

	InitGraf ( &qd.thePort );
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs ((ResumeProcPtr) Panic );
	InitCursor ();
	FlushEvents ( everyEvent, 0 );
	
	// how about some memory fun! Two should be enough!
	MoreMasters ();
	MoreMasters ();
	}

/*
	Let's setup those global variables that the DropShell uses.
	
	If you add any globals for your own use,
	init them in the InitUserGlobals routine in DSUserProcs.c
*/
#pragma segment Initialize
Boolean InitGlobals (void) 
{
	long aLong;

	gDone			= false;
	gOApped			= false;	// probably not since users are supposed to DROP things!
	gHasAppleEvents	= Gestalt ( gestaltAppleEventsAttr, &aLong ) == noErr;
	gSplashScreen	= NULL;

	return(InitUserGlobals());	// call the user proc
}

/*
	Again, nothing fancy.  Just setting up the menus.
	
	If you add any menus to your DropBox - insert them here!
*/
#pragma segment Initialize
void SetUpMenus (void) {

	gAppleMenu = GetMenu ( kAppleNum );
	AddResMenu ( gAppleMenu, 'DRVR' );
	InsertMenu ( gAppleMenu, 0 );

	gFileMenu = GetMenu ( kFileNum );
	InsertMenu ( gFileMenu, 0 );
	DrawMenuBar ();
}

/*
	This routine is called during startup to display a splash screen.
	
	This was recommend by the Blue Team HI person, John Sullivan, who
	feels that all apps should display something so that users can easily
	tell what is running, and be able to switch by clicking.  Thanks John!
*/
#pragma segment Initialize
void InstallSplashScreen(void) 
{
	#define windowPicID	128

	PicHandle	picH;

	if (!gSplashScreen) {  // show the splash screen window
		picH = GetPicture(windowPicID);
		if (picH) {
			gSplashScreen = GetNewWindow(windowPicID, NULL, (WindowPtr)-1L);
			if (gSplashScreen) {
				SetWindowPic(gSplashScreen, picH);
				// Don't show it here, since we only want to it for oapp launches!
				// ShowWindow(gSplashScreen);
			}
		}
	}
}


/*	--------------- Standard Event Handling routines ---------------------- */
#pragma segment Main
void ShowAbout () {
	(void) Alert ( 128, NULL );
	}


#pragma segment Main
void DoMenu ( long retVal ) {
	short	menuID, itemID;
	Str255	itemStr;

	menuID = HiWord ( retVal );
	itemID = LoWord ( retVal );
	
	switch ( menuID ) {
		case kAppleNum:
			if ( itemID == 1 )
				ShowAbout ();	/*	Show the about box */
			else
			{
				GetItem(GetMHandle(kAppleNum), itemID, itemStr);
				OpenDeskAcc(itemStr);
			}
			break;
			
		case kFileNum:
			if ( itemID == 1 )
				SelectFile();		// call file selection userProc
			else
				SendQuitToSelf();	// send self a 'quit' event
			break;
		
		default:
			break;
			
		}
	HiliteMenu(0);		// turn it off!
	}


#pragma segment Main
void DoMouseDown ( EventRecord *curEvent ) {
	WindowPtr	whichWindow;
	short		whichPart;

	whichPart = FindWindow ( curEvent->where, &whichWindow );
	switch ( whichPart ) {
		case inMenuBar:
			DoMenu ( MenuSelect ( curEvent->where ));
			break;
		
		case inSysWindow:
			SystemClick ( curEvent, whichWindow );
			break;
		
		case inDrag:
			#ifndef THINK_C
				DragWindow ( whichWindow, curEvent->where, &qd.screenBits.bounds );
			#else
				DragWindow ( whichWindow, curEvent->where, &screenBits.bounds );
			#endif
		default:
			break;
		}
	}


#pragma segment Main
void DoKeyDown ( EventRecord *curEvent ) {
	if ( curEvent->modifiers & cmdKey )
		DoMenu ( MenuKey ((char) curEvent->message & charCodeMask ));
	}



#pragma segment Main
void main ( ) 
{

	InitToolbox ();
	if ( InitGlobals () ) {	// if we succeeding in initting self
		if ( !gHasAppleEvents )
			ErrorAlert ( kErrStringID, kCantRunErr, 0 );
		else {
			InitAEVTStuff ();
			SetUpMenus ();
			InstallSplashScreen ();
			
			while ( !gDone ) {
				gWasEvent = WaitNextEvent ( everyEvent, &gEvent, 0, NULL );
				if ( gWasEvent ) {
					switch ( gEvent.what ) {
						case kHighLevelEvent:
							DoHighLevelEvent ( &gEvent );
							break;
							
						case mouseDown:
							DoMouseDown ( &gEvent );
							break;
							
						case keyDown:
						case autoKey:
							DoKeyDown ( &gEvent );
							break;
							
						default:
							break;
					}
				}
			}
		}
		DisposeUserGlobals();	// call the userproc to clean itself up
	}
}
