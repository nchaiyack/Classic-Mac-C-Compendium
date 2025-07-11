#include "dialogs.h"
#include "main.h"

void PositionDialog(ResType theType, short theID)
/* call this function BEFORE loading an alert or dialog box; this function will
   load the alert/dialog into memory and position it centered horizontally and
   positioned 1:3 vertically.  Then, load the alert/dialog right after the call
   to PositionDialog, and the alert/dialog will be loaded from the copy in
   memory, which has already been positioned correctly.  Cute, eh?  See error.c
   for an example. */
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
			if ((theChar==0x0d) || (theChar==0x03))		/* RETURN or ENTER */
			{
				*theItem=FakeSelect(theDialog, 1);
				return TRUE;
			}
			break;
		case updateEvt:
			if ((theEvent->message)!=(unsigned long)theDialog)
				DispatchEvents(*theEvent, FALSE);
			else
				OutlineDefaultButton(theDialog, 1);
			break;
	}
	
	return FALSE;	/* no faking, proceed as planned */
}

pascal Boolean TwoButtonFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem)
/* use this as the proc filter when calling ModalDialog -- it maps RETURN and
   ENTER to button 1, and ESCAPE and COMMAND-PERIOD to button 2.  Of course,
   button 1 should be the OK button and button 2 should be the cancel button. */
{
	unsigned char	theChar;
	
	switch (theEvent->what)	/* examine event record */
	{
		case keyDown:	/* keypress */
		case autoKey:
			theChar=theEvent->message & charCodeMask;	/* get ascii char value */
			if ((theChar==0x0d) || (theChar==0x03))		/* RETURN or ENTER */
			{
				*theItem=FakeSelect(theDialog, 1);
				return TRUE;
			}
			if ((theChar==0x1b) || (theChar=='`') ||	/* ESCAPE */
				((theEvent->modifiers & cmdKey) && (theChar=='.')))	/* COMMAND-PERIOD */
			{
				*theItem=FakeSelect(theDialog, 2);
				return TRUE;
			}
			break;
		case updateEvt:
			if ((theEvent->message)!=(unsigned long)theDialog)
				DispatchEvents(*theEvent, FALSE);
			else
				OutlineDefaultButton(theDialog, 1);
			break;
	}
	
	return FALSE;	/* no faking, proceed as planned */
}

pascal Boolean ThreeButtonFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem)
/* use this as the proc filter when calling ModalDialog -- it maps RETURN and
   ENTER to button 1, and ESCAPE and COMMAND-PERIOD to button 2, and command-D to
   button 3.  (This is good for the "save document?" alert.  Of course,
   button 1 should be the OK button and button 2 should be the cancel button. */
{
	unsigned char	theChar;
	
	switch (theEvent->what)	/* examine event record */
	{
		case keyDown:	/* keypress */
		case autoKey:
			theChar=theEvent->message & charCodeMask;	/* get ascii char value */
			if ((theChar==0x0d) || (theChar==0x03))		/* RETURN or ENTER */
			{
				*theItem=FakeSelect(theDialog, 1);
				return TRUE;
			}
			if ((theChar==0x1b) || (theChar=='`') ||	/* ESCAPE */
				((theEvent->modifiers & cmdKey) && (theChar=='.')))	/* COMMAND-PERIOD */
			{
				*theItem=FakeSelect(theDialog, 2);
				return TRUE;
			}
			if ((theEvent->modifiers & cmdKey) && ((theChar|0x20)=='d'))
			{
				*theItem=FakeSelect(theDialog, 3);
				return TRUE;
			}
			break;
		case updateEvt:
			if ((theEvent->message)!=(unsigned long)theDialog)
				DispatchEvents(*theEvent, FALSE);
			else
				OutlineDefaultButton(theDialog, 1);
			break;
	}
	
	return FALSE;	/* no faking, proceed as planned */
}

short FakeSelect(DialogPtr theDialog, short itemNum)
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
{
	Rect			box;
	short			itemType;
	Handle			itemH;
	
	GetDItem(theDialog, itemNum, &itemType, &itemH, &box);
	HiliteControl((ControlHandle)itemH, isOn ? 0 : 255);
}

void SetButtonTitle(DialogPtr theDialog, short itemNum, Str255 theStr)
{
	Rect			box;
	short			itemType;
	Handle			itemH;
	
	GetDItem(theDialog, itemNum, &itemType, &itemH, &box);
	SetCTitle((ControlHandle)itemH, theStr);
}
