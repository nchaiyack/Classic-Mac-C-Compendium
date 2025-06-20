/*
 * SkelDlogMapKeyToButton ()
 *
 * This routine looks at an event to see whether or not it's a key event
 * that should be mapped onto an item hit in a dialog's default or cancel
 * button.
 *
 * defaultItem is 0 if no return/enter mapping should be done, > 0 to
 * explicitly specify the item that return/enter map to, < 0 to map
 * return/enter to the item specified as the default in the dialog record.
 *
 * cancelItem is 0 if no escape/cmd-period mapping should be done, > 0 to
 * explicitly specify the item that escape/cmd-period map to.
 *
 * If the key maps to a button item, the button is flashed for visual
 * feedback if it is hilited normally (not dimmed).  If the button is enabled,
 * the item parameter is set to the item number and the function returns true.
 * If the key doesn't map to an item or the button is disabled, the function
 * returns false.
 *
 * If the key maps to a button, but the button isn't hilited properly or is
 * disabled, the event is mapped to a null event so that nothing else is done
 * with it.  This is done based on the assumption that if the caller is trying
 * to do key mapping, it doesn't want the mapped keys to get into dialog edit
 * text items.
 */

# include	"TransSkel.h"


# define	returnKey	13
# define	enterKey	3
# define	escapeKey	27


# define	normalHilite	0
# define	dimHilite		255


/*
 * Function that checks whether a dialog item is a button.  If so,
 * flash it.  In addition, if it's active, return true to indicate an
 * item hit.  Otherwise return false.
 */

static Boolean
TestDlogButton (DialogPtr d, short item)
{
ControlHandle	ctrl;
Handle	itemHandle;
short	itemType;
Rect	itemRect;

	GetDItem (d, item, &itemType, &itemHandle, &itemRect);
	if ((itemType & (ctrlItem + btnCtrl)) == (ctrlItem + btnCtrl))
	{
		ctrl = (ControlHandle) itemHandle;
		if ((**ctrl).contrlHilite == normalHilite)
		{
			SkelFlashButton (ctrl);
			if ((itemType & itemDisable) == 0)
				return (true);
		}
	}
	return (false);
}


pascal Boolean
SkelDlogMapKeyToButton (DialogPtr dlog, EventRecord *evt, short *item,
								short defaultItem, short cancelItem)
{
char	c;
short	i;

	c = evt->message & charCodeMask;
	if (c == returnKey || c == enterKey)
	{
		i = defaultItem;
		if (i != 0)
		{
			if (i < 0)
				i = ((DialogPeek) dlog)->aDefItem;
			if (TestDlogButton (dlog, i))
			{
				*item = i;
				return (true);
			}
			evt->what = nullEvent;
		}
	}
	if (c == escapeKey || SkelCmdPeriod (evt))
	{
		*item = cancelItem;
		if (*item > 0)
		{
			if (TestDlogButton (dlog, *item))
				return (true);
			evt->what = nullEvent;
		}
	}
	return (false);
}
