#include "error.h"
#include "main.h"
#include "dialog layer.h"
#include "environment.h"
#include "window layer.h"

ErrorStateRec		gPendingErrorRec;

void FailNilUPP(UniversalProcPtr theUPP)
{
	if (theUPP == nil)
		HandleError(kNoMemory, FALSE, FALSE);
}

void HandleError(enum ErrorTypes resultCode, Boolean isFatal, Boolean isSmall)
/* if we're in the foreground, just get the error string (from the .rsrc file) and
   display the error alert; otherwise, we have to queue it, put up a notification
   (if possible), and wait patiently to come back in the foreground.  (see main.c,
   DispatchEvents(), case osEvt) */
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
			gPendingErrorRec.notificationRec.qType=nmType;			/* for more detail on these params, */
			gPendingErrorRec.notificationRec.nmMark=1;				/* see IM Processes, 5-8 */
			gPendingErrorRec.notificationRec.nmIcon=myResHand;
			gPendingErrorRec.notificationRec.nmSound=(Handle)-1L;
			gPendingErrorRec.notificationRec.nmStr=0L;
			gPendingErrorRec.notificationRec.nmResp=0L;
			gPendingErrorRec.notificationRec.nmRefCon=0L;
			NMInstall(&gPendingErrorRec.notificationRec);
		}
		gPendingErrorRec.resultCode=resultCode;				/* remember error code for later */
	}
	else
	{
		GetIndString(tempStr, 129, resultCode);	/* get error string from .rsrc */
		DisplayTheAlert(kStopAlert, isSmall ? kSmallAlertID : kLargeAlertID, tempStr, "\p", "\p", "\p",
			0L);
		
		if (isFatal)		/* for fatal errors */
		{
			ShutDownEnvironment(FALSE);
			ExitToShell();
		}
	}
}
