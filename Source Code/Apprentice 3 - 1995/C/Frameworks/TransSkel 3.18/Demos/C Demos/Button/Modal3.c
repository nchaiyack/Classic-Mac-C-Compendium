/*
 * Modal Dialog 3
 *
 * This dialog consists of:
 * - A Dismiss pushbutton for dismissing the dialog.
 * - Three pushbuttons, Button 1, Button 2, and Button 3, any
 * of which may be selected as the default button.  It is also
 * possible for none of the buttons to be selected.
 * - Four radio buttons used to select which of Button[1-3]
 * is the default, or that none of them is.
 * - Three checkboxes for activating or deactivating Button[1-3].
 * - A couple of static text items for titling the radio button
 * and check box sets.
 * - A user item that's used for drawing the outline around the
 * default button.
 *
 * This dialog demonstrates:
 * - How to associate an outlining function with an arbitrary button.
 * - How to change which button is the default, including how to move
 * the outlining function from one button to another.
 * - How to make sure the outline is redrawn properly when the hiliting
 * state of the outlined button changes.
 *
 * The routines that associate the outline item with buttons or dissociate
 * the item from buttons are written in a more modular fashion than for
 * Dialog 2.  They're also written more generally, since they must handle
 * the case where the default button can be set explicitly to "none".
 */

# include	"TransSkel.h"

# include	"Button.h"


typedef enum
{
	iPushDismiss = 1,
	iPushButton1,
	iPushButton2,
	iPushButton3,
	iRadioStaticText,
	iRadioButton1,
	iRadioButton2,
	iRadioButton3,
	iRadioNone,
	iCheckStaticText,
	iCheckButton1,
	iCheckButton2,
	iCheckButton3,
	iOutline
};


static short		defaultButton = 0;


static pascal void OutlineButton (DialogPtr dlog, short item);


/*
 * Set up a variable to point to the drawing procedure.  For 68K code this
 * is just a direct pointers to OutlineButton().  For PowerPC code it is a
 * routine descriptor into which is stuffed the address of OutlineButton().
 */

# if skelPPC		/* PowerPC code */

static RoutineDescriptor	drawDesc =
		BUILD_ROUTINE_DESCRIPTOR(uppUserItemProcInfo, OutlineButton);
static UserItemUPP	drawProc = (UserItemUPP) &drawDesc;

# else				/* 68K code */

static UserItemUPP	drawProc = OutlineButton;

# endif


/*
 * Draw heavy outline around default dialog button.  This function is
 * associated with the user item when there is a default button, and
 * is called by ModalDialog() when the outline needs redrawing.
 */

static pascal void
OutlineButton (DialogPtr dlog, short item)
{
	SkelDrawButtonOutline (SkelGetDlogCtl (dlog, defaultButton));
}


/*
 * Install outlining procedure to associate outline user item with
 * the given button and draw the outline item immediately.
 */

static void
InstallOutliner (DialogPtr dlog, short item)
{
Rect	r;

	SkelGetDlogRect (dlog, item, &r);			/* get button rect */
	InsetRect (&r, -4, -4);						/* expand it */
	SkelSetDlogRect (dlog, iOutline, &r);		/* use for outline item */
	SkelSetDlogProc (dlog, iOutline, drawProc);
	SkelDrawButtonOutline (SkelGetDlogCtl (dlog, defaultButton));
}


/*
 * Remove outlining procedure from current default and erase the
 * current outline.
 */

static void
RemoveOutliner (DialogPtr dlog)
{
	SkelSetDlogProc (dlog, iOutline, nil);
	SkelEraseButtonOutline (SkelGetDlogCtl (dlog, defaultButton));
}


/*
 * Set up button given by item number as default button:
 * - Remove the outliner for the current default, if there is one.
 * - Install an outliner for the new default, if there is one.
 */

static void
SetDefaultButton (DialogPtr dlog, short item)
{
	if (defaultButton != 0)
		RemoveOutliner (dlog);
	defaultButton = item;
	if (defaultButton != 0)
		InstallOutliner (dlog, defaultButton);
}


void
DoModal3 (void)
{
ModalFilterUPP	filter;
DialogPtr	dlog;
GrafPtr	savePort;
short	item;
short	value;
short	hilite;
short	loop;

	dlog = GetNewDialog (modal3Res, nil, (WindowPtr) -1L);
	if (dlog == (DialogPtr) nil)
	{
		SysBeep (1);
		return;
	}

	SkelPositionWindow (dlog, skelPositionOnMainDevice, horizRatio, vertRatio);

	GetPort (&savePort);
	SetPort (dlog);

	SetDefaultButton (dlog, iPushButton1);
	SkelSetDlogCtlValue (dlog, iCheckButton1, 1);
	SkelSetDlogCtlValue (dlog, iCheckButton2, 1);
	SkelSetDlogCtlValue (dlog, iCheckButton3, 1);
	SkelSetDlogRadioButtonSet (dlog, iRadioButton1, iRadioNone, iRadioButton1);

	ShowWindow (dlog);

	loop = 1;
	while (loop)
	{
		/*
		 * Get the standard filter and specify the default button so it will
		 * map return/enter.  If the default is zero, mapping is turned off.
		 *
		 * Note: the mapping causes the default button to flash when return/enter
		 * are typed, but the dialog isn't dismissed until Dismiss is clicked,
		 * which is unlike normal dialog operation.
		 */
		filter = SkelDlogFilter (nil, false);
		SkelDlogDefaultItem (defaultButton);	/* turns off if zero */
		ModalDialog (filter, &item);
		SkelRmveDlogFilter ();

		switch (item)
		{
		case iPushDismiss:
			loop = 0;
			break;
		case iPushButton1:		/* ignore hits in these items */
		case iPushButton2:
		case iPushButton3:
			break;
		case iRadioButton1:
		case iRadioButton2:
		case iRadioButton3:
			SkelSetDlogRadioButtonSet (dlog, iRadioButton1, iRadioNone, item);
			/* remap item number from radio button range into pushbutton range */
			item += iPushButton1 - iRadioButton1;
			SetDefaultButton (dlog, item);
			break;
		case iRadioNone:
			SkelSetDlogRadioButtonSet (dlog, iRadioButton1, iRadioNone, item);
			SetDefaultButton (dlog, 0);	/* no default button */
			break;
		case iCheckButton1:
		case iCheckButton2:
		case iCheckButton3:
			value = SkelToggleDlogCtlValue (dlog, item);
			hilite = (value ? normalHilite : dimHilite);
			/* remap item number from checkbox range into pushbutton range */
			item += iPushButton1 - iCheckButton1;
			/*
			 * Set button's hiliting state to make it active or inactive.
			 * If the new state is different than previous and the button
			 * is the default, redraw the outline.
			 */
			if (SkelSetDlogCtlHilite (dlog, item, hilite) && item == defaultButton)
				SkelDrawButtonOutline (SkelGetDlogCtl (dlog, item));
			break;
		}
	}
	DisposeDialog (dlog);
	SetPort (savePort);
}
