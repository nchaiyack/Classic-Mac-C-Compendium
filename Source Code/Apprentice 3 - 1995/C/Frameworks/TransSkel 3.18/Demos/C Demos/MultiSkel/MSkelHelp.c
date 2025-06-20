/*
 * TransSkel multiple-window demonstration: Help module
 * 
 * This module handles a help window, in which text may be scrolled but
 * not edited.  A TextEdit record is used to hold the text, though.
 * 
 * 21 Apr 88 Paul DuBois
 * 29 Jan 89
 * - Conversion for TransSkel 2.0.
 * 12 Jan 91
 * - Conversion for TransSkel 3.0.
 */

# include	"TransSkel.h"

# include	"MultiSkel.h"


static TEHandle			teHelp;		/* handle to help window TextEdit record */
static ControlHandle	helpScroll;	/* help window scroll bar */
static short			helpLine;	/* line currently at top of window */
static short			halfPage;	/* number of lines in half a window */


/*
 * Scroll to the correct position.  lDelta is the
 * amount to CHANGE the current scroll setting by.
 */

static void
DoScroll (short lDelta)
{
short	newLine;

	newLine = helpLine + lDelta;
	if (newLine < 0)
		newLine = 0;
	if (newLine > GetCtlMax (helpScroll))
		newLine = GetCtlMax (helpScroll);
	SetCtlValue (helpScroll, newLine);
	lDelta = (helpLine - newLine ) * (**teHelp).lineHeight;
	TEScroll (0, lDelta, teHelp);
	helpLine = newLine;
}


/*
 * Filter proc for tracking mousedown in scroll bar.  The part code
 * of the part originally hit is stored as the control's reference
 * value.
 *
 * The "void" had better be there!  Otherwise the compiler will treat
 * it as an integer function, not a procedure.
 */

static pascal void
TrackScroll (ControlHandle theScroll, short partCode)
{
short			lDelta;

	if (partCode == GetCRefCon (theScroll))	/* still in same part? */
	{
		switch (partCode)
		{
			case inUpButton: lDelta = -1; break;
			case inDownButton: lDelta = 1; break;
			case inPageUp: lDelta = -halfPage; break;
			case inPageDown: lDelta = halfPage; break;
		}
		DoScroll (lDelta);
	}
}


/* Compatibility machinery for non-universal header compilation */

# if !skelUnivHeaders

typedef pascal void (*ControlActionProcPtr)(ControlHandle theControl, short partCode);
typedef ControlActionProcPtr ControlActionUPP;

# endif

/*
 * Set up a variable to point to the scroll tracking procedure.  For 68K code this
 * is just a direct pointer to TrackScroll().  For PowerPC code it is a
 * routine descriptor into which the address of TrackScroll() is stuffed.
 */

# if skelPPC		/* PowerPC code */

static RoutineDescriptor	trackDesc =
		BUILD_ROUTINE_DESCRIPTOR(uppControlActionProcInfo, TrackScroll);
static ControlActionUPP	trackProc = (ControlActionUPP) &trackDesc;

# else				/* 68K code */

static ControlActionUPP	trackProc = TrackScroll;

# endif


/*
 * Handle hits in scroll bar
 */

static pascal void
Mouse (Point pt, long t, short mods)
{
short	thePart;

		if ((thePart = TestControl (helpScroll, pt)) == inThumb)
		{
			(void) TrackControl (helpScroll, pt, nil);
			DoScroll (GetCtlValue (helpScroll) - helpLine);
		}
		else if (thePart != 0)
		{
			SetCRefCon (helpScroll, (long) thePart);
			(void) TrackControl (helpScroll, pt, trackProc);
		}
}


/*
 * Update help window.  The update event might be in response to a
 * window resizing.  If so, resize the rects and recalc the linestarts
 * of the text.  To resize the rects, only the right edge of the
 * destRect need be changed (the bottom is not used, and the left and
 * top should not be changed). The viewRect should be sized to the
 * screen.  Pull text down if necessary to fill window.
 */

