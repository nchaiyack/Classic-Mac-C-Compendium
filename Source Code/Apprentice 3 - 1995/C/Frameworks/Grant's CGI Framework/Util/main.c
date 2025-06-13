/*****
 *
 *	Grant's CGI Shell (Common Grant Interface :-)
 *		http://arpp1.carleton.ca/grant/mac/cgi.html
 *
 *	main.c
 *
 *	Sample application using the cgi interface.
 *
 *	MyCGIProcess is where you will do your application specific processing of the cgi stuff.
 *
 *	by Grant Neufeld (with help from Scott T. Boyd among others)
 *		the original source that this was inspired by is "Responder" written by John O'Fallon
 *		see his site: http://www.maxum.com/maxum/
 *
 *	Copyright ©1995 by Grant Neufeld
 *
 *	http://arpp1.carleton.ca/grant/
 *	gneufeld@ccs.carleton.ca
 *	grant@acm.org
 *
 *	This source may be freely used as long as the copyright notice is kept in the source.
 *	I ask that you let me know of any enhancements (read: bug fixes) to this code.
 *	I would also like copies of (or discounts on) anything you produce using this code, please.
 *
 *****/

#define __MainSegment__	1

#include <string.h>
#include <Threads.h>

#include "constants.h"
#include "globals.h"

#include "CGI.h"
#include "DebugUtil.h"
#include "ErrorUtil.h"
#include "Events.h"
#include "MemoryUtil.h"
#include "MenuFunc.h"
#include "Quit.h"
#include "Startup.h"


/***  LOCAL VARIABLES  ***/

static	RgnHandle	vTheMouseRgn;
	

/***  LOCAL PROTOTYPES ***/

		void	mainEventLoop		( void );
		void	startupToolbox		( void );
static	Boolean	initAppMemory		( void );
		

/***  FUNCTIONS  ***/

/* Application entry point. */
void
main ( void )
{
	/* Application is not set to quit */
	gQuit = false;
	
	/* Included here because of InitWindows() allocation of non-relocatable blocks.
		This needs to be done before any other code segments are loaded.
		IM:Processes 7-5 */
	/* initialize standard toolbox managers */
	startupToolbox ();
	
	/* Included here because of MoreMasters() allocation of non-relocatable blocks.
		This needs to be done before any other code segments are loaded.
		IM:Processes 7-5 */
	if ( !( initAppMemory () ) )
	{
		/* inform user of insufficient memory */
		ErrorStartup	( kerrStartupMemory );
		ExitToShell		();
	}
	
	StartupApplication	();
	
	/* set up the cgi utilities */
	InitCGIUtil ();
	
	/* start off with an empty region. IM-MTE: 2-24 */
	vTheMouseRgn = NewRgn ();
	
	/* main loop for handling system and user events */
	mainEventLoop ();
	
	DisposeRgn ( vTheMouseRgn );
	
	ExitToShell ();
} /* main */


/* main loop for handling system and user events.
	Continuously loop until application is set to quit. */
void
mainEventLoop ( void )
{
	Boolean			eventResult;
	EventRecord		event;

	/* modeless dialogs */
	Boolean			dialogEvent;
	Boolean			dialogResult;
	DialogPtr		theDialog;
	short			itemHit;
	
	while ( !gQuit )
	{
		/* IM-Memory: 1-48 */
		if ( !IsEmergencyMemAvail() )
		{
			RecoverEmergencyMemory ();
		}
		
		if ( gHasThreadMgr )
		{
			YieldToAnyThread ();
		}
		
		/* wait until the next event */
		eventResult = WaitNextEvent ( everyEvent, &event, gSleepTicks, vTheMouseRgn );
		
		/* adjust the cursor as necessary. IM-MTE: 2-24,25 */
		if ( gFrontProcess && (event.what != kHighLevelEvent) )
		{
			/* MyAdjustCursor ( &event, theMouseRgn ); */
		}
		
		/* determine whether the event is for a modeless dialog */
		dialogEvent = IsDialogEvent ( &event );
		
		if ( eventResult )
		{
			/* if there is an event other than null */
			/* clean up the menus */
			adjustMenus ();
			
			/* figure out which event and process it */
			switch ( event.what ) 
			{
				case kHighLevelEvent:
					doHighLevelEvent ( &event );
					break;
				
				case mouseDown:	
					if ( dialogEvent )
					{
						dialogResult = DialogSelect ( &event, &theDialog, &itemHit );
					}
					else
					{
						doMouseDown ( &event );
					}
					break;
				
				case mouseUp:
					if ( dialogEvent )
					{
						dialogResult = DialogSelect ( &event, &theDialog, &itemHit );
					}
					else
					{
						doMouseUp ( &event );
					}
					break;
				
				case keyDown:
					doKeyDown ( &event );
					break;
				
				case autoKey:
					doAutoKey ( &event );
					break;
				
				/* typically not sent to the application. IM-MTE: 2-28 */
				case keyUp:
					doKeyUp ( &event );
					break;
				
				case activateEvt:
					doActivateEvent ( &event );
					break;
	
				case updateEvt:
					doUpdateEvent ( &event );
					break;
				
				/* suspend or resume */
				case osEvt:
					doOsEvt ( &event );
					break;
				
				/* disk inserted */
				case diskEvt:
					doDiskEvt ( &event );
					break;
				
				default:
					doIdle ( &event );
					break;
			}
		}
		else
		{
			/* not a handled event, or event is null */
			doIdle ( &event );
		}
	}
} /* mainEventLoop */


