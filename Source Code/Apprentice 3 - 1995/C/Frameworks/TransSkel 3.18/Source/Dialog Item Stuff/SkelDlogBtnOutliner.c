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
 * If you set the default button's hiliting state, you should redraw the outline
 * as necessary to match the button.  (This can be accomplished by by invalidating
 * the outline item's bounding rectangle.)  You can avoid unnecessary redrawing by
 * drawing only when the button hiliting actually changes, like this:
 *
 *	if (SkelSetDlogCtlHilite (dlog, buttonItem, newHilite)
 *	{
 *		SkelGetDlogRect (dlog, outlineItem, &r);
 *		InvalRect (&r);
 *	}
 *
 * SkelSetDlogCtlHilite() returns true only if the control hiliting *changes*.
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


/*
 * For 68K a pointer to the drawing procedure DrawDlogButtonOutline() can
 * be passed directly to SetDItem().  For PowerPC code a pointer to a
 * routine descriptor must be passed instead.  The descriptor is allocated
 * statically since it doesn't have to be deallocated.  (There's no way to
 * tell when it would be safe to deallocate it anyway.)
 */

# if skelPPC		/* PowerPC code */
static RoutineDescriptor	drawDesc =
		BUILD_ROUTINE_DESCRIPTOR(uppUserItemProcInfo, DrawDlogButtonOutline);
# endif


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
# if skelPPC		/* PowerPC code */
	SetDItem (d, item, type, (Handle) &drawDesc, &r);
# else				/* 68K code */
	SetDItem (d, item, type, (Handle) DrawDlogButtonOutline, &r);
# endif
}
