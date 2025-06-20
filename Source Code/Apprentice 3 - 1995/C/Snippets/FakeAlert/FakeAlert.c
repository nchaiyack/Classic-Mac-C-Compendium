#include	<Dialogs.h>
#include	<Types.h>
#include	"FakeAlert.h"
#include	"WindUtils.h"

//This code was originally found in a TransSkel release
//This version has been polished a bit by James Chandler Jr., CIS 71333,2651
//It draws an alert without requiring an ALRT or DITL resource

//examples---

//FakeAlert ("\pMIDI Manager already open!", "\p", "\p", "\p", 1, 1, "\pOK",  "\p", "\p");
//draws alert string 1, uses 1 button "OK", with button 1 the default

//rc = FakeAlert ("\pThe file \"", SetList[CurrentSetListItem],"\p\" could not be found.  Want to find it?.","\p", 2, 1, "\pFind",  "\pCancel", "\p");
//	if (rc == 1)
//	{
//	}
//else
//	{
//	}
// Prints--  The file "WipeOut" could not be found.  Want to find it?
// assuming SetList[CurrentSetListItem] is an unsigned char string pointer
// then it puts up two buttons, "Find" and "Cancel", with "Find" as the default.

//This version will properly respond to return key as equivalent to clicking default button

static short DefaultButton;

static pascal Boolean myFilterProc(DialogPtr theDialog, EventRecord *theEvent, short *itemHit);	//prototype

/*
	In-memory item list for dialog with four items:

	1	"^0^1^2^3" (static text)
	2	Button 1
	3	Button 2
	4	Button 3

	The caller of FakeAlert passes the four strings that are to be
	substituted into the first item, the number of buttons that
	should be used, and the titles to put into each button.
	A copy of the item list is hacked to use the right number of
	buttons.

	Thanks to Erik Kilk and Jason Haines.  Some of the stuff to do
	this is modified from code they wrote.
*/


static short	itemList [] =
{
	3,					/* max number of items - 1 */

/*
	statText item
*/
	0, 0,				/* reserve a long for item handle */
	10, 27, 61, 225,	/* display rectangle */
	((8 + 128) << 8) | 8,	/* 8 + 128 = statText (disabled), title 8 bytes long */
	'^0', '^1',		/* ^0^1^2^3 */
	'^2', '^3',

/*
	first button
*/

	0, 0,				/* reserve a long for item handle */
	102, 140, 120, 210,	/* display rectangle */
	(4 << 8) | 0,		/* 4 = pushButton, title is 0 bytes long*/

/*
	second button
*/

	0, 0,				/* reserve a long for item handle */
	102, 30, 120, 100,	/* display rectangle */
	(4 << 8) | 0,		/* 4 = pushButton, title is 0 bytes long */

/*
	third button
*/

	0, 0,				/* reserve a long for item handle */
	78, 30, 96, 100,	/* display rectangle */
	(4 << 8) | 0		/* 4 = pushButton, title is 0 bytes long */
};


/*
	Set dialog button title and draw bold outline if makeBold true.
	This must be done after the window is shown or else the bold
	outline won't show up (which is probably the wrong way to do it).
*/

static void SetDControl (DialogPtr	theDialog,
				short	itemNo,
				StringPtr	title,
				Boolean	makeBold)
{
Handle	itemHandle;
short	itemType;
Rect		itemRect;
PenState	pState;

	GetDItem (theDialog, itemNo, &itemType, &itemHandle, &itemRect);
	SetCTitle ((ControlHandle) itemHandle, title);
	if (makeBold)
	{
		GetPenState (&pState);
		PenNormal ();
		PenSize (3, 3);
		InsetRect (&itemRect, -4, -4);
		FrameRoundRect (&itemRect, 16, 16);
		SetPenState (&pState);
	}
}


/*
	Fake an alert, using an in-memory window and item list.
	The message to be presented is constructed from the first
	four arguments.  nButtons is the number of buttons to use,
	defButton is the default button, the next three args are
	the titles to put into the buttons.  The return value is
	the button number (1..nButtons).  This must be interpreted
	by the caller, since the buttons may be given arbitrary
	titles.

	nButtons should be between 1 and 3, inclusive.
	defButton should be between 1 and nButtons, inclusive.
*/


short FakeAlert (StringPtr	s1,
			StringPtr	s2,
			StringPtr	s3,
			StringPtr	s4,
			short	nButtons,
			short	defButton,
			StringPtr	t1, 
			StringPtr	t2,
			StringPtr	t3)
{
GrafPtr			savePort;
DialogPtr			theDialog;
Handle			iListHandle;
Rect				bounds;
Rect				r;
short			itemHit;
short			itemType;
long				finalTick;
ControlHandle		theControl;
AuxWinHandle		wincolors;

	DefaultButton = defButton;
	InitCursor ();
	GetPort (&savePort);
	iListHandle = NewHandle (512L);
	HLock (iListHandle);
	BlockMove (&itemList, *iListHandle, 512L);
	((short *) *iListHandle)[0] = nButtons;	/* = number items - 1 */
	SetRect (&bounds, 115, 80, 355, 220);
	if (HasColorQD) theDialog = (DialogPtr) NewCDialog (nil, &bounds, "\p", false, dBoxProc, (WindowPtr) -1L, false, 0L, iListHandle);
	//uses public var "HasColorQD" from WindUtils to determine if running on a color mac
	//if you don't want to use WindUtils, you can just use a black'n'white alert window all the time
	//or copy the relevant code from the ColorImplemented() routine in WindUtils.c
	else theDialog = NewDialog (nil, &bounds, "\p", false, dBoxProc, (WindowPtr) -1L, false, 0L, iListHandle);

	ParamText (s1, s2, s3, s4);		/* construct message */

	SetPort (theDialog);
	TextFont(geneva);
	TextSize(12);
	TextFace(0);
	ShowWindow (theDialog);

	switch (nButtons)				/* set button titles */
	{
		case 3:
			SetDControl (theDialog, 4, t3, defButton == 3);
			//fall through...
		case 2:
			SetDControl (theDialog, 3, t2, defButton == 2);
			//fall through...
		case 1:
			SetDControl (theDialog, 2, t1, defButton == 1);
	}
	
	ModalDialog (myFilterProc, &itemHit);
	
	GetDItem(theDialog, itemHit, &itemType, (Handle *) &theControl, &r);
	Delay(5, &finalTick);
	HiliteControl(theControl, 10);
	Delay(10, &finalTick);
	HiliteControl(theControl, 0);
	Delay(5, &finalTick);

	HUnlock (iListHandle);
	DisposDialog (theDialog);
	SetPort (savePort);
	return (itemHit - 1);
	// returns 1 -> 3 for button hit.  If key hit, returns default button number
}

static pascal Boolean myFilterProc(DialogPtr theDialog, EventRecord *theEvent, short *itemHit)
{

	if (theEvent->what == keyDown)
	{
		if ((theEvent->message & charCodeMask) == 0x0D)
		{
			*itemHit = DefaultButton + 1;
			return(true);
		}
	}
	return(false);
}