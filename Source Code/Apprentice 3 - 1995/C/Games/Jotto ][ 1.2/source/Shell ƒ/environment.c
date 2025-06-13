#include "environment.h"
#include "apple events.h"
#include "error.h"
#include "program globals.h"
#include "GestaltEqu.h"
#include "Traps.h"

Boolean			gHasColorQD;				/* color Quickdraw present? */
Boolean			gHasNotificationManager;	/* notification manager present? (cf error.c) */

short			gForegroundWaitTime;		/* WaitNextEvent sleep time while in frgrnd */
short			gBackgroundWaitTime;		/* WaitNextEvent sleep time while in bkgrnd */
Boolean			gIsInBackground;			/* program is in background currently? */
Boolean			gInProgress;				/* progress bar up? */
Boolean			gDone;						/* program done? */
Boolean			gFrontWindowIsOurs;			/* frontmost window is one of ours? */
short			gFrontWindowIndex;			/* gTheWindowData[] index of front window, if ours */
Boolean			gIsVirgin;					/* first time running? */
Boolean			gCustomCursor;				/* front window is using custom cursor ? */
short			gKludgeIter;
Boolean			gNeedToOpenWindow;

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

Boolean TrapAvailable(short theTrap);

Boolean TrapAvailable(short theTrap)
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
	
	if (!TrapAvailable(_WaitNextEvent))
		return FALSE;
	
	if (!TrapAvailable((short)GESTALT_TRAP))
		return FALSE;

	isHuman=Gestalt(gestaltSystemVersion, &gestalt_temp);
	if (isHuman || (gestalt_temp < 1792))
		return FALSE;
	
	isHuman=Gestalt(gestaltQuickdrawVersion, &gestalt_temp);
	gHasColorQD=!(isHuman || (gestalt_temp < gestalt8BitQD));

	isHuman=Gestalt(gestaltFSAttr, &gestalt_temp);
	if (!((isHuman==noErr) && (gestalt_temp&(1<<gestaltHasFSSpecCalls))))
		return FALSE;

	isHuman=Gestalt(gestaltStandardFileAttr, &gestalt_temp);
	if (!((isHuman==noErr) && (gestalt_temp&(1<<gestaltStandardFile58))))
		return FALSE;

	isHuman=Gestalt(gestaltNotificationMgrAttr, &gestalt_temp);
	gHasNotificationManager=(!((isHuman) ||
		(((gestalt_temp >> gestaltNotificationPresent) & 0x01) != 1)));
	
	isHuman=Gestalt(gestaltAppleEventsAttr, &gestalt_temp);
	if (!((!isHuman) && (gestalt_temp&(1<<gestaltAppleEventsPresent))))
		return FALSE;
	
	InstallRequiredAppleEvents();
	
	gForegroundWaitTime=10;			/* WaitNextEvent sleep time when we're in frgrnd */
	gBackgroundWaitTime=100;		/* WaitNextEvent sleep time when we're in bkgrnd */
	gDone=FALSE;					/* TRUE if program is done (exit event loop) */
	gInProgress=FALSE;				/* TRUE if progress bar is up */
	gIsInBackground=FALSE;			/* TRUE if program is in background */
	gPendingResultCode=allsWell;	/* set to error code if error occurs while in bkgrnd */
	gFrontWindowIsOurs=FALSE;
	gFrontWindowIndex=0;
	gCustomCursor=FALSE;
	gKludgeIter=0;
	gNeedToOpenWindow=TRUE;

	return TRUE;
}
