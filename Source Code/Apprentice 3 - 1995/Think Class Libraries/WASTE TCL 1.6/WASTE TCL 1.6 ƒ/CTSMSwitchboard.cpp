/******************************************************************************
 CTSMSwitchboard.cpp

 ******************************************************************************/

#include "CTSMSwitchboard.h"
#include "Global.h"
#include "TBUtilities.h"
#include "CApplication.h"
#include "CDesktop.h"
#include "CBartender.h"
#include "CWindow.h"
#include "Commands.h"
#include "Constants.h"
#include "CAppleEvent.h"
#ifndef __APPLEEVENTS__
#include <AppleEvents.h>
#endif
#ifndef __AEOBJECTS__
#include <AEObjects.h>
#endif
#ifndef __AEREGISTRY__
#include "AERegistry.h"
#endif
#ifndef __DISKINIT__
#include <DiskInit.h>
#endif
#ifndef __DESK__
#include <Desk.h>
#endif
#include <TextServices.h>

#include "CAppleEventObject.h"
//#include "EPPC.h"
#include "WASTE.h"

/*** Global Variables ***/
	
extern CApplication	*gApplication;		/* Application object				*/
extern CDesktop		*gDesktop;			/* The visible Desktop				*/
extern CBartender	*gBartender;		/* Manages all menus				*/
extern CBureaucrat	*gGopher;			/* First in line to get commands	*/
extern EventRecord	gLastMouseDown;		/* Previous mouse down event		*/
extern EventRecord	gLastMouseUp;		/* Previous mouse up event			*/
extern CView		*gLastViewHit;		/* Last view clicked in				*/
extern long			gSleepTime;			/* Max time between events			*/
										// TCL 1.1.3 11/30/92 BF
extern Boolean		gInBackground;		/* In background under MultiFinder	*/
extern short 		gUsingTSM;			/* Using Text Service Manager       */

/*** Class Constants ***/

#define		MouseMovedEvt		0xFA	/* Mouse moved event code			*/
#define		SuspendResumeEvt	0x01	/* Suspend/Resume event code		*/
#define		ResumeEvtMask		0x1		/* Supend or Resume selector		*/
#define		ConvertScrapMask	0x2		/* Scrap conversion flag			*/

CTSMSwitchboard::CTSMSwitchboard()
	: CSwitchboard()
{
}

void CTSMSwitchboard::ITSMSwitchboard(void)
{
}

void	CTSMSwitchboard::ProcessEvent( void)
{
	EventRecord			macEvent;		/* Mac Event to be processed		*/
	Boolean				isMyEvent;		/* Should program handle event?		*/
	Point				mouseLoc;		/* Mouse location in global coords	*/
	Boolean				handledByTSM=false;
	Boolean				tsmSetCursor = false;
        
	gDesktop->Prepare();
    GetMouse(&mouseLoc);

    if (gUsingTSM)
    {
		tsmSetCursor = SetTSMCursor(mouseLoc);
	}
	if (!tsmSetCursor) gDesktop->DispatchCursor(mouseLoc, mouseRgn);

										/* Get an event from the queue		*/
	isMyEvent = GetAnEvent( &macEvent);
	
	if (gUsingTSM)
	{
		handledByTSM = TSMEvent(&macEvent);
	}
	
	if (isMyEvent && !handledByTSM)			/* We must respond to this event	*/
	{			
		DispatchEvent( &macEvent);		
	} 
	else 
	{									/* No action required				*/
		DoIdle( &macEvent);
	}
}

