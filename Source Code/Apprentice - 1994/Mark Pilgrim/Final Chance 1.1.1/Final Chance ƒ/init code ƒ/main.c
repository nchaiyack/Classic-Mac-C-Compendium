/**********************************************************************\

File:		main.c

Purpose:	This module handles the actual shutdown proc -- creating
			a grafport on the screen and showing the dialog box.
			

Final Chance -=- a "do you really want to do this?" dialog on shutdown
Copyright �1993-4, Mark Pilgrim

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

#include "init.h"
#include "main.h"
#include "globals.h"

Str255			gTheQuote;
Handle			gChanceDitl;
DialogTHndl		gChanceDlog;

void FinalChance(void)
{
	DialogTPtr		dtmp;
	DialogRecord	dlog;
	Handle			newDitl, itemH;
	Rect			*theRect, box;
	int				left, top, item, itemType;
	KeyMap			theKeys;
	unsigned long	startTime;
	
	GetKeys(&theKeys);
	if(theKeys[1] & 0x04)	/* option key down? */
		return;
	
	SetUpA4();
	
	InitGraf(&thePort);
	SetCursor(&arrow);
	
	HLock(gChanceDlog);
	newDitl=gChanceDitl;
	HandToHand(&newDitl);

	theRect = (Rect*) *gChanceDlog;
	left = (screenBits.bounds.right - (theRect->right - theRect->left)) / 2;
	top = (screenBits.bounds.bottom - (theRect->bottom - theRect->top)) / 3;
	
	if(top < (GetMBarHeight() + 1))
		top = GetMBarHeight() + 1;
	theRect->right += left - theRect->left;
	theRect->left = left;
	theRect->bottom += top - theRect->top;
	theRect->top = top;

	dtmp=*gChanceDlog;
	NewDialog(&dlog, theRect, dtmp->title, dtmp->visible, dtmp->procID,
				(WindowPtr)-1L, dtmp->goAwayFlag, dtmp->refCon, newDitl);
	
	ParamText(gTheQuote, "\p", "\p", "\p");
	
	GetDItem(&dlog, 6, &itemType, &itemH, &box);
	InsetRect(&box, -4, -4);
	SetDItem(&dlog, 6, itemType, (Handle)OutlineDefaultButton, &box);
	
	ShowWindow(&dlog);
	
	GetDateTime(&startTime);
	GetDItem(&dlog, 8, &itemType, &itemH, &box);
	/* DrawModuleIcon needs access to the start time, but has no access to our globals, */
	/* so we'll store the time in the refcon of a control for lack of anything cleaner */
	SetCRefCon((ControlHandle)itemH, startTime);

	item=0;
	do { ModalDialog((ProcPtr)ProcOFilter, &item); } while ((item!=1) && (item!=2));
	
	HideWindow(&dlog);
	CloseDialog(&dlog);
	
	HUnlock(gChanceDlog);
	DisposeHandle(newDitl);
	
	RestoreA4();
	
	if (item==2)
		ExitToShell();
}

pascal Boolean ProcOFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem)
{
	unsigned char	theChar;
	unsigned long	theTime, startTime;
	int				itemType;
	Rect			box;
	Handle			itemH;
	
	switch (theEvent->what)
	{
		case keyDown:
		case autoKey:
			theChar=theEvent->message & charCodeMask;
			if ((theChar==0x0d) || (theChar==0x03))
			{
				*theItem=1;
				return TRUE;
			}
			if ((theChar==0x1b) ||
				((theEvent->modifiers & cmdKey) && (theChar=='.')))
			{
				*theItem=2;
				return TRUE;
			}
			break;
		case nullEvent:
			GetDateTime(&theTime);
			GetDItem(theDialog, 8, &itemType, &itemH, &box);
			startTime=(unsigned long)GetCRefCon((ControlHandle)itemH);
			if (theTime-startTime>=60)
			{
				*theItem=1;
				return TRUE;
			}
			break;
	}
	
	return FALSE;
}

pascal void OutlineDefaultButton(DialogPtr myDlog, short itemNum)
{
	short	itemType;
	Handle	itemH;
	Rect	box;
	
	GetDItem(myDlog, 1, &itemType, &itemH, &box);
	PenSize(3, 3);
	InsetRect(&box, -4, -4);
	FrameRoundRect(&box, 16, 16);
	PenNormal();
}
