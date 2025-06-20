/*
 * The stuff in this file presents a movable modal dialog with an outlined
 * pushbutton.  It demonstrates:
 * - Movable modal dialog handling.
 * - Button outlining in a movable modal dialog.
 *
 * Clicks in either the Cancel or OK buttons dismiss the dialog.
 */

# include	"TransSkel.h"

# include	"Button.h"


typedef enum
{
	okBtn = 1,
	cancelBtn
};


static DialogPtr	dlog = (DialogPtr) nil;


/*
 * This filter must react to deactivates, not just activates, because the
 * application can be put in the background while the movable modal dialog
 * is in front.
 */

static pascal Boolean
Filter (DialogPtr dlog, EventRecord *evt, short *item)
{
Boolean	result = false;
short	hilite;

	switch (evt->what)
	{
	case updateEvt:
		SkelDrawButtonOutline (SkelGetDlogCtl (dlog, okBtn));
		break;
	case activateEvt:
		hilite = ((evt->modifiers & activeFlag) ? normalHilite : dimHilite);
		(void) SkelSetDlogCtlHilite (dlog, okBtn, hilite);
		SkelDrawButtonOutline (SkelGetDlogCtl (dlog, okBtn));
		(void) SkelSetDlogCtlHilite (dlog, cancelBtn, hilite);
		break;
	case keyDown:
		result = SkelDlogMapKeyToButton (dlog, evt, item, okBtn, cancelBtn);
		break;
	}
	return (result);
}


static pascal void
Select (DialogPtr dlog, short item)
{
	switch (item)
	{
	case okBtn:
	case cancelBtn:
		HideWindow (dlog);
		AdjustMenus ();
		break;
	}
}


static pascal void
Clobber (void)
{
DialogPtr	dlog;

	GetPort (&dlog);
	HideWindow (dlog);
	DisposeDialog (dlog);
}


/*
 * Present movable modal dialog
 */

void
DoMovableModal (void)
{
	if (dlog != (DialogPtr) nil)
	{
		SelectWindow (dlog);
		ShowWindow (dlog);
		return;
	}
	dlog = GetNewDialog (movableRes, nil, (WindowPtr) -1L);
	if (dlog == (DialogPtr) nil)
	{
		SysBeep (1);
		return;
	}
	SkelPositionWindow (dlog, skelPositionOnMainDevice, horizRatio, vertRatio);
	SkelDialog (dlog,
				Filter,
				Select,
				nil,			/* no close box, so no close handler */
				Clobber);
	ShowWindow (dlog);
	AdjustMenus ();
}
