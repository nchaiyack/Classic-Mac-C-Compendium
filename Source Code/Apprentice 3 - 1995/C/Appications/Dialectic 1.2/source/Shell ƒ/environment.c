/**********************************************************************\

File:		environment.c

Purpose:	This module handles initializing the environment and
			checking for various environmental characteristics.

\**********************************************************************/

#include "environment.h"
#include "apple events.h"
#include "error.h"
#include "program globals.h"
#include "GestaltEqu.h"
#include "Traps.h"

Boolean			gHasColorQD;				/* color Quickdraw present? */
Boolean			gHasAppleEvents;			/* apple events supported? */
Boolean			gHasFSSpecs;				/* FSSpec calls supported? */
Boolean			gStandardFile58;			/* standard file package supported? */
Boolean			gHasPowerManager;			/* power manager present? (cf progress.c) */
Boolean			gHasNotificationManager;	/* notification manager present? (cf error.c) */
Boolean			gSystemSevenOrLater;		/* system 7 running? */
Boolean			gWaitNextEventAvailable;	/* WaitNextEvent() implemented? */
Boolean			gHasGestalt;				/* Gestalt() implemented? */

int				gForegroundWaitTime;		/* WaitNextEvent sleep time while in frgrnd */
int				gBackgroundWaitTime;		/* WaitNextEvent sleep time while in bkgrnd */
Boolean			gIsInBackground;			/* program is in background currently? */
Boolean			gInProgress;				/* progress bar up? */
Boolean			gDone;						/* program done? */
Boolean			gFrontWindowIsOurs;			/* frontmost window is one of ours? */

/* This is to stop the compiler from using Gestalt glue without killing
   all the other pre-system 7 glue for other routines. */
#pragma parameter __D0 RealGestalt(__D0,__A1)
pascal OSErr RealGestalt(OSType selector,long *response) = {0xA1AD,0x2288};
#define		Gestalt			RealGestalt
#define		GESTALT_TRAP	0xA1AD			/* _Gestalt */

#define	GetTrapType(T)	(((T & 0x0800) > 0) ? ToolTrap : OSTrap)
#define NumToolboxTraps()	((NGetTrapAddress(_InitGraf, ToolTrap) ==	\
							NGetTrapAddress(0xAA6E, ToolTrap)) ? 0x0200	\
							: 0x0400)

Boolean TrapAvailable(int theTrap);

Boolean TrapAvailable(int theTrap)
{
	TrapType		tType;
	
	tType=GetTrapType(theTrap);
	if (tType==ToolTrap)
	{
		theTrap=theTrap&0x07FF;
		if (theTrap>=NumToolboxTraps())
			theTrap=_Unimplemented;
	}
	return (NGetTrapAddress(theTrap, tType)!=NGetTrapAddress(_Unimplemented, ToolTrap));
}

Boolean InitTheEnvironment(void)
/* called very early -- this uses Gestalt to check out the computing environment;
   see above for explanations of variables */
{
	long			gestalt_temp;
	OSErr			isHuman;
	SysEnvRec		theWorld;
	
	if (SysEnvirons(1, &theWorld)==envNotPresent)
		return FALSE;
	
	gWaitNextEventAvailable=TrapAvailable(_WaitNextEvent);
	gHasGestalt=TrapAvailable((int)GESTALT_TRAP);

	if (gHasGestalt)
	{
		isHuman=Gestalt(gestaltSystemVersion, &gestalt_temp);
		gSystemSevenOrLater=!(isHuman || (gestalt_temp < 1792));
		
		isHuman=Gestalt(gestaltQuickdrawVersion, &gestalt_temp);
		gHasColorQD=!(isHuman || (gestalt_temp < gestalt8BitQD));
	
		isHuman=Gestalt(gestaltFSAttr, &gestalt_temp);
		gHasFSSpecs=((isHuman==noErr) && (gestalt_temp&(1<<gestaltHasFSSpecCalls)));
	
		isHuman=Gestalt(gestaltStandardFileAttr, &gestalt_temp);
		gStandardFile58=((isHuman==noErr) && (gestalt_temp&(1<<gestaltStandardFile58)));
	
		isHuman=Gestalt(gestaltPowerMgrAttr, &gestalt_temp);
		gHasPowerManager=((!isHuman) && (gestalt_temp&(1<<gestaltPMgrCPUIdle)));
		
		isHuman=Gestalt(gestaltNotificationMgrAttr, &gestalt_temp);
		gHasNotificationManager=(!((isHuman) ||
			(((gestalt_temp >> gestaltNotificationPresent) & 0x01) != 1)));
		
		isHuman=Gestalt(gestaltAppleEventsAttr, &gestalt_temp);
		gHasAppleEvents=((!isHuman) && (gestalt_temp&(1<<gestaltAppleEventsPresent)));
	}
	else
	{
		gHasColorQD=theWorld.hasColorQD;
		gSystemSevenOrLater=(theWorld.systemVersion>=1792);
		gHasFSSpecs=TrapAvailable(_HFSPinaforeDispatch);
		gStandardFile58=gSystemSevenOrLater;
		gHasPowerManager=TrapAvailable(0xA285);	/* _IdleUpdate */
		gHasNotificationManager=TrapAvailable(_NMInstall);
		gHasAppleEvents=TrapAvailable(_Pack8);
	}
	
	if (gHasAppleEvents)
		SetUpAppleEvents();		/* see apple events.c */
	
	gForegroundWaitTime=30;			/* WaitNextEvent sleep time when we're in frgrnd */
	gBackgroundWaitTime=100;		/* WaitNextEvent sleep time when we're in bkgrnd */
	gDone=FALSE;					/* TRUE if program is done (exit event loop) */
	gInProgress=FALSE;				/* TRUE if progress bar is up */
	gIsInBackground=FALSE;			/* TRUE if program is in background */
	gPendingResultCode=allsWell;	/* set to error code if error occurs while in bkgrnd */
	
	return TRUE;
}
