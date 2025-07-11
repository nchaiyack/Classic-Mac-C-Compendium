#include "dialog layer.h"
#include "key layer.h"
#include "list layer.h"
#include "window layer.h"
#include "main.h"
#include "program globals.h"

DialogPtr SetupTheDialog(short resourceID, short userItemForDefaultButton,
	unsigned char *dialogTitle, UniversalProcPtr filterProc,
	UniversalProcPtr *modalFilterProc, UniversalProcPtr *userItemProc)
/* returns pointer to dialog, stores ModalFilterUPP in modalFilterProc parameter */
{
	short			itemType;
	Handle			item;
	Rect			box;
	DialogPtr		theDialog;
	UserItemUPP		userUPP=NewUserItemProc(OutlineDefaultButton);
	
	DeactivateFloatersAndFirstDocumentWindow();
	PositionDialog('DLOG', resourceID);
	theDialog = GetNewDialog(resourceID, 0L, (WindowRef)-1L);
	GetDItem(theDialog, userItemForDefaultButton, &itemType, &item, &box);
	InsetRect(&box, -4, -4);
	SetDItem(theDialog, userItemForDefaultButton, userItem, (Handle)userUPP, &box);
	SetWTitle(theDialog, dialogTitle);
	*modalFilterProc=(UniversalProcPtr)NewModalFilterProc(filterProc);
	*userItemProc=(UniversalProcPtr)userUPP;
	return theDialog;
}

short DisplayTheAlert(short alertType, short resourceID, unsigned char *paramString1,
	unsigned char *paramString2, unsigned char *paramString3, unsigned char *paramString4,
	UniversalProcPtr filterProc)
/* displays alert of ID resourceID with param. strings using filterProc and returns result */
{
	short			result;
	ModalFilterUPP	modalFilterProc;
	
	DeactivateFloatersAndFirstDocumentWindow();
	modalFilterProc=NewModalFilterProc(filterProc);
	PositionDialog('ALRT', resourceID);
	ParamText(paramString1, paramString2, paramString3, paramString4);
	switch (alertType)
	{
		case kPlainAlert:
			result=Alert(resourceID, modalFilterProc);
			break;
		case kStopAlert:
			result=StopAlert(resourceID, modalFilterProc);
			break;
		case kCautionAlert:
			result=CautionAlert(resourceID, modalFilterProc);
			break;
		case kNoteAlert:
			result=NoteAlert(resourceID, modalFilterProc);
			break;
	}
	DisposeRoutineDescriptor(modalFilterProc);
	ActivateFloatersAndFirstDocumentWindow();
	return result;
}

void DisplayTheDialog(DialogPtr theDialog, Boolean makeVisible)
/* shows and hides the dialog */
{
	if (makeVisible)
	{
		SelectWindow(theDialog);
		ShowWindow(theDialog);
		SetPort(theDialog);
	}
	else
	{
		HideWindow(theDialog);
	}
}

void ShutDownTheDialog(DialogPtr theDialog, UniversalProcPtr modalFilterProc,
	UniversalProcPtr userItemProc)
/* hides and disposes of dialog and modal filter proc */
{
	#pragma unused(modalFilterProc, userItemProc)
	
	HideWindow(theDialog);
	DisposeDialog(theDialog);
	DisposeRoutineDescriptor((ModalFilterUPP)modalFilterProc);
	DisposeRoutineDescriptor((UserItemUPP)userItemProc);
	ActivateFloatersAndFirstDocumentWindow();
}

short GetOneDialogEvent(DialogPtr theDialog, UniversalProcPtr modalFilterProc)
/* returns item number of item selected (0 if none) */
{
	#pragma unused(theDialog)
	
	short			itemSelected;
	
	ModalDialog((ModalFilterUPP)modalFilterProc, &itemSelected);
	return itemSelected;
}

void GetDialogItemString(DialogPtr theDialog, short itemNum, unsigned char *theString)
/* returns text of item in theString parameter; will never be more than 255 characters */
{
	short			itemType;
	Handle			item;
	Rect			box;
	
	GetDItem(theDialog, itemNum, &itemType, &item, &box);
	GetIText(item, theString);
}

void SetDialogItemString(DialogPtr theDialog, short itemNum, unsigned char *theString)
/* sets text of item to theString parameter; will be chopped to 255 characters */
{
	short			itemType;
	Handle			item;
	Rect			box;
	
	GetDItem(theDialog, itemNum, &itemType, &item, &box);
	SetIText(item, theString);
}

