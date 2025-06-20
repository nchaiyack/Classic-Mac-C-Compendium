/**********************************************************************\

File:		progress.c

Purpose:	This module handles the progress bar and dealing with
			events while the progress bar is up.
			
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

#include "Power.h"
#include "program globals.h"
#include "progress.h"
#include "dialogs.h"
#include "environment.h"
#include "menus.h"
#include "graphics.h"
#include "main.h"

enum
{
	progressDialogID = 210,
	progressText = 1,
	progressBar = 2
};

DialogPtr		gProgressDlog=0L;		/* pointer to progress dialog */
short			gOldForegroundWaitTime;

/*-----------------------------------------------------------------------------------*/
/* internal stuff for progress.c                                                     */

static	Rect			box;				/* box of actual progress bar in dialog */
static	unsigned long	curProgress;		/* current progress value */
static	unsigned long	maxProgress;		/* maximum progress value */

static pascal void DrawProgressBar(WindowPtr theWindow, short item);


static pascal void DrawProgressBar(WindowPtr theWindow, short item)
/* the useritem procedure for the actual progress bar in the dialog */
{
	Rect				tempBox;
	unsigned long		length;
	unsigned long		width;
	long double			temp;
	
	SetPort(theWindow);		/* the dialog */
	FrameRect(&box);		/* progress area outline */
	
	length = box.right - box.left;
	width = length * curProgress;
	if ((width / length) != curProgress)	/* if we have overflow problems... */
	{										/* use long double math instead */
		temp = ((long double)curProgress) / ((long double)maxProgress);
		width = temp * length;
	}
	else width /= maxProgress;
	
	tempBox = box;
	InsetRect(&tempBox, 1, 1);
	tempBox.left += width;
	FillRect(&tempBox, ltGray);		/* gray background in progress area */
	
	tempBox = box;
	InsetRect(&tempBox, 1, 1);
	tempBox.right = tempBox.left + width - 1;
	ForeColor(cyanColor);
	PaintRect(&tempBox);			/* paint progress area as much as we've progressed */
	ForeColor(blackColor);			/* important!  always set ForeColor back to black */
}

DialogPtr OpenProgressDialog(unsigned long max, Str255 theTitle)
{
	short			itemType;
	Handle			itemH;
	Rect			otherBox;
	
	PositionDialog('DLOG', progressDialogID);	/* see dialogs.c */
	/* get the progress dialog from .rsrc file */
	
	gProgressDlog = GetNewDialog(progressDialogID, 0L, (WindowPtr)-1L);
	if (gProgressDlog == 0L)
		return 0L;
	
	/* set up useritem procedure to draw the progress area (see above) */
	GetDItem(gProgressDlog, progressBar, &itemType, &itemH, &box);
	SetDItem(gProgressDlog, progressBar, userItem + itemDisable, (ProcPtr)DrawProgressBar, &box);
	
	curProgress = 0;		/* start at empty */
	maxProgress = max;		/* max value as passed in parameter */
	
	SetWTitle((WindowPtr)gProgressDlog, theTitle);	/* set title as passed in parameter */
	
	ShowWindow(gProgressDlog);	/* show it */
	DrawDialog(gProgressDlog);	/* draw it */
	
	UpdateProgressDialog(0);	/* draw progress area as empty (zero progress) */
	
	gOldForegroundWaitTime=gForegroundWaitTime;
	gForegroundWaitTime=0;
	
	gInProgress=TRUE;			/* so we know progress bar is up */
	AdjustMenus();				/* dims almost everything */
	DrawMenuBar();				/* needed so menus look dimmed immediately */
	
	return gProgressDlog;
}

void SetProgressText(Str255 p1, Str255 p2, Str255 p3, Str255 p4)
{
	Str255			totalStr;
	unsigned char	i;
	short			itemType;
	Handle			itemH;
	Rect			otherBox;
	
	/* DON'T use ParamText to set text in progress dialog.  ParamText handles are
	   low-mem globals and can be changed if you switch out of the application and
	   another program displays an alert/dialog with ParamText strings.  Instead,
	   add up all four strings into one big string (max 255 characters total) and
	   sets the dialog item to be that text. */
	totalStr[0]=0x00;
	for (i=1; i<=p1[0]; i++)
		totalStr[++totalStr[0]]=p1[i];
	for (i=1; i<=p2[0]; i++)
		totalStr[++totalStr[0]]=p2[i];
	for (i=1; i<=p3[0]; i++)
		totalStr[++totalStr[0]]=p3[i];
	for (i=1; i<=p4[0]; i++)
		totalStr[++totalStr[0]]=p4[i];
	GetDItem(gProgressDlog, 1, &itemType, &itemH, &otherBox);
	SetIText(itemH, totalStr);
}

void UpdateProgressDialog(unsigned long cur)
{
	curProgress = cur;		/* set our global variable of current progress */
	if (curProgress >= maxProgress)		/* can't be >= than max progress */
		curProgress = maxProgress-1;
	
	SetPort(gProgressDlog);
	
	DrawProgressBar(gProgressDlog, progressBar);	/* draw progress area manually */
	
	if (gHasPowerManager)	/* so Powerbooks won't go down to 1 MHz during a */
		IdleUpdate();		/* lengthy progress operation */
}

void DismissProgressDialog(void)
{
	if (gProgressDlog!=0L)		/* so you can be sloppy and dismiss it even if it's */
		DisposDialog(gProgressDlog);	/* not up (: */
	gProgressDlog=0L;			/* so we know it's gone */
	gInProgress=FALSE;			/* not in progress anymore */
	gForegroundWaitTime=gOldForegroundWaitTime;
	AdjustMenus();				/* so adjust menus accordingly */
	DrawMenuBar();				/* and redraw menubar to see effect immediately */
}

#define TheCancelKey	'.'

Boolean DealWithOtherPeople(void)
{
	/* this is just a small useful function to see if the user has cancelled */
	/* a lengthy operation with command-period; could come in handy, I suppose, */
	/* in a somewhat bizarre set of circumstances... */
	/* Note that this procedure will break under AUX */
	/* Note also that this returns TRUE if there has been no attempt to cancel */
	
	Boolean			foundEvent;
	EvQElPtr		eventQPtr;
	QHdrPtr			eventQHdr;
	char			thisChar;
	long			isCmdKey;
	
	foundEvent=FALSE;
	eventQHdr=GetEvQHdr();
	eventQPtr=(EvQElPtr)(eventQHdr->qHead);
	while ((eventQPtr!=0L) && (!foundEvent))
	{
		if (eventQPtr->evtQWhat==keyDown)
		{
			thisChar=(char)((eventQPtr->evtQMessage)&charCodeMask);
			isCmdKey=(eventQPtr->evtQModifiers)&cmdKey;
			if (isCmdKey!=0L)
				foundEvent=(thisChar==TheCancelKey);
		}
		if (!foundEvent)
			eventQPtr=(EvQElPtr)(eventQPtr->qLink);
	}

	while (HandleSingleEvent());
	
	return !foundEvent;
}
