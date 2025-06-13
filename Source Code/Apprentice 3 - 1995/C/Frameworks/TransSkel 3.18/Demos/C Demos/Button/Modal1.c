/*
 * Modal Dialog 1
 *
 * This dialog consists of an OK button that's always the default button,
 * a Cancel button, an edittext item, and a user item that's used for
 * drawing the outline around the default button.
 *
 * The dialog arranges that the OK button is active only when there is text
 * in the edittext item.  The Cancel button is always active so the user
 * can get out of the dialog even when there's no text in the edittext item.
 *
 * This dialog demonstrates:
 * - How to install an outliner for the default item, where the default is
 * the button that's specified in the dialog template as the default.
 * - How to make sure the outline is redrawn properly when the hiliting
 * state of the outlined button changes.
 */

# include	"TransSkel.h"

# include	"Button.h"


typedef enum
{
	iOK = 1,
	iCancel,
	iEditText,
	iOutline
};



void
DoModal1 (void)
{
ModalFilterUPP	filter;
DialogPtr	dlog;
GrafPtr	savePort;
short	item;
Str255	str;

	dlog = GetNewDialog (modal1Res, nil, (WindowPtr) -1L);
	if (dlog == (DialogPtr) nil)
	{
		SysBeep (1);
		return;
	}

	SkelPositionWindow (dlog, skelPositionOnMainDevice, horizRatio, vertRatio);

	GetPort (&savePort);
	SetPort (dlog);

	SkelSetDlogButtonOutliner (dlog, iOutline);
	SkelSetDlogStr (dlog, iEditText,
		"\pDefault button is active only when this field is non-empty");
	SelIText (dlog, iEditText, 0, 32767);

	ShowWindow (dlog);

	for (;;)
	{
		/*
		 * Tell the standard filter to map return/enter to the default
		 * item (iOK).  Also map escape/command-period to Cancel.
		 */
		filter = SkelDlogFilter (nil, true);
		SkelDlogCancelItem (iCancel);
		SkelDlogTracksCursor (true);
		ModalDialog (filter, &item);
		SkelRmveDlogFilter ();
		if (item == iOK || item == iCancel)
			break;

		/*
		 * Set hiliting of default button according to whether or not
		 * the edittext item now has anything in it.  If hiliting changes,
		 * redraw outline.
		 */
		SkelGetDlogStr (dlog, iEditText, str);
		if (SkelSetDlogCtlHilite (dlog, iOK, str[0] > 0 ? normalHilite : dimHilite))
			SkelDrawButtonOutline (SkelGetDlogCtl (dlog, iOK));
	}
	DisposeDialog (dlog);
	SetPort (savePort);
}