void SetDialogUserItem(DialogPtr theDialog, short itemNum, UniversalProcPtr proc,
	UniversalProcPtr *outputProc)
{
	#pragma unused(proc)
	
	short			itemType;
	Handle			item;
	Rect			box;
	UserItemUPP		userUPP=NewUserItemProc(OutlineDefaultButton);
	
	GetDItem(theDialog, itemNum, &itemType, &item, &box);
	SetDItem(theDialog, itemNum, userItem, (Handle)userUPP, &box);
	*outputProc=(UniversalProcPtr)userUPP;
}

void GetDialogItemRect(DialogPtr theDialog, short itemNum, Rect *theRect)
{
	short			itemType;
	Handle			item;
	
	GetDItem(theDialog, itemNum, &itemType, &item, theRect);
}

void HighlightDialogTextItem(DialogPtr theDialog, short itemNum, short start, short end)
/* highlights part or all of a text dialog item */
{
	SelIText(theDialog, itemNum, start, end);
}

short GetDialogCurrentEditField(DialogPtr theDialog)
{
	return ((DialogPeek)theDialog)->editField+1;
}

TEHandle GetDialogCurrentTextHandle(DialogPtr theDialog)
{
	return ((DialogPeek)theDialog)->textH;
}

Boolean AnyHighlightedInDialogQQ(DialogPtr theDialog)
{
	TEHandle		hTE;
	
	hTE=GetDialogCurrentTextHandle(theDialog);
	return ((**hTE).selStart!=(**hTE).selEnd);
}

short FakeSelect(DialogPtr theDialog, short itemNum)
/* looks and feels like the real thing; fakes a button selection */
{
	short			itemType;
	Handle			itemH;
	Rect			box;
	long			dummy;
	
	GetDItem(theDialog, itemNum, &itemType, &itemH, &box);
	HiliteControl((ControlHandle)itemH, 1);
	Delay(8, &dummy);
	HiliteControl((ControlHandle)itemH, 0);
	
	return itemNum;
}

void SetButtonHighlight(DialogPtr theDialog, short itemNum, Boolean isOn)
/* highlights and dims dialog buttons */
{
	Rect			box;
	short			itemType;
	Handle			itemH;
	
	GetDItem(theDialog, itemNum, &itemType, &itemH, &box);
	HiliteControl((ControlHandle)itemH, isOn ? 0 : 255);
}

Boolean ButtonIsHighlightedQQ(DialogPtr theDialog, short itemNum)
/* returns TRUE if button is highlighted (available), false if dimmed (unavailable) */
{
	Rect			box;
	short			itemType;
	Handle			itemH;
	
	GetDItem(theDialog, itemNum, &itemType, &itemH, &box);
	return ((**(ControlHandle)itemH).contrlHilite==255) ? FALSE : TRUE;
}

void SetButtonTitle(DialogPtr theDialog, short itemNum, Str255 theStr)
/* self-explanatory */
{
	Rect			box;
	short			itemType;
	Handle			itemH;
	
	GetDItem(theDialog, itemNum, &itemType, &itemH, &box);
	SetCTitle((ControlHandle)itemH, theStr);
}

void SetCheckboxState(DialogPtr theDialog, short itemNum, Boolean isOn)
{
	Rect			box;
	short			itemType;
	Handle			itemH;
	
	GetDItem(theDialog, itemNum, &itemType, &itemH, &box);
	SetControlValue((ControlHandle)itemH, isOn ? 1 : 0);
}

Boolean CheckboxIsCheckedQQ(DialogPtr theDialog, short itemNum)
{
	Rect			box;
	short			itemType;
	Handle			itemH;
	
	GetDItem(theDialog, itemNum, &itemType, &itemH, &box);
	return (GetControlValue((ControlHandle)itemH)==0) ? FALSE : TRUE;
}

void SetDialogControlInfo(DialogPtr theDialog, short itemNum, short min, short max, short value)
{
	Rect			box;
	short			itemType;
	Handle			itemH;
	
	GetDItem(theDialog, itemNum, &itemType, &itemH, &box);
	SetControlMinimum((ControlHandle)itemH, min);
	SetControlMaximum((ControlHandle)itemH, max);
	SetControlValue((ControlHandle)itemH, value);
}

