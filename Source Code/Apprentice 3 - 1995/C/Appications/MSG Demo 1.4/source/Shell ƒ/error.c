/**********************************************************************\

File:		error.c

Purpose:	This module handles altering the user when an error has
			occurred.  (If the program is currently in the background,
			we use the Notification Manager to queue a notification
			request and display the alert as soon as we are in the
			foreground -- see main.c, DispatchEvents, case osEvt.)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

#include "error.h"
#include "main.h"
#include "dialogs.h"
#include "environment.h"
#include "program globals.h"

NMRec			gMyNotification;
int				gPendingResultCode;

void HandleError(int resultCode, Boolean exitToShell)
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
		GetIndString(tempStr, 129, -resultCode);	/* get error string from .rsrc */
		ParamText(tempStr, "\p", "\p", "\p");
		PositionDialog('ALRT', largeAlert);		/* position alert (see dialogs.c) */
		StopAlert(largeAlert, 0L);				/* show it */
	}
	
	if (exitToShell)		/* for fatal errors */
		ExitToShell();
}
