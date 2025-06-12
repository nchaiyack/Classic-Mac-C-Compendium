/*
>>	Dizzy 1.0 Menu.c
>>
>>	A digital circuit simulator & design program for the X Window System
>>
>>	Copyright 1990 Juri Munkki, all rights reserved
*/

#include "dizzy.h"

/*
>>	DoMenuClick implements menu handling with just a few
>>	graphics routines and a routine that uses events to
>>	get the mouse state. It should be easy to port this
>>	code to just about any platform.
>>
>>	Note that the user always has the choice of cancelling
>>	an action before releasing the mouse button by moving
>>	the mouse away from the menu area.
>>
>>	The same routine was used as a basis for the more
>>	complicated tool selection routine.
*/
void	DoMenuClick()
{
	Rect		HiliteRect,OldHiliteRect,Inside;
	int 		HiliteItem,NewHilite,i,downflag;
	Point		MousePoint;
	
#ifdef	MACINTOSH
	SetPort(MyWind);
#endif
	if(SplashVisible)
	{	SplashVisible=0;
		InvalTrash();
	}

	Inside.left=MenuR.left+1;
	Inside.right=MenuR.right-3;
	Inside.top=MenuR.top+1;
	Inside.bottom=MenuR.bottom-3;

	HiliteItem=0;
	HiliteRect=Inside;
	
	do	/*	Track mouse while button is down.				*/
	{	downflag=GetMouseTrackEvent(&MousePoint);
		if(PtInRect(MousePoint,&Inside))
		{	/*	Find selected menu that mouse points to.	*/
			MousePoint.v-=Inside.top;
			NewHilite=MousePoint.v/24;
			HiliteRect.top=Inside.top+24*NewHilite;
			HiliteRect.bottom=HiliteRect.top+24;
			NewHilite++;
		}
		else
		{	NewHilite=0;	/*	No selected item.			*/
		}
		if(HiliteItem!=NewHilite)
		{	if(HiliteItem)	/*	Unhilite old item.			*/
			{	HILITEMODE
				InvertRect(&OldHiliteRect);
			}
			if(NewHilite)	/*	Hilite new item.			*/
			{	HILITEMODE
				InvertRect(&HiliteRect);
			}				/*	Old item = new item.		*/
			HiliteItem=NewHilite;
			OldHiliteRect=HiliteRect;
		}
	}	while(downflag);	/*	Do until button comes up.	*/

	if(HiliteItem)
	{	for(i=MenuFlash*2;i;i--)	/*	Flash menu item.	*/
		{	long	dummy;
		
			HILITEMODE
			InvertRect(&OldHiliteRect);
			Delay(2,&dummy);
		}
		
		MenuHilite=OldHiliteRect;
	}
	
	switch(HiliteItem)			/*	Handle menu commands.	*/
	{	case 1: /*	Quit									*/
			QuitNow= -1;
			break;
		case 2: /*	Open…									*/
			PromptOpenFile();
			break;
		case 3: /*	Save									*/
			SaveFile();
			break;
		case 4: /*	Save as...								*/
			SaveFileAs();
			break;
		case 5: /*	Include a custom chip					*/
			AddCustomChip();
			break;
	}

	if(HiliteItem)			/*	Unhilite command, if any.	*/
	{	HILITEMODE;
		InvertRect(&OldHiliteRect);
	}
	MenuHilite=NilRect;
}
