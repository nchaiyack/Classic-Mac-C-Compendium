/*
 * The stuff in this file presents a modeless dialog with an outlined
 * pushbutton.  It demonstrates:
 * - Button outlining in a modeless dialog.
 *
 * Note that button clicks are tracked and return/enter and escape/
 * command-period keyclicks are mapped onto button clicks, but that no
 * other action is associated with those clicks.  This window hander
 * just shows the visible user interface stuff associated with those
 * actions.
 */

# include	"TransSkel.h"

# include	"Button.h"


typedef enum
{
	okBtn = 1,
	cancelBtn
};


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
Clobber (void)
{
DialogPtr	dlog;

	GetPort (&dlog);
	HideWindow (dlog);
	DisposeDialog (dlog);
}


/*
 * Initialize modeless dialog
 */

void
SetupModeless (void)
{
DialogPtr	dlog;

	dlog = GetNewDialog (modelessRes, nil, (WindowPtr) -1L);
	if (dlog == (DialogPtr) nil)
	{
		SysBeep (1);
		return;
	}
	SkelDialog (dlog,
				Filter,
				nil,
				nil,			/* no close box, so no close handler */
				Clobber);
	ShowWindow (dlog);
	SkelDoEvents (activMask + updateMask);
}
