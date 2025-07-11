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
**	  Date		Author	Description
**	---------	------	---------------------------------------------
**	23 Jun	94	LDR		Implemented support for disk insertion events
**	02 Feb	94	LDR		Updated for Final SDK & CodeWarrior a2
**						Removed the ResumeProc as per new Apple recommendations
**	11 Dec 93	SCS		Universal Headers/UPPs (Phoenix 68k/PPC & PPCC)
**						Skipped System 6 compatible rev of DropShell source
**	09 Dec 91	LDR		Added support for new "Select File�" menu item
**						Quit now sends AEVT to self to be politically correct
**						Added support for the new gSplashScreen
**	24 Nov 91	LDR		Added support for the Apple Menu (duh!)
**	29 Oct 91	SCS		Changes for THINK C 5
**	28 Oct 91	LDR		Officially renamed DropShell (from QuickShell)
**						Added a bunch of comments for clarification
**	06 Oct 91	MTC		Converted to MPW C
**	09 Apr 91	LDR		Added to Projector
**
******************************************************************************/

#ifndef __MWERKS__
#include <Desk.h>
#include <Dialogs.h>
#include <Errors.h>
#include <Files.h>
#include <Fonts.h>
#include <Memory.h>
#include <Menus.h>
#include <StandardFile.h>
#include <TextEdit.h>
#include <Types.h>
#include <Windows.h>
#endif

#include "DSGlobals.h"
#include "DSUserProcs.h"
#include "DSAppleEvents.h"

#include "DropShell.h"



Boolean		gDone, gOApped, gHasAppleEvents, gWasEvent;
EventRecord	gEvent;
MenuHandle	gAppleMenu, gFileMenu;
WindowPtr	gSplashScreen;

#ifdef MPW
extern void _DataInit();	
#endif



void InitToolbox (void) 
{

#ifdef MPW
	UnloadSeg ((Ptr) _DataInit );
#endif

	InitGraf ( &qd.thePort );
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (NULL);		// use of ResumeProcs no longer approved by Apple
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

void SetUpMenus (void) {

	gAppleMenu = GetMenu ( kAppleNum );
	AppendResMenu ( gAppleMenu, 'DRVR' );
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

void ShowAbout () {
	(void) Alert ( 128, NULL );
	}



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
				GetMenuItemText(GetMenuHandle(kAppleNum), itemID, itemStr);
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
			{
				Rect	boundsRect = (*GetGrayRgn())->rgnBBox;
				DragWindow ( whichWindow, curEvent->where, &boundsRect );
			}
		default:
			break;
		}
	}



void DoKeyDown ( EventRecord *curEvent ) {
	if ( curEvent->modifiers & cmdKey )
		DoMenu ( MenuKey ((char) curEvent->message & charCodeMask ));
	}




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

						case diskEvt:
							if (HiWord(gEvent.message)) {
								Point diskInitPt;
								
								diskInitPt.v = diskInitPt.h = 100;
								DILoad();
								DIBadMount(diskInitPt, gEvent.message);
								DIUnload();
							}
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
