/*
 * Associate a button-outlining function with the given item, which should
 * be a user item.  The item outlined will be the default item, and should
 * be a push button.  The user item bounding rectangle is positioned and
 * sized to surround the default item.
 *
 * There's a subtle point here -- the outline drawing proc is called when the
 * user item rect is becomes invalid, but the drawing proc bases its calculations
 * on the rect for the default button item.  This works because the rect it
 * calculates based on the button rect is identical to that of the user item.
 *
 * If you change the button's hiliting state, you should make sure the outline
 * is redrawn by invalidating its bounding rectangle.  You can avoid unnecessary
 * redrawing by using SkelSetDlogCtlHilite() like so:
 *
 *	if (SkelSetDlogCtlHilite (dlog, buttonItem, newHilite)
 *	{
 *		SkelGetDlogRect (dlog, outlineItem, &r);
 *		InvalRect (&r);
 *	}
 */

# include	"TransSkel.h"


/*
 * Draw heavy outline around default dialog button.
 */

static pascal void
DrawDlogButtonOutline (DialogPtr d, short item)
{
	SkelDrawButtonOutline (SkelGetDlogCtl (d, ((DialogPeek) d)->aDefItem));
}


pascal void
SkelSetDlogButtonOutliner (DialogPtr d, short item)
{
short	type;
Handle	h;
Rect	r, rJunk;
short	defItem;

	/* find default item bounding rectangle */
	defItem = ((DialogPeek) d)->aDefItem;
	GetDItem (d, defItem, &type, &h, &r);

	/* get user item, position rectangle, and install draw proc using it */
	GetDItem (d, item, &type, &h, &rJunk);
	InsetRect (&r, -4, -4);
	SetDItem (d, item, type, (Handle) DrawDlogButtonOutline, &r);
}