static pascal void
Update (Boolean resized)
{
Rect	r;
short	visLines;
short	lHeight;
short	topLines;
short	nLines;
short	scrollLines;

	r = helpWind->portRect;
	EraseRect (&r);
	if (resized)
	{
		r.left += 4;
		r.bottom -= 2;
		r.top += 2;
		r.right -= 19;
		(**teHelp).destRect.right = r.right;
		(**teHelp).viewRect = r;
		TECalText (teHelp);
		lHeight = (**teHelp).lineHeight;
		nLines = (**teHelp).nLines;
		visLines = (r.bottom - r.top) / lHeight;
		halfPage = visLines / 2;
		topLines = (r.top - (**teHelp).destRect.top) / lHeight;
		scrollLines = visLines - (nLines - topLines);
		if (scrollLines > 0 && topLines > 0)
		{
			if (scrollLines > topLines)
				scrollLines = topLines;
			TEScroll (0, scrollLines * lHeight, teHelp);
		}
		scrollLines = nLines - visLines;
		helpLine = (r.top - (**teHelp).destRect.top) / lHeight;

		/*
		 * Move and resize the scroll bar as well.  The ValidRect call is done
		 * because the HideControl adds the control bounds box to the update
		 * region - which would generate another update event!  Since everything
		 * gets redrawn below, the ValidRect is used to cancel the update.
		 */

		HideControl (helpScroll);
		r = helpWind->portRect;
		r.left = r.right - 15;
		r.bottom -= 14;
		--r.top;
		++r.right;
		SizeControl (helpScroll, r.right - r.left, r.bottom - r.top);
		MoveControl (helpScroll, r.left, r.top);
		SetCtlMax (helpScroll, nLines - visLines < 0 ? 0 : nLines - visLines);
		SetCtlValue (helpScroll, helpLine);
		/*if (scrollLines <= 0)
			HiliteControl (helpScroll, (scrollLines > 0 ? 0 : 255));*/
		ShowControl (helpScroll);
		/*if (GetCtlValue (helpScroll) > scrollLines)
			DoScroll (GetCtlValue (helpScroll) - scrollLines);*/
	}
	DrawGrowBox (helpWind);
	DrawControls (helpWind);	/* redraw scroll bar */
	r = (**teHelp).viewRect;
	TEUpdate (&r, teHelp);		/* redraw text display */
	ValidRect (&helpWind->portRect);
}


/*
 * When the window comes active, disable the Edit menu and highlight
 * the scroll bar if there are any lines not visible in the content
 * region.  When the window is deactivated, enable the Edit menu and
 * un-highlight the scroll bar.
 */

static pascal void
Activate (Boolean active)
{
	DrawGrowBox (helpWind);
	if (active)
	{
		DisableItem (editMenu, 0);
		HiliteControl (helpScroll,
					(GetCtlMax (helpScroll) > 0 ? normalHilite : dimHilite));
	}
	else
	{
		EnableItem (editMenu, 0);
		HiliteControl (helpScroll, dimHilite);
	}
	DrawMenuBar ();
}


static pascal void
Clobber (void)
{
	TEDispose (teHelp);
	DisposeControl (helpScroll);
	DisposeWindow (helpWind);
}


void
HelpWindInit (void)
{
Rect	r;
Handle	textHandle;
short	visLines;
short	scrollLines;

	if (SkelQuery (skelQHasColorQD))
		helpWind = GetNewCWindow (helpWindRes, nil, (WindowPtr) -1L);
	else
		helpWind = GetNewWindow (helpWindRes, nil, (WindowPtr) -1L);
	if (helpWind == (WindowPtr) nil)
		return;
	(void) SkelWindow (helpWind,
				Mouse,		/* handle clicks in scrollbar */
				nil,		/* ignore keyclicks */
				Update,
				Activate,
				nil,		/* no close proc */
				Clobber,	/* disposal proc */
				nil,		/* no idle proc */
				false);

	TextFont (0);
	TextSize (0);

	r = helpWind->portRect;
	r.left += 4;
	r.bottom -= 2;
	r.top += 2;
	r.right -= 19;
	teHelp = TENew (&r, &r);
	textHandle = GetResource ('TEXT', helpTextRes);	/* read help text */
	HLock (textHandle);		/* lock it and insert into TERec */
	TEInsert (*textHandle, GetHandleSize (textHandle), teHelp);
	HUnlock (textHandle);
	ReleaseResource (textHandle);	/* done with it, so goodbye */
	/*
	 * Now figure out how many lines will fit in the window and how many
	 * will not.  Determine the number of lines in half a window for use
	 * in tracking clicks in the page up and page down regions of the
	 * scroll bar.  Then create the scroll bar .  Make sure the borders
	 * overlap the window frame and the frame of the grow box.
	 */
	visLines = (r.bottom - r.top) / (**teHelp).lineHeight;
	scrollLines = (**teHelp).nLines - visLines;
	halfPage = visLines / 2;
	helpLine = 0;
	r = helpWind->portRect;
	r.left = r.right - 15;
	r.bottom -= 14;
	--r.top;
	++r.right;
	/*
	 * Build the scroll bar.  Don't need to bother testing whether to
	 * highlight it or not, since that will be done in response to the
	 * activate event.
	 */
	helpScroll = NewControl (helpWind, &r, "\p", true,
					helpLine, 0, scrollLines, scrollBarProc, 0L);

	/*
	 * GetNewWindow generates an update event for entire portRect.
	 * Cancel it, since the everything has been drawn already,
	 * except for the grow box (which will be drawn in response
	 * to the activate event).
	 */
	ValidRect (&helpWind->portRect);
}
