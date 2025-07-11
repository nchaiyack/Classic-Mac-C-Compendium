/*
 * Modal Dialog 2
 *
 * This dialog consists of an edittext item for entering a search
 * string, a statictext item for titling the edittext item, a Cancel
 * button, a Find button, and a user item for outlining the default
 * button.
 * 
 * The Cancel button is the default when the edittext item is empty.
 * The Find button is the default when the edittext item contains text.
 *
 * This dialog demonstrates:
 * - How to associate an outlining function with an arbitrary button.
 * - How to change which button is the default, which involves moving
 * the outlining function from one button to another.
 */

# include	"TransSkel.h"

# include	"Button.h"


typedef enum
{
	findItem = 1,
	cancelItem,
	staticTextItem,
	editTextItem,
	outlineItem
};


static short	defaultButton = 0;


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
 * Set up button given by item number as default button.
 * Install an outlining procedure to associate outline user item with
 * the given button and draw the outline item immediately.
 */

static void
SetDefaultButton (DialogPtr dlog, short item)
{
Rect	r;

	defaultButton = item;
	SkelGetDlogRect (dlog, defaultButton, &r);	/* get button rect */
	InsetRect (&r, -4, -4);						/* expand it */
	SkelSetDlogRect (dlog, outlineItem, &r);	/* use for outline item */
	SkelSetDlogProc (dlog, outlineItem, drawProc);
	SkelDrawButtonOutline (SkelGetDlogCtl (dlog, defaultButton));
}


void
DoModal2 (void)
{
ModalFilterUPP	filter;
DialogPtr	dlog;
GrafPtr	savePort;
short	item;
short	newDefault;
Str255	str;

	dlog = GetNewDialog (modal2Res, nil, (WindowPtr) -1L);
	if (dlog == (DialogPtr) nil)
	{
		SysBeep (1);
		return;
	}

	SkelPositionWindow (dlog, skelPositionOnMainDevice, horizRatio, vertRatio);

	GetPort (&savePort);
	SetPort (dlog);

	SetDefaultButton (dlog, cancelItem);
 
	ShowWindow (dlog);

	for (;;)
	{
		/*
		 * Get the standard filter and specify the default button so it will
		 * map return/enter.  Map escape and command-period onto the cancel
		 * button.
		 */
		filter = SkelDlogFilter (nil, false);
		SkelDlogDefaultItem (defaultButton);
		SkelDlogCancelItem (cancelItem);
		SkelDlogTracksCursor (true);
		ModalDialog (filter, &item);
		SkelRmveDlogFilter ();

		if (item == findItem || item == cancelItem)
			break;

		/*
		 * Select Find button as default if editText item is
		 * non-empty.  Select Cancel button otherwise.  But
		 * don't actually do anything if the default button's
		 * already set correctly.
		 */

		SkelGetDlogStr (dlog, editTextItem, str);
		newDefault = (str[0] > 0 ? findItem : cancelItem);
		if (newDefault != defaultButton)
		{
			SkelEraseButtonOutline (SkelGetDlogCtl (dlog, defaultButton));
			SetDefaultButton (dlog, newDefault);
		}
	}
	DisposeDialog (dlog);
	SetPort (savePort);
}
