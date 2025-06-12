///--------------------------------------------------------------------------------------
//	DialogUtils.c
//
//	Created:	12/16/91 @ 11:37:55 PM
//	By:		Tony Myles
//
//	Copyright © 1991-94, Tony Myles	All rights reserved worldwide.
//
//	Description: multitudinous dialog utility routines
///--------------------------------------------------------------------------------------


#ifndef __DIALOGS__
#include <Dialogs.h>
#endif

#ifndef __OSUTILS__
#include <OSUtils.h>
#endif

#ifndef __DIALOGUTILS__
#include "DialogUtils.h"
#endif

#if MPW
#pragma segment Utils
#endif


///--------------------------------------------------------------------------------------
// GetDItemHilite
//
//	get the hilite of a dialog item
///--------------------------------------------------------------------------------------

Boolean GetDItemHilite(DialogPtr dlgP, short itemNum)
{
	short itemType;
	Rect itemRect;
	Handle itemH;

	GetDItem(dlgP, itemNum, &itemType, &itemH, &itemRect);
	return (**(ControlHandle)itemH).contrlHilite == 0;
}


///--------------------------------------------------------------------------------------
// SetDItemHilite
//
//	set the hilite of a dialog item
///--------------------------------------------------------------------------------------

void SetDItemHilite(DialogPtr dlgP, short itemNum, Boolean hilite)
{
	short itemType;
	Rect itemRect;
	Handle itemH;

	GetDItem(dlgP, itemNum, &itemType, &itemH, &itemRect);
	HiliteControl((ControlHandle)itemH, (hilite) ? 0 : 255);
}


///--------------------------------------------------------------------------------------
// GetDItemText
//
//	get the text of a dialog item
///--------------------------------------------------------------------------------------

void GetDItemText(DialogPtr dlgP, short itemNum, Str255 iStr)
{
	short itemType;
	Rect itemRect;
	Handle itemH;

	GetDItem(dlgP, itemNum, &itemType, &itemH, &itemRect);
	GetIText(itemH, iStr);				
}


///--------------------------------------------------------------------------------------
// SetDItemText
//
//	set the text of a dialog item
///--------------------------------------------------------------------------------------

void SetDItemText(DialogPtr dlgP, short itemNum, Str255 iStr)
{
	short itemType;
	Rect itemRect;
	Handle itemH;

	GetDItem(dlgP, itemNum, &itemType, &itemH, &itemRect);
	SetIText(itemH, iStr);				
}


///--------------------------------------------------------------------------------------
// GetDItemCtlTitle
//
//	get the title of a control dialog item
///--------------------------------------------------------------------------------------

void GetDItemCtlTitle(DialogPtr dlgP, short itemNum, Str255 iStr)
{
	short itemType;
	Rect itemRect;
	Handle itemH;

	GetDItem(dlgP, itemNum, &itemType, &itemH, &itemRect);
	GetCTitle((ControlHandle)itemH, iStr);				
}


///--------------------------------------------------------------------------------------
// SetDItemCtlTitle
//
//	set the title of a control dialog item
///--------------------------------------------------------------------------------------

void SetDItemCtlTitle(DialogPtr dlgP, short itemNum, Str255 iStr)
{
	short itemType;
	Rect itemRect;
	Handle itemH;

	GetDItem(dlgP, itemNum, &itemType, &itemH, &itemRect);
	SetCTitle((ControlHandle)itemH, iStr);

		// validate the control’s rectangle since the
		// Control Manager needlessly invalidates it
	itemRect = (**(ControlHandle)itemH).contrlRect;
	ValidRect(&itemRect);
}


///--------------------------------------------------------------------------------------
// GetDItemHdl
//
//	return the handle of a dialog item
///--------------------------------------------------------------------------------------

Handle GetDItemHdl(DialogPtr dlgP, short itemNum)
{
	short itemType;
	Rect itemRect;
	Handle itemH;

	GetDItem(dlgP, itemNum, &itemType, &itemH, &itemRect);

	return itemH;
}


///--------------------------------------------------------------------------------------
// SetDItemHdl
//
//	set the handle of a dialog item
///--------------------------------------------------------------------------------------

void SetDItemHdl(DialogPtr dlgP, short itemNum, Handle newItemH)
{
	short itemType;
	Rect itemRect;
	Handle itemH;

	GetDItem(dlgP, itemNum, &itemType, &itemH, &itemRect);
	SetDItem(dlgP, itemNum, itemType, newItemH, &itemRect);
}


///--------------------------------------------------------------------------------------
// GetDItemRect
//
//	return the rect of a dialog item
///--------------------------------------------------------------------------------------

void GetDItemRect(DialogPtr dlgP, short itemNum, Rect *itemRect)
{
	short itemType;
	Handle itemH;

	GetDItem(dlgP, itemNum, &itemType, &itemH, itemRect);
}


///--------------------------------------------------------------------------------------
// SetDItemRect
//
//	set the rect of a dialog item
///--------------------------------------------------------------------------------------

void SetDItemRect(DialogPtr dlgP, short itemNum, Rect *newItemRect)
{
	short itemType;
	Handle itemH;
	Rect itemRect;

	GetDItem(dlgP, itemNum, &itemType, &itemH, &itemRect);
	SetDItem(dlgP, itemNum, itemType, itemH, newItemRect);
}


///--------------------------------------------------------------------------------------
// GetDItemType
//
//	return the type of a dialog item
///--------------------------------------------------------------------------------------

void GetDItemType(DialogPtr dlgP, short itemNum, short *itemType)
{
	Handle itemH;
	Rect itemRect;

	GetDItem(dlgP, itemNum, itemType, &itemH, &itemRect);
}


///--------------------------------------------------------------------------------------
// SetDItemType
//
//	set the type of a dialog item
///--------------------------------------------------------------------------------------

void SetDItemType(DialogPtr dlgP, short itemNum, short newItemType)
{
	short itemType;
	Handle itemH;
	Rect itemRect;

	GetDItem(dlgP, itemNum, &itemType, &itemH, &itemRect);
	SetDItem(dlgP, itemNum, newItemType, itemH, &itemRect);
}


///--------------------------------------------------------------------------------------
// OutlineDefaultButton
///--------------------------------------------------------------------------------------

void OutlineDefaultButton(DialogPtr dlgP, short itemNum)
{
	Rect buttonRect;
	
	GetDItemRect(dlgP, itemNum, &buttonRect);

	InsetRect(&buttonRect, -4, -4);
	PenSize(3, 3);
	FrameRoundRect(&buttonRect, 16, 16);
	PenNormal();
}


///--------------------------------------------------------------------------------------
// ClickDialogButton
///--------------------------------------------------------------------------------------

void ClickDialogButton(DialogPtr dlgP, short itemNum)
{
	ControlHandle dialogButtonH;
	long finalTicks;

	dialogButtonH = (ControlHandle)GetDItemHdl(dlgP, itemNum);

	if (dialogButtonH != NULL)
	{
		HiliteControl(dialogButtonH, 1);
		Delay(10, &finalTicks);
		HiliteControl(dialogButtonH, 0);
	}
}

