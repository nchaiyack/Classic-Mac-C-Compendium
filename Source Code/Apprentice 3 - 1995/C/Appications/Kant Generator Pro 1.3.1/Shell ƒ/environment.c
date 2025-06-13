#include "environment.h"
#include "ae layer.h"
#include "error.h"
#include "program globals.h"

Boolean			gHasColorQD;				/* color Quickdraw present? */
Boolean			gHasNotificationManager;	/* notification manager present? (cf error.c) */

short			gForegroundWaitTime;		/* WaitNextEvent sleep time while in frgrnd */
short			gBackgroundWaitTime;		/* WaitNextEvent sleep time while in bkgrnd */
Boolean			gIsInBackground;			/* program is in background currently? */
Boolean			gInProgress;				/* progress bar up? */
Boolean			gDone;						/* program done? */
short			gFrontWindowIndex;			/* gTheWindowData[] index of front window, if ours */
Boolean			gIsVirgin;					/* first time running? */
Boolean			gCustomCursor;				/* front window is using custom cursor ? */
short			gKludgeIter;
Boolean			gNeedToOpenWindow;

#define		GESTALT_TRAP	0xA1AD			/* _Gestalt */

#define	GetTrapType(T)	(((T & 0x0800) > 0) ? ToolTrap : OSTrap)
#define NumToolboxTraps()	((NGetTrapAddress(_InitGraf, ToolTrap) ==	\
							NGetTrapAddress(0xAA6E, ToolTrap)) ? 0x0200	\
							: 0x0400)

Boolean MyTrapAvailable(short theTrap);

Boolean MyTrapAvailable(short theTrap)
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

ErrorTypes InitTheEnvironment(void)
/* called very early -- this uses Gestalt to check out the computing environment;
   see above for explanations of variables */
{
	long			gestalt_temp;
	OSErr			isHuman;
	SysEnvRec		theWorld;
	
	if (SysEnvirons(1, &theWorld)==envNotPresent)
		return kSystemTooOld;
	
	if (!MyTrapAvailable(_WaitNextEvent))
		return kSystemTooOld;
	
	if (!MyTrapAvailable((short)GESTALT_TRAP))
		return kSystemTooOld;

	isHuman=Gestalt(gestaltSystemVersion, &gestalt_temp);
	if (isHuman || (gestalt_temp < 1792))
		return kSystemTooOld;
	
	isHuman=Gestalt(gestaltQuickdrawVersion, &gestalt_temp);
	gHasColorQD=!(isHuman || (gestalt_temp < gestalt8BitQD));

	isHuman=Gestalt(gestaltFSAttr, &gestalt_temp);
	if (!((isHuman==noErr) && (gestalt_temp&(1<<gestaltHasFSSpecCalls))))
		return kSystemTooOld;

	isHuman=Gestalt(gestaltStandardFileAttr, &gestalt_temp);
	if (!((isHuman==noErr) && (gestalt_temp&(1<<gestaltStandardFile58))))
		return kSystemTooOld;

	isHuman=Gestalt(gestaltNotificationMgrAttr, &gestalt_temp);
	gHasNotificationManager=(!((isHuman) ||
		(((gestalt_temp >> gestaltNotificationPresent) & 0x01) != 1)));
	
	isHuman=Gestalt(gestaltAppleEventsAttr, &gestalt_temp);
	if (!((!isHuman) && (gestalt_temp&(1<<gestaltAppleEventsPresent))))
		return kSystemTooOld;
	
	InstallRequiredAppleEvents();
	
	gForegroundWaitTime=0;			/* WaitNextEvent sleep time when we're in frgrnd */
	gBackgroundWaitTime=100;		/* WaitNextEvent sleep time when we're in bkgrnd */
	gDone=FALSE;					/* TRUE if program is done (exit event loop) */
	gInProgress=FALSE;				/* TRUE if progress bar is up */
	gIsInBackground=FALSE;			/* TRUE if program is in background */
	gPendingErrorRec.resultCode=allsWell;
	gFrontWindowIndex=0;
	gCustomCursor=FALSE;
	gKludgeIter=0;
	gNeedToOpenWindow=TRUE;

	return allsWell;
}