short GetDialogControlValue(DialogPtr theDialog, short itemNum)
{
	Rect			box;
	short			itemType;
	Handle			itemH;
	
	GetDItem(theDialog, itemNum, &itemType, &itemH, &box);
	return GetControlValue((ControlHandle)itemH);
}

void SetDialogControlValue(DialogPtr theDialog, short itemNum, short theValue)
{
	Rect			box;
	short			itemType;
	Handle			itemH;
	
	GetDItem(theDialog, itemNum, &itemType, &itemH, &box);
	SetControlValue((ControlHandle)itemH, theValue);
}

void PositionDialog(ResType theType, short theID)
/* call this function BEFORE loading an alert or dialog box; this function will
   load the alert/dialog into memory and position it centered horizontally and
   positioned 1:3 vertically.  Then, load the alert/dialog right after the call
   to PositionDialog, and the alert/dialog will be loaded from the copy in
   memory, which has already been positioned correctly.  Cute, eh? */
{
	Handle				theTemplate;	/* Handle to resource template	*/
	register Rect		*theRect;		/* Bounding box of dialog		*/
	register short		left;			/* Left side of centered rect	*/
	register short		top;			/* Top side of centered rect	*/
	
	/* The first field of the resource template for DLOG's and ALRT's */
	/* is its bounding box.  Get a pointer to this rectangle.  This   */
	/* handle dereferencing is safe since the remaining statements in */
	/* this function do not move memory (assignment and simple math). */

	theTemplate = GetResource(theType, theID);
	if (theTemplate == 0)
		return;
	theRect=(Rect*)*theTemplate;	/* bounding rectangle */
	
	left = (qd.screenBits.bounds.right - (theRect->right - theRect->left)) / 2;
	top = (qd.screenBits.bounds.bottom - (theRect->bottom - theRect->top)) / 3;
	if (top < (GetMBarHeight() + 1))	/* don't put it over menu bar */
		top = GetMBarHeight() + 1;

	theRect->right += left - theRect->left;
	theRect->left = left;
	theRect->bottom += top - theRect->top;
	theRect->top = top;
}

pascal void OutlineDefaultButton(DialogPtr myDlog, short itemNum)
/* Use this as the useritem for a dialog which needs an outlined default button. */
/* Make sure the default button is item 1 in the DITL. */
{
	#pragma unused(itemNum)
	
	short			itemType;
	Handle			itemH;
	Rect			box;
	
	GetDItem(myDlog, 1, &itemType, &itemH, &box);
	PenSize(3, 3);
	InsetRect(&box, -4, -4);
	FrameRoundRect(&box, 16, 16);
	PenNormal();
}

pascal Boolean OneButtonFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem)
/* use this as the proc filter when calling ModalDialog -- it maps RETURN and
   ENTER to button 1.  Of course, button 1 should be the OK button. */
{
	unsigned char	theChar;
	
	switch (theEvent->what)	/* examine event record */
	{
		case keyDown:	/* keypress */
		case autoKey:
			theChar=theEvent->message & charCodeMask;	/* get ascii char value */
			if ((theChar==key_Return) || (theChar==key_Enter))		/* RETURN or ENTER */
			{
				*theItem=FakeSelect(theDialog, ok);
				return TRUE;
			}
			break;
		case updateEvt:
			if ((theEvent->message)!=(unsigned long)theDialog)
				DispatchEvents(*theEvent, FALSE);
			else
				OutlineDefaultButton(theDialog, ok);
			break;
	}
	
	return FALSE;	/* no faking, proceed as planned */
}

pascal Boolean TwoButtonFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem)
/* use this as the proc filter when calling ModalDialog -- it maps RETURN and
   ENTER to button 1, and ESCAPE and COMMAND-PERIOD to button 2.  Of course,
   button 1 should be the OK button and button 2 should be the cancel button. */
{
	return PoorMansFilterProc(theDialog, theEvent, theItem, 0L);
}

pascal Boolean SaveModalFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem)
/* use this as the proc filter when calling ModalDialog -- it maps RETURN and
   ENTER to button 1, and ESCAPE and COMMAND-PERIOD to button 2, and command-D to
   button 3.  (This is good for the "save document?" alert.)  Of course,
   button 1 should be the OK button and button 2 should be the cancel button. */
{
	if (PoorMansFilterProc(theDialog, theEvent, theItem, 0L))
		return TRUE;
	if (CheckCommandKey(theDialog, theEvent, theItem, 'D', 3))
		return TRUE;
	
	return FALSE;
}

