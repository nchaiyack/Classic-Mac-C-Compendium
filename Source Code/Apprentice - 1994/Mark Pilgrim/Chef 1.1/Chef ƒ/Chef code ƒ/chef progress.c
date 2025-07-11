/**********************************************************************\

File:		chef progress.c

Purpose:	This module handles configuring & displaying a progress
			bar, as well as dealing with other people (events) while
			the conversion is in progres.
			

Chef -=- convert text to Swedish chef talk
Copyright �1994, Mark Pilgrim

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
#include "chef progress.h"
#include "msg dialogs.h"
#include "msg environment.h"
#include "msg menus.h"
#include "msg main.h"

enum
{
	progressDialogID = 210,
	progressText = 1,
	progressBar = 2
};

DialogPtr		gProgressDlog;

static	Rect			box;
static	unsigned long	curProgress;
static	unsigned long	maxProgress;

static pascal void DrawProgressBar(WindowPtr theWindow, int item);

static pascal void DrawProgressBar(WindowPtr theWindow, int item)
{
	Rect				tempBox;
	unsigned long		length;
	unsigned long		width;
	long double			temp;
	
	SetPort(theWindow);
	
	FrameRect(&box);
	
	length = box.right - box.left;
	
	width = length * curProgress;
	if((width / length) != curProgress) {
		temp = ((long double)curProgress) / ((long double)maxProgress);
		width = temp * length;
	} else {
		width /= maxProgress;
	}
	
	tempBox = box;
	InsetRect(&tempBox, 1, 1);
	tempBox.left += width;
	FillRect(&tempBox, ltGray);
	
	tempBox = box;
	InsetRect(&tempBox, 1, 1);
	tempBox.right = tempBox.left + width - 1;
	ForeColor(cyanColor);
	PaintRect(&tempBox);
	ForeColor(blackColor);
}

DialogPtr OpenProgressDialog(unsigned long max, Str255 theTitle)
{
	int				itemType;
	Handle			itemH;
	Rect			otherBox;
	
	PositionDialog('DLOG', progressDialogID);
	gProgressDlog = GetNewDialog(progressDialogID, 0L, (WindowPtr)-1L);
	if(gProgressDlog == 0L)
		return 0L;
	
	GetDItem(gProgressDlog, progressBar, &itemType, &itemH, &box);
	SetDItem(gProgressDlog, progressBar, userItem + itemDisable, DrawProgressBar, &box);
	
	curProgress = 0;
	maxProgress = max;
	
	SetWTitle((WindowPtr)gProgressDlog, theTitle);
	
	ShowWindow(gProgressDlog);
	DrawDialog(gProgressDlog);
	
	UpdateProgressDialog(0);
	
	gInProgress=TRUE;
	AdjustMenus();
	DrawMenuBar();
	
	return gProgressDlog;
}

void SetProgressText(Str255 p1, Str255 p2, Str255 p3, Str255 p4)
{
	Str255			totalStr;
	unsigned char	i;
	int				itemType;
	Handle			itemH;
	Rect			otherBox;
	
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
	SetIText((ControlHandle)itemH, totalStr);
}

void UpdateProgressDialog(unsigned long cur)
{
	curProgress = cur;
	if(curProgress >= maxProgress)
		curProgress = maxProgress-1;
	
	SetPort(gProgressDlog);
	
	DrawProgressBar(gProgressDlog, progressBar);
	
	if (gHasPowerManager)
		IdleUpdate();
}

void DismissProgressDialog(void)
{
	if (gProgressDlog!=0L)
		DisposDialog(gProgressDlog);
	gProgressDlog=0L;
	gInProgress=FALSE;
	AdjustMenus();
	DrawMenuBar();
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
	EventRecord		event;
	Boolean			notDoneYet;
	
	SetCursor(&arrow);
	HiliteMenu(0);

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
	
	notDoneYet=TRUE;
	do
	{
		if (WaitNextEvent(everyEvent, &event, gIsInBackground ? 100 : 0, 0L))
			DispatchEvents(event);
		else
			notDoneYet=FALSE;
		if (event.what==nullEvent)
			notDoneYet=FALSE;
	}
	while (notDoneYet);
	
	return !foundEvent;
}
