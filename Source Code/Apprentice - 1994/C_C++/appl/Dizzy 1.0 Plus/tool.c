/*
>>	Dizzy 1.0 Menu.c
>>
>>	A digital circuit simulator & design program for the X Window System
>>
>>	Copyright 1990 Juri Munkki, all rights reserved
>>
>>	Please read the included file called "DizzyDoc" for information on
>>	what your rights are concerning this product.
*/

#include "dizzy.h"

/*
>>	Mouse button functions are marked by what I call "tags" in the
>>	tool palette. This routine draws those tags into the palette.
*/
void	InvertButtonTag(index)
int 	index;
{
				Rect			src,dest;
	register	ToolSelection	*sel;

	if(index>=0 && index<MOUSE_BUTTONS)
	{	sel= &ToolButtons[index];
		if(sel->Function)
		{	src.left=index*7;
			src.right=src.left+7;
			src.top=0;
			src.bottom=6;
			
			dest=src;
			OffsetRect(&dest,sel->Prime.left+1,sel->Prime.top+1);
			if(sel->Function==1)
			{	OffsetRect(&dest,index*24,0);
			}
#ifdef MACINTOSH
			CopyBits(&ButtonBits,&MyWind->portBits,&src,&dest,srcXor,0L);
#else
			PenXor();
			MiscPixCopy(src.left,src.top,&dest);
			PenCopy();
#endif
			if(sel->Function < 0)
			{	dest=src;
				OffsetRect(&dest,sel->Secondary.left+index*24+1,sel->Secondary.top+1);
#ifdef MACINTOSH
				CopyBits(&ButtonBits,&MyWind->portBits,&src,&dest,srcXor,0L);
#else
				PenXor();
				MiscPixCopy(src.left,src.top,&dest);
				PenCopy();
#endif
			}
		}
	}
}

/*
>>	This routine is called in response to an update/expose event.
>>	It just redraws all the button tags.
*/
void	UpdateButtonTags()
{
	int 	i;
	
	for(i=0;i<MOUSE_BUTTONS;i++)
	{	InvertButtonTag(i);
	}
	
	PenXor();
	FrameRect(&InputFrame);
	PenCopy();
}

/*
>>	The way a double click is detected on the Macintosh is
>>	very specific. In the unix version, it suffices to click
>>	the same tool twice. You can go have a coke in between
>>	the clicks, if you feel so and it will still work. These
>>	variables and the following routine are for double click
>>	detection.
*/
static	long	DoubleTimer=0;
static	int 	DoubleButton= -1;
static	int 	DoubleFunction=0;

void	CancelDoubleToolClick()
{
/*	Button -1 should not exist. Thus, the next click is not a double click. */
	DoubleButton= -1;
}