#pragma mark -
/***  INITIALIZATION  ***/

/* initialize standard toolbox managers */
void
startupToolbox ( void )
{
	InitGraf	( (Ptr)(&qd.thePort) );	/* InitGraf must be first */
	InitFonts	();
	InitWindows	();
	InitMenus	();			/* InitMenus must be after InitWindows */
	FlushEvents	( everyEvent, nil );	/* IM-MTE: 2-93 */
	TEInit		();
	InitDialogs	( nil );	/* InitDialogs must be after TEInit */
	InitCursor	();
} /* startupToolbox */


/* Return true if memory initialization was successful. */
static Boolean
initAppMemory ( void )
{
	short 		mstrPtrsAllocated;
	long 		freeMemAvail;
	Boolean		success;
	
	/* NOTE: MaxApplZone must be called before any memory is allocated by threads
		other than the main (application) thread. */
	/* maximize available memory in the application heap */
	MaxApplZone ();
	
	freeMemAvail = FreeMem ();
	
	if ( freeMemAvail < kMinSegSize )
	{
		/* not enough memory for application to run */
		success = false;
	}
	else
	{
		/* to avoid memory fragmentation, allocate master pointers at bottom of heap */
		for ( mstrPtrsAllocated = 0; mstrPtrsAllocated < kMoreMasterCalls; mstrPtrsAllocated++ )
		{
			MoreMasters ();
		}
		
		success = InitializeEmergencyMemory ( nil );
	}
	
	return success;
} /* initAppMemory */


/***  CUSTOM CGI INTERFACE FUNCTION  ***/
#pragma mark -

/* This function is where the CGI is actually processed.
	You should replace its contents with your own.
	You should set (*theCGIHandle)->responseData to a string (including HTTP header).
	(*theCGIHandle)->responseSize will be automatically deallocated by the CGI handler.
	You should set (*theCGIHandle)->responseSize to be the size of the responseData
	plus one for the null terminator.
	(this function's prototype is defined in CGI.h) */
void
MyCGIProcess ( CGIHdl theCGIHandle )
{
	(*theCGIHandle)->responseSize = gHTTPHeaderOKSize
		+ 141 /* size of my constant string */ + 1 /* null terminator */;
	
	(*theCGIHandle)->responseData = (char *) MyNewPtr ( (*theCGIHandle)->responseSize, nil );
	
	if ( (*theCGIHandle)->responseData != nil )
	{
		HLock ( (Handle)theCGIHandle );
		
		strcpy ( (*theCGIHandle)->responseData, (char *)gHTTPHeaderOK );
		strcpy ( &(((*theCGIHandle)->responseData)[gHTTPHeaderOKSize]),
			"\r<TITLE>Grant's CGI Framework<TITLE>\r<BODY>This framework doesn't do anything. You must add your own code to generate useful results.</BODY>\r" );
		
		HUnlock ( (Handle)theCGIHandle );
	}
	else
	{
		(*theCGIHandle)->responseSize = nil;
	}
	
	/* you should try to make liberal use of yielding to other threads */
	if ( gHasThreadMgr )
	{
		YieldToAnyThread ();
	}
} /* MyCGIProcess */


/*** EOF ***/