pascal Boolean PoorMansFilterProc(DialogPtr theDialog, EventRecord *theEvent, short *theItem,
	ListHandle theList)
/* not to be used as a modal filter proc by itself! (too many params) -- see above for examples */
{
	WindowRef		theWindow;
	Rect 			oPortRect, dDragRect;
	short 			thePart;
	Point 			thePoint, startPoint, offSet, theLocalPoint;
	unsigned char	theChar;
	
	switch (theEvent->what)	/* examine event record */
	{
		case keyDown:	/* keypress */
		case autoKey:
			theChar=theEvent->message & charCodeMask;	/* get ascii char value */
			if ((theChar==key_Return) || (theChar==key_Enter))		/* RETURN or ENTER */
			{
				if (ButtonIsHighlightedQQ(theDialog, ok))
					*theItem=FakeSelect(theDialog, ok);
				else
					theEvent->what=nullEvent;
				return TRUE;
			}
			else if (((theChar==key_Escape) ||
				((theChar=='.') && (theEvent->modifiers&cmdKey))))
			{
				if (ButtonIsHighlightedQQ(theDialog, cancel))
					*theItem=FakeSelect(theDialog, cancel);
				else
					theEvent->what=nullEvent;
				return TRUE;
			}
			break;
		case updateEvt:
			if ((theEvent->message)!=(unsigned long)theDialog)
				DispatchEvents(*theEvent, FALSE);
			else
			{
				if (theList!=0L)
				{
					MyDrawListBorder(theList);
					MyUpdateList(theList);
				}
				OutlineDefaultButton(theDialog, ok);
			}
			break;
 		case mouseDown:
			dDragRect=qd.screenBits.bounds;
			thePoint=startPoint=theEvent->where;
			thePart=FindWindow(thePoint, &theWindow);
			offSet.h=offSet.v=0;
			InsetRect(&dDragRect, 4, 4);
			
			if (thePart == inDrag)
			{
				if (theWindow == (WindowRef)theDialog)
				{
					DragWindow(theWindow,theEvent->where, &dDragRect);
					theEvent->what=nullEvent;
					return TRUE;
				}
				else
				{
					SetPort(theWindow);
					GetMouse(&startPoint);
					LocalToGlobal(&offSet);
					oPortRect = (*theWindow).portRect;			
					oPortRect.top -=19;
					OffsetRect(&dDragRect,-offSet.h,-offSet.v);
					RectRgn((*theWindow).visRgn,&oPortRect);
					DragGrayRgn((*theWindow).visRgn,startPoint,&dDragRect,&dDragRect,0,0L);
					
					GetMouse(&thePoint);
					if (PtInRect(thePoint,&dDragRect))
					{
						LocalToGlobal(&thePoint);
						MoveWindow(theWindow,thePoint.h-startPoint.h,thePoint.v-startPoint.v,FALSE);
						if (WindowHasLayer(theWindow))
						{
							SetWindowBounds(theWindow,
								(*(((WindowPeek)theWindow)->contRgn))->rgnBBox);
							theLocalPoint.v=GetWindowBounds(theWindow).top;
							theLocalPoint.h=GetWindowBounds(theWindow).left;
							SetWindowTopLeft(theWindow, theLocalPoint);
						}
					}
					
					SetPort(theDialog);
					theEvent->what=nullEvent;
					return TRUE;
				}
			}
			break;
	}
	
	return FALSE;
} 

Boolean CheckCommandKey(DialogPtr theDialog, EventRecord *theEvent, short *theItem,
	unsigned char upperChar, short equivalentItem)
{
	unsigned char		theChar;
	
	if ((theEvent->what!=keyDown) && (theEvent->what!=autoKey))
		return FALSE;
	if ((theEvent->modifiers & cmdKey)==0)
		return FALSE;
	
	theChar=theEvent->message & charCodeMask;
	if ((theChar|0x20)==(upperChar|0x20))
	{
		if (ButtonIsHighlightedQQ(theDialog, equivalentItem))
			*theItem=FakeSelect(theDialog, equivalentItem);
		else
			theEvent->what=nullEvent;
		return TRUE;
	}
	
	return FALSE;
}