/*
>>	This routine is called in response to a mousedown in the
>>	tool palette. It tracks the mouse and determines what was
>>	selected. The selection is just marked and remembered. There
>>	is no further action. Double clicks are detected.
*/
void	DoToolClick()
{
	Rect		HiliteRect,OldHiliteRect,Inside,NotRect;
	int 		HiliteItem,NewHilite,NotIsOn,i,downflag;
	Point		MousePoint;
	int 		WhichButton;
	int 		DEnable;
	
	WhichButton=GetDownButton();
	
	if(DoubleButton==WhichButton)
		DEnable= -1;	/*	Same button twice. Possibly double click.	*/
	else
		DEnable=0;

#ifdef	MACINTOSH		/*	Macintosh requires double click timing. 	*/
	if(DoubleTimer+GetDblTime()<MyEvent.when)
		DEnable=0;

	SetPort(MyWind);
#endif

	DoubleButton=WhichButton;	/*	Remember for next time. 			*/

	Inside.left=ToolR.left+33;	/*	Define active tool palette bounds.	*/
	Inside.right=ToolR.right-3; /*	Eliminate shadow area.				*/
	Inside.top=ToolR.top+1;
	Inside.bottom=ToolR.bottom-3;

	HiliteItem=0;				/*	Nothing selected.					*/
	NotIsOn=0;					/*	The "NOT" modifier is not on.		*/
	
	do	/*	Track mouse until button comes up.							*/
	{	downflag=GetMouseTrackEvent(&MousePoint);
		if(PtInRect(MousePoint,&Inside))
		{	MousePoint.v-=Inside.top;
			NewHilite=MousePoint.v/24;	/*	24 pixel high buttons.		*/

			HiliteRect=Inside;
			HiliteRect.top=Inside.top+24*NewHilite;
			HiliteRect.bottom=HiliteRect.top+23;

			if(NewHilite)		/*	Is this something else than "NOT"?	*/
			{	NewHilite=NewHilite*3-2;	/*	Three buttons/row.		*/
				MousePoint.h-=Inside.left;
				MousePoint.h/=24;			/*	24 pixel wide buttons.	*/
				NewHilite+=MousePoint.h;
				HiliteRect.left+=MousePoint.h*24;
				HiliteRect.right=HiliteRect.left+23;
			}
			
			NewHilite++;			/*	Adjust so that 0 is unused. 	*/
			
			if(NotIsOn && NewHilite>4)	/*	Deselect "NOT" option.		*/
			{	NotIsOn=0;
				HILITEMODE;
				InvertRect(&NotRect);
			}
			
			if(NotIsOn && NewHilite==1) /*	Deselect "NOT" option.		*/
			{	if(HiliteItem)	/*	Unhilite old item.					*/
				{	HILITEMODE;
					InvertRect(&OldHiliteRect);
				}
				NotIsOn=0;
				HiliteItem=1;	/*	This item is already hilited.		*/
			}		

			/*	Move from NOT to something that accepts "NOT"?			*/
			if(NewHilite<5 && NewHilite!=1 && HiliteItem==1)
			{	NotIsOn= -1;
				NotRect=OldHiliteRect;
				HiliteItem=NewHilite;
				HILITEMODE;
				InvertRect(&HiliteRect);
			}	
		}
		else	/*	Mouse is outside active area.						*/
		{	NewHilite=0;
			if(NotIsOn)
			{	NotIsOn=0;
				HILITEMODE;
				InvertRect(&NotRect);
			}
		}
		if(HiliteItem!=NewHilite)	/*	Do we have to change the hilite?	*/
		{	if(HiliteItem)			/*	Unhilite old item.					*/
			{	HILITEMODE
				InvertRect(&OldHiliteRect);
			}
			if(NewHilite)			/*	Hilite new item.					*/
			{	HILITEMODE
				InvertRect(&HiliteRect);
			}
		}
		HiliteItem=NewHilite;
		OldHiliteRect=HiliteRect;
	}	while(downflag);			/*	Did mouse button come up?			*/

#ifdef	MACINTOSH
	DoubleTimer=MyEvent.when;
#endif
		
	if(HiliteItem)					/*	Was a tool selected?				*/
	{	for(i=MenuFlash*2;i;i--)	/*	Flash that tool.					*/
		{	long	dummy;
		
			HILITEMODE
			InvertRect(&OldHiliteRect);
			if(NotIsOn)
			{	HILITEMODE;
				InvertRect(&NotRect);
			}
			Delay(2,&dummy);
		}	

		HILITEMODE					/*	Unhilite tool.						*/
		InvertRect(&OldHiliteRect);

		if(NotIsOn) 				/*	Unhilite NOT.						*/
		{	HILITEMODE;
			InvertRect(&NotRect);
		}
		
		/*	Did user select the number of inputs in AND_, OR_,...			*/
		if(HiliteItem>=5 && HiliteItem <=7)
		{	PenXor();
			FrameRect(&InputFrame);
			InputFrame=OldHiliteRect;
			InsetRect(&InputFrame,1,1);
			InputSelector=HiliteItem-3;
			FrameRect(&InputFrame);
			PenCopy();
		}
		else	/*	User selected a real tool. Store it in ToolButtons. 	*/
		{	InvertButtonTag(WhichButton);	/*	Undraw old tag. 			*/
		
			if(NotIsOn)
			{	HiliteItem= -HiliteItem;
				ToolButtons[WhichButton].Secondary=NotRect;
			}
			else
			{	ToolButtons[WhichButton].Secondary=NilRect;
			}
			ToolButtons[WhichButton].Function=HiliteItem;
			ToolButtons[WhichButton].Prime=OldHiliteRect;
			
			if(HiliteItem==DoubleFunction && DEnable)	/*	Double click?	*/
			{	ToolLocks[WhichButton]=ToolButtons[WhichButton];
			}
			DoubleFunction=HiliteItem;

			InvertButtonTag(WhichButton);	/*	Draw new tag.				*/
		}
	}
}
