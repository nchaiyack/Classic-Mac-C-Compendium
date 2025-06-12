/*
** aevent.c
**
** This module contains all the code for the program to handle the
** required Apple Events.  Of the four types, only two really
** pertain to the program (open and quit) with quit
** being the only used.
*/

#ifndef THINK_C
 /* #include all the macintosh headers... */
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

static pascal OSErr QuitHandler(AppleEvent*, AppleEvent*, long);
static pascal OSErr OappHandler(AppleEvent*, AppleEvent*, long);
static pascal OSErr OdocHandler(AppleEvent*, AppleEvent*, long);
static pascal OSErr PrintHandler(AppleEvent*, AppleEvent*, long);


void InitAEs(void)
{
	OSErr err;
	
	/* install event handlers for the core apple events. */
	err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication,
				QuitHandler, 0, FALSE);
	err = AEInstallEventHandler( kCoreEventClass, kAEOpenApplication,
				OappHandler, 0, FALSE);
	err = AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments,
				OdocHandler, 0, FALSE);
	err = AEInstallEventHandler( kCoreEventClass, kAEPrintDocuments,
				PrintHandler, 0, FALSE);
} /* InitAEs() */


void DoAppleEvent(EventRecord *theEvent)
{
	OSErr err;
	err = AEProcessAppleEvent(theEvent);
} /* DoAppleEvent() */


/*===================================  HANDLERS FOLLOW... */


static pascal OSErr QuitHandler(AppleEvent *theAppleEvent,
				AppleEvent *reply, long handlerRefcon)
{
	Cleanup();
} /* QuitHandler() */


static pascal OSErr OappHandler(AppleEvent *theAppleEvent,
				AppleEvent *reply, long handlerRefcon)
{
}

static pascal OSErr OdocHandler(AppleEvent *theAppleEvent,
				AppleEvent *reply, long handlerRefcon)
{
}

static pascal OSErr PrintHandler(AppleEvent *theAppleEvent,
				AppleEvent *reply, long handlerRefcon)
{
}

