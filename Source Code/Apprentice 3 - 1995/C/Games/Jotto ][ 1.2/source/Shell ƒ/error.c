#include "error.h"
#include "main.h"
#include "dialogs.h"
#include "environment.h"
#include "window layer.h"

NMRec			gMyNotification;
enum ErrorTypes	gPendingResultCode;

void FailNilUPP(UniversalProcPtr theUPP)
{
	if (theUPP == nil)
		HandleError(kNoMemory, FALSE);
}

void HandleError(enum ErrorTypes resultCode, Boolean exitToShell)
/* if we're in the foreground, just get the error string (from the .rsrc file) and
   display the error alert; otherwise, we have to queue it, put up a notification
   (if possible), and wait patiently to come back in the foreground.  (see main.c,
   DispatchEvents(), case osEvt. */
/* All error codes are listed in program globals.h */
{
	Str255			tempStr;
	Handle			myResHand;
	
	/* if there is no error, or the error is that the user cancelled an operation
	   in progress, don't display anything; it would only confuse them. */
	if ((resultCode==userCancelErr) || (resultCode==allsWell)) return;
	
	if (gIsInBackground)	/* if program is in background, can't display alert immed. */
	{
		if (gHasNotificationManager)	/* if they don't have notification, f*ck 'em */
		{
			myResHand=GetResource('SICN', 1234);	/* small icon for menu bar flashing */
			gMyNotification.qType=nmType;			/* for more detail on these params, */
			gMyNotification.nmMark=1;				/* see IM Processes, 5-8 */
			gMyNotification.nmIcon=myResHand;
			gMyNotification.nmSound=(Handle)-1L;
			gMyNotification.nmStr=0L;
			gMyNotification.nmResp=0L;
			gMyNotification.nmRefCon=0L;
			NMInstall(&gMyNotification);
		}
		gPendingResultCode=resultCode;				/* remember error code for later */
	}
	else
	{
		RemoveHilitePatch();
		GetIndString(tempStr, 129, resultCode);	/* get error string from .rsrc */
		ParamText(tempStr, "\p", "\p", "\p");
		PositionDialog('ALRT', largeAlert);		/* position alert (see dialogs.c) */
		StopAlert(largeAlert, 0L);				/* show it */
		InstallHilitePatch();
	}
	
	if (exitToShell)		/* for fatal errors */
	{
		ShutDownEnvironment(FALSE);
		ExitToShell();
	}
}
