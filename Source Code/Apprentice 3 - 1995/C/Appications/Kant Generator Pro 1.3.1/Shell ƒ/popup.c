#include "popup.h"

Boolean MouseInModelessPopUp(MenuHandle theMenu, short *theChoice, Rect *theRect,
	short menuResID)
{
	Point			popLoc;
	short			chosen;
	short			newChoice;
	
	InsertMenu(theMenu, -1);
	popLoc.h=theRect->left;
	popLoc.v=theRect->top;
	LocalToGlobal(&popLoc);
	CalcMenuSize(theMenu);
	chosen=PopUpMenuSelect(theMenu, popLoc.v, popLoc.h, 0);
	DeleteMenu(menuResID);
	
	if (chosen!=0)
	{
		newChoice=LoWord(chosen);
		if (newChoice!=*theChoice)
		{
			*theChoice=newChoice;
			return TRUE;
		}
	}
	
	return FALSE;
}
