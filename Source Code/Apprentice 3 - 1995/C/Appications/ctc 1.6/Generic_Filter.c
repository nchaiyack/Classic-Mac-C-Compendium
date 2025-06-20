#define OKButton	1
#include <Dialogs.h>
#include "ctc.h"

/* changes the cursor to an IBeam if it's over an active edit line, or to the */
/* arrow if it's not */

/* Tells the Dialog Manager that there is an edit line in this dialog, and */ 
/* it should track and change to an I-Beam cursor when over the edit line */
pascal OSErr SetDialogTracksCursor(DialogPtr theDialog, Boolean tracks)
	= { 0x303C, 0x0306, 0xAA68 };

pascal OSErr GetStdFilterProc(ProcPtr *theProc)
		= {0x303C, 0x0203, 0xAA68};

pascal OSErr SetDialogDefaultItem (DialogPtr theDialog,
		short newItem) = {0x303C,0x0304,0xAA68};


pascal OSErr SetDialogCancelItem (DialogPtr theDialog,
		short newItem) = {0x303C,0x0305,0xAA68};


void IBeamIt(WindowPtr dwind, Rect *r)
{
    Point mouseAt;
    short itemtype;
    Handle itemhandle;
    Rect borderRect;
    short itemNum;
    
    /* first get the current edit line out of the dialog record */
    itemNum = ((DialogPeek)dwind)->editField + 1;           /* always stored 1 less */
    GetDItem(dwind, itemNum, &itemtype, &itemhandle, &borderRect);
    GetMouse(&mouseAt);
    if (PtInRect(mouseAt, &borderRect)) {
    	*r = borderRect;				/* Update Mouse Region */
        SetCursor(*(GetCursor(1)));		/* IBeam Cursor */
    } else {
        InitCursor();
     }
}

/************************************************************************
 *
 *  Function:		OutlineOK
 *
 *  Purpose:		outline the OK button in a dialog
 *
 *  Returns:		nothing
 *
 *  Side Effects:	standard box is drawn around OK button
 *
 *  Description:	draws the heavy rounded box around the OK button,
 *					so that the user knows that hitting enter or return
 *					is the same as pressing the OK button.  OK is assumed
 *					to be the first item in the dialog.
 *
 ************************************************************************/
void OutlineOK(DialogPtr dPtr)
{	int unusedItemType;
	Handle unusedItemHandle;
	Rect box;
	PenState p;
	GrafPtr	savedPort;

	/*	This next little piece of code puts the default heavy rounded
		box around the "OK" button, so the user knows that pressing
		return is the same as hitting "OK"
	*/
	GetPort(&savedPort);
	SetPort(dPtr);		/* without this, can't highlite OK */
	GetDItem(dPtr, OKButton, &unusedItemType, &unusedItemHandle, &box);
	GetPenState(&p);
	PenSize(3,3);
	InsetRect(&box, -4, -4);
	FrameRoundRect(&box, 16, 16);
	PenSize(p.pnSize.h, p.pnSize.v);
	SetPort(savedPort);
}


/************************************************************************
 *
 *  Function:		GenericFilter
 *
 *	Purpose:		generic dialog filter
 *
 *	Returns:		true if nothing has happened, false if a choice has
 *					been made.  itemHit is modified to return the value
 *					of the item selected.
 *
 *	Side Effects:	Handles cut, copy, paste, cancel for system 6.
 *
 *	Description:	You'll get either an updateEvt, a keyDown, or an 
 *					autoKey.  On updateEvt, handle window updates and
 *					if necessary, draw the OK outline.  If a keydown (or
 *					autokey) handle the key as apporpriate.
 *
 ************************************************************************/
pascal Boolean GenericFilter(DialogPtr theDialog,EventRecord *theEvent,short *itemHit)
{	short code;
	WindowPtr updateWindow;
  	WindowPtr wWindow;
	char theChar;
	Rect r;
	
	IBeamIt(theDialog, &r);
	switch (theEvent->what) {
	   case updateEvt:
		   updateWindow = (WindowPtr) theEvent->message;
		   if (updateWindow!=theDialog)
				HandleUpdates(updateWindow);    /* <<<----- call window updates here */
		   else	OutlineOK(theDialog);   /*  <<<--- I draw ok outline here */
		   break;
		   
	   case mouseDown:
	   		SetPort(theDialog);											/* Add Help Text to Dialog.... */
	
			code = FindWindow(theEvent->where, &wWindow);
			
			switch (code) {
				case inDrag :
					if (wWindow == theDialog) {
						Rect DragRect = (**GetGrayRgn()).rgnBBox;
						DragWindow(wWindow, theEvent->where, &DragRect);
						}
					else SysBeep(0);
					*itemHit = 0;
					return true;
					break;
				}
	   		break;
	   		
	   case keyDown:
	   case autoKey:
		   theChar = theEvent->message & charCodeMask;
		   if ((theEvent->modifiers & cmdKey) != 0) {
			   switch (theChar) {
				case 'M':
				case 'm':
				   *itemHit = MAC;	return true;
				case 'U':
				case 'u':
				   *itemHit = UNIX;	return true;
				case 'D':
				case 'd':
				   *itemHit = DOS;	return true;
				case 'I':
				case 'i':
				   *itemHit = IGNORE;	return true;
				case 'X':
				case 'x':
				   DlgCut(theDialog);
				   *itemHit = 0;	return true;
				case 'C':
				case 'c':
				   DlgCopy(theDialog);
				   *itemHit = 0;	return true;
				case 'V':
				case 'v':
				   DlgPaste(theDialog);
				   *itemHit = 0;	return true;
			   case '.':
				   *itemHit = Cancel;	return true;
			   default:	return false;
			  }
		   } else
		   switch (theChar) {
			   case 0x0d:  /* CR */
			   case 0x03:  /* enter */
				   *itemHit = OK;
				   return true;
			   case 0x1b:  /* ESC */
				   *itemHit = Cancel;
			   return true;
		   }
		   break;
	   }
   return false;
}

