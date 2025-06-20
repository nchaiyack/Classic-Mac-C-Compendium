/*
 * TransSkel multiple-window demonstration: TextEdit module
 *
 * This module handles a simple TextEdit window, in which text may be
 * typed and standard Cut/Copy/Paste/Clear operations may be performed.
 * Undo is not supported, nor is text scrolling.
 *
 * 21 Apr 88 Paul DuBois
 * 29 Jan 89
 * - Conversion for TransSkel 2.0.
 * 12 Jan 91
 * - Conversion for TransSkel 3.0.
 */

# include	"TransSkel.h"

# include	"MultiSkel.h"


/* Edit menu item numbers */

typedef enum {
	undo = 1,
	/* --- */
	cut = 3,
	copy,
	paste,
	clear
} editItems;


static TEHandle		teEdit;		/* handle to text window TextEdit record */


static pascal void
Mouse (Point pt, long t, short mods)
{
	TEClick (pt, (Boolean) ((mods & shiftKey) != 0), teEdit);
}


static pascal void
Key (short c, short code, short mods)
{
	TEKey (c, teEdit);
}


/*
 * Update text window.  The update event might be in response to a
 * window resizing.  If so, resize the rects and recalc the linestarts
 * of the text.  To resize the rects, only the right edge of the
 * destRect need be changed (the bottom is not used, and the left and
 * top should not be changed). The viewRect should be sized to the
 * screen.
 */

static pascal void
Update (Boolean resized)
{
Rect	r;

	r = editWind->portRect;
	EraseRect (&r);
	r.left += 4;
	r.bottom -= 2;
	r.top += 2;
	r.right -= 19;
	if (resized)
	{
		(**teEdit).destRect.right = r.right;
		(**teEdit).viewRect = r;
		TECalText (teEdit);
	}
	DrawGrowBox (editWind);
	TEUpdate (&r, teEdit);
}


static pascal void
Activate (Boolean active)
{
	DrawGrowBox (editWind);
	if (active)
	{
		TEActivate (teEdit);
		DisableItem (editMenu, undo);
	}
	else
	{
		TEDeactivate (teEdit);
		EnableItem (editMenu, undo);
	}
}


static pascal void
Clobber (void)
{
	TEDispose (teEdit);
	DisposeWindow (editWind);
}


static pascal void
Idle (void)
{
	TEIdle (teEdit);	/* blink that cursor! */
}


void EditWindInit (void)
{
Rect	r;
StringPtr	str;

	if (SkelQuery (skelQHasColorQD))
		editWind = GetNewCWindow (editWindRes, nil, (WindowPtr) -1L);
	else
		editWind = GetNewWindow (editWindRes, nil, (WindowPtr) -1L);
	if (editWind == (WindowPtr) nil)
		return;
	(void) SkelWindow (editWind,
				Mouse,		/* handle mouse-clicks */
				Key,		/* Key keyclicks */
				Update,
				Activate,
				nil,		/* no close proc */
				Clobber,	/* disposal proc */
				Idle,		/* idle proc */
				true);

	TextFont (0);
	TextSize (0);

	r = editWind->portRect;
	r.left += 4;
	r.bottom -= 2;
	r.top += 2;
	r.right -= 19;
	teEdit = TENew (&r, &r);
	str = (StringPtr) "\pThis is the text editing window.\r";
	TEInsert (&str[1], (long) str[0], teEdit);
}


/*
 * Handle Edit menu items for text window
 */

pascal void
EditWindEditMenu (short item)
{
	switch (item)
	{
		/*
		 * cut selection, put in TE Scrap, clear clipboard and put
		 * TE scrap in it
		 */
		case cut:
			TECut (teEdit);
			(void) ZeroScrap ();
			(void) TEToScrap ();
			break;

		/*
		 * copy selection to TE Scrap, clear clipboard and put
		 * TE scrap in it
		 */
		case copy:
			TECopy (teEdit);
			(void) ZeroScrap ();
			(void) TEToScrap ();
			break;

		/*
		 * get clipboard into TE scrap, put TE scrap into edit record
		 */
		case paste:
			(void) TEFromScrap ();
			TEPaste (teEdit);
			break;

		/*
		 * delete selection without putting into TE scrap or clipboard
		 */
		case clear:
			TEDelete (teEdit);
			break;
	}
}
