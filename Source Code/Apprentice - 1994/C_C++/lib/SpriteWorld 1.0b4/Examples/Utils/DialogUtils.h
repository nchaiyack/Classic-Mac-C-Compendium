///--------------------------------------------------------------------------------------
//	DialogUtils.h
//
//	Created:	12/16/91 @ 11:37:55 PM
//	By:		Tony Myles
//
//	Copyright � 1991-94, Tony Myles	All rights reserved worldwide.
//
//	Description: prototypes for dialog utility routines
///--------------------------------------------------------------------------------------


#ifndef __DIALOGUTILS__
#define __DIALOGUTILS__

#ifndef __TYPES__
#include <Types.h>
#endif

#ifndef __DIALOGS__
#include <Dialogs.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


Boolean GetDItemHilite(DialogPtr dlgP, short itemNum);
void SetDItemHilite(DialogPtr dlgP, short itemNum, Boolean hilite);

void GetDItemText(DialogPtr dlgP, short itemNum, Str255 itemStr);
void SetDItemText(DialogPtr dlgP, short itemNum, Str255 itemStr);

void GetDItemCtlTitle(DialogPtr dlgP, short itemNum, Str255 itemStr);
void SetDItemCtlTitle(DialogPtr dlgP, short itemNum, Str255 itemStr);

Handle GetDItemHdl(DialogPtr dlgP, short itemNum);
void SetDItemHdl(DialogPtr dlgP, short itemNum, Handle newItemH);

void GetDItemRect(DialogPtr dlgP, short itemNum, Rect *itemRect);
void SetDItemRect(DialogPtr dlgP, short itemNum, Rect *newItemRect);

void GetDItemType(DialogPtr dlgP, short itemNum, short *itemType);
void SetDItemType(DialogPtr dlgP, short itemNum, short newItemType);

void OutlineDefaultButton(DialogPtr dlgP, short itemNum);
void ClickDialogButton(DialogPtr dlgP, short itemNum);


#ifdef __cplusplus
};
#endif
#endif
