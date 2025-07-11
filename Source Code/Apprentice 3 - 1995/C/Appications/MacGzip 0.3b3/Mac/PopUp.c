/*
 * SPDsoft 1994 for MacGzip 0.2.2
 *
 * From: Pop-up Menu Example
 * Bryan Stearns 05May87 
 */

#include "PopUp.h"

/* 22may95,ejo */
#include <Fonts.h>
#include <ToolUtils.h>

PopUpType	PopUp[NumOfPopUps];

#define PUP		PopUp[theItem-PopUpBaseItem]

#if 1
/* 22may95,ejo: this one had the wrong signature */
pascal void DrawPopUp(DialogPtr theDialog, short theItem)
#else
pascal void DrawPopUp(DialogPtr theDialog, int theItem)
#endif
{
	Str255	curStr;
	int		newWid, newLen, wid;
	Rect	r;
	
	r = PopUp[theItem-PopUpBaseItem].MenuRect;
	
	GetItem(PUP.Menu,PUP.Selection,curStr);
	InsetRect(&r,-1,-1);
	
	wid = (r.right - r.left) - (LeftSlop + RightSlop);
	newWid = StringWidth(curStr);
	
	if (newWid > wid )
	{
		newLen = (int) curStr[0];
		wid = wid - CharWidth('�');
		do
		{
			newWid = newWid - CharWidth(curStr[newLen]);
			newLen--;
		}while((newWid > wid) && (curStr[0] != 0x00));

		newLen ++;
		curStr[newLen] = '�';
		curStr[0] = (char)newLen;

	}
	FrameRect(&r);
	MoveTo(r.right,r.top+2); LineTo(r.right,r.bottom);
	LineTo(r.left+2,r.bottom);
	
	MoveTo(r.left+LeftSlop,r.bottom-BotSlop);
	DrawString(curStr);
}

int DoPopUp(DialogPtr theDialog, int theItem)
{
	Rect	r;
	Point	popLoc;
	long	chosen;
	int		choice;
	
	r = PopUp[theItem-PopUpBaseItem].MenuRect;

/*	We're going to pop up our menu. Insert our menu into the menu list,
 *	then call CalcMenuSize (to work around a bug in the Menu Manager),
 *	then call PopUpMenuSelect and let the user drag around. Note that the
 *	(top,left) parameters to PopUpMenuSelect are our item�s, converted to
 *	global coordinates.
 */
 
	InvertRect(&r);
	InsertMenu(PUP.Menu,-1);
	
	/* 22may95,ejo: where is topLeft ()? */
	/*popLoc = topLeft(r);*/
	popLoc.v = r.top;
	popLoc.h = r.left;
	
	LocalToGlobal(&popLoc);
	CalcMenuSize(PUP.Menu);
	chosen = PopUpMenuSelect(PUP.Menu, popLoc.v, popLoc.h, PUP.Selection);

	InvertRect(&r);
	DeleteMenu(PUP.MenuID);

	if (chosen != 0)
	{
		choice = LoWord(chosen);
		if ( choice != PUP.Selection)
		{
			SetItemMark(PUP.Menu,PUP.Selection,noMark);
			SetItemMark(PUP.Menu,choice,checkMark);
			PUP.Selection = choice;
						
			EraseRect(&r);
			DrawPopUp(theDialog, theItem);
		}
	}
	
	return( choice ); 
}