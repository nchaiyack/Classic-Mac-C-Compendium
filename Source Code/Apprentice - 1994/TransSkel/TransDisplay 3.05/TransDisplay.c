/*
 * Use SetScrollValue() in preference to SetCtlValue() when setting scroll
 */

/*
 * TransDisplay version 3.05 - TransSkel plug-in module supporting
 * an arbitrary number of generic display windows with memory.
 *
 * TransSkel and TransDisplay are public domain.  For more information,
 * contact:
 *
 * 			Paul DuBois
 * 			Wisconsin Regional Primate Research Center
 * 			1220 Capitol Court
 * 			Madison, WI  53715-1299  USA
 *
 * Internet:	dubois@primate.wisc.edu
 *
 *
 * This version of TransDisplay is written for THINK C 6.0.
 * THINK C is a trademark of:
 *
 * 			Symantec Corporation
 * 			10201 Torre Avenue
 * 			Cupertino, CA 95014  USA
 *
 * History
 * 08/25/86	Genesis.  Beta version.
 *
 * 09/15/86 Release 1.0
 * Changed to allow arbitrary number of windows.
 *
 * 01/17/87	Release 1.01
 * The window type when a new window is created with NewDWindow is
 * documentProc+8 now, so that the window will have a zoom box on a
 * machine with 128K ROMS.
 *
 * 01/29/89	Release 2.0
 * Converted to work with TransSkel 2.0.  Display window creation routines
 * now check whether window and window handler creation succeeded and
 * return nil if not.  2-byte and 4-byte integer types are typedef'ed to
 * Integer and LongInt to ease porting.
 *
 * 24 Dec 90 Release 3.01
 * - Started converting to work with TransSkel 3.
 * - Began adding prototypes.
 * 01 Jan 91
 * - Took out the Macintosh header file #includes.  <MacHeaders> is assumed
 * to be turned on so that it's not necessary.
 * 06 Jun 92
 * - Fixed GetDWindowTE() bug.  Was returning nil under multiwindow
 * compilation.
 *
 * 05 Jun 93 Release 3.02
 * - Conversion for THINK C 6.0.
 *
 * 07 Jun 93 Release 3.03
 * - Took out all the stuff to allow compiling to handle only a single
 * display window.  The savings in bytes of object code is no longer worth
 * the extra source code complexity.  It's also unnecessary because I no longer
 * maintain a window list, since I ...
 * - Reimplemented linked list holding display window data using TransSkel's
 * window property functions (new in TS 3.00).  The property type is
 * skelWPropDisplayWind, and the data value is a handle to the window data
 * structure.
 * - Took out all the "register" declarations.  The compiler's smart enough
 * now that they don't make any difference, so they're just clutter.
 * 05 Jul 93
 * - Fixed Activate() and Update() so that when a window goes inactive,
 * the scroll bar is hidden (and only the frame drawn).  Previously, the
 * scroll bar was just made inactive, which doesn't quite conform to the user
 * interface guidelines.
 * 17 Oct 93
 * - Added DisplayOSType() function.
 * 25 Nov 93
 * - Tests to check whether a window is the active window now test w->hilited
 * rather than FrontWindow(), since the front window of a suspended application
 * isn't active.
 * - Changed GetDWindow() so the WindowPtr is returned as the function value
 * rather than in the argument.
 * 21 Dec 93
 * - Use color grafports when available.
 * 04 Jan 94
 * - Undid Integer/LongInt type stuff back to short/long.
 *
 * 18 Jan 94 Release 3.04
 * - DisplayInt() and DisplayHexInt() are problematic since the THINK C can
 * treat int's as either 2 or 4 bytes.  Added new routines DisplayShort()
 * and DisplayHexShort() which are unambiguous for 2 byte integers.
 * DisplayInt() and DisplayHexInt() are now deprecated and will disappear
 * in the future.
 * - New routine DisplayCString() for C-style strings.
 * 19 Jan 94
 * - Window creation routines do better checks for allocation failures.
 * - Much rewriting to eliminate many global variables.  No more SyncGlobals().
 *
 * 20 Feb 94 Release 3.05
 * - Updated for TransSkel 3.11.
 * - Converted interface to be Pascal-compatible.
 */


# include	"TransSkel.h"

# include	"TransDisplay.h"


# define	normalHilite	0
# define	dimHilite		255

# define	WindowIsActive(w)	((WindowPeek) (w))->hilited


/*
 * New(TypeName) returns handle to new object, for any TypeName.
 * If there is insufficient memory, the result is nil.
 */

# define	New(type)	(type **) NewHandle ((Size) sizeof (type))


/*
 * Default values for display window characteristics.  Used when
 * new display windows are created.
 */

static short	d_font = monaco;		/* default font              */
static short	d_size = 9;				/* default pointsize         */
static short	d_wrap = 0;				/* default word wrap (on)    */
static short	d_just = teJustLeft;	/* default justification     */
static long	d_maxText = 30000L;		/* default max text allowed  */
static long	d_flushAmt = 25000L;	/* default autoflush amount  */
static TDispActivateProcPtr		d_activate = (TDispActivateProcPtr) nil;	/* default notification proc */


/*
 * Lowest allowable values for autoflush characteristics
 */

# define	d_loMaxText		(100L)
# define	d_loFlushAmt	(100L)


/*
 * Display window document record.  A handle to a window's document
 * record is stored in the window's property list.
 */

typedef struct DocRecord DocRecord, *DocPtr, **DocHandle;

struct DocRecord
{
	WindowPtr				dWind;		/* display window         */
	TEHandle				dTE;		/* window text            */
	ControlHandle			dScroll;	/* window scroll bar      */
	TDispActivateProcPtr	dActivate;	/* notification procedure */
	long					dMaxText;	/* max text length        */
	long					dFlushAmt;	/* amount to autoflush    */
};


/*
 * Macros for accessing parts of a document record, given a document
 * handle.
 */

# define	DocWind(doc)			((**doc).dWind)
# define	DocTE(doc)				((**doc).dTE)
# define	DocScroll(doc)			((**doc).dScroll)
# define	DocActivateProc(doc)	((**doc).dActivate)
# define	DocMaxText(doc)			((**doc).dMaxText)
# define	DocFlushAmt(doc)		((**doc).dFlushAmt)


/*
 * curDispWind is the current output window.
 * If curDispWind = nil, output is currently turned off.
 */

static WindowPtr	curDispWind = nil;



/* -------------------------------------------------------------------- */
/*				Miscellaneous Internal (private) Routines				*/
/* -------------------------------------------------------------------- */


/*
 * Draw grow box of dispWind in lower right hand corner
 */


static void
DrawGrowBox (WindowPtr w)
{
RgnHandle	oldClip;
Rect		r;

	r = w->portRect;
	r.left = r.right - 15;		/* draw only in corner */
	r.top = r.bottom - 15;
	oldClip = NewRgn ();
	GetClip (oldClip);
	ClipRect (&r);
	DrawGrowIcon (w);
	SetClip (oldClip);
	DisposeRgn (oldClip);
}



/* -------------------------------------------------------------------- */
/*			Lowest-level Internal (Private) Display Window Routines		*/
/* -------------------------------------------------------------------- */


/*
 * Get document handle associated with display window.
 * Return nil if window isn't a known display window.
 */

static DocHandle
WindDocHandle (WindowPtr w)
{
SkelWindPropHandle	ph;
DocHandle	doc = (DocHandle) nil;

	if (w != (WindowPtr) nil)
	{
		ph = SkelGetWindProp (w, skelWPropDisplayWind);
		if (ph != (SkelWindPropHandle) nil)
			doc = (DocHandle) (**ph).skelWPropData;
	}
	return (doc);
}


/*
 * Calculate the dimensions of the editing rectangle for
 * a window (which is assumed to be
 * the current port).  (The viewRect and destRect are the
 * same size.)  Assumes the port, text font and text size are all
 * set properly.  The viewRect is sized so that an integral
 * number of lines can be displayed in it, i.e., so that a
 * partial line never shows at the bottom.
 */

static void
CalcEditRect (WindowPtr w, Rect *r)
{
FontInfo	f;
short		lineHeight;

	GetFontInfo (&f);
	lineHeight = f.ascent + f.descent + f.leading;
	*r = w->portRect;
	r->left += 4;
	r->right -= 17;			/* leave room for scroll bar + 2 */
	r->top += 2;
	r->bottom = r->top + ((r->bottom - r->top - 2) / lineHeight) * lineHeight;
}


/*
 * Calculate the dimensions of the scroll bar rectangle for a
 * window.  Make sure that the edges overlap the window frame and
 * the grow box.
 */

static void
CalcScrollRect (WindowPtr w, Rect *r)
{
	*r = w->portRect;
	++r->right;
	--r->top;
	r->left = r->right - 16;
	r->bottom -= 14;
}


/*
 * Calculate the number of lines currently scrolled off
 * the top of an edit record.
 */

static short
LinesOffTop (TEHandle hTE)
{
	return (((**hTE).viewRect.top - (**hTE).destRect.top) / (**hTE).lineHeight);
}


/*
 * Highlight the scroll bar properly.  This means that it's not
 * made active if the window itself isn't active, even if
 * there's enough text to fill the window.
 */

static void
HiliteScroll (DocHandle doc)
{
WindowPtr	w;
ControlHandle	scroll;
short	hilite;

	w = DocWind (doc);
	scroll = DocScroll (doc);
	hilite = (WindowIsActive (w) && GetCtlMax (scroll) > 0
				? normalHilite : dimHilite);
	HiliteControl (scroll, hilite);
}


/*
 * Set scroll bar current value (but only if it's different than
 * the current value, to avoid needless flashing).
 */

static void
SetScrollValue (ControlHandle scroll, short value)
{
	if (GetCtlValue (scroll) != value)
		SetCtlValue (scroll, value);
}


/*
 * Scroll to the correct position.  lDelta is the
 * amount to CHANGE the current scroll setting by.
 * Positive scrolls the text up, negative down.
 */

static void
ScrollText (DocHandle doc, short lDelta)
{
ControlHandle	scroll;
TEHandle	hTE;
short	topVisLine;
short	newTopVisLine;

	scroll = DocScroll (doc);
	hTE = DocTE (doc);

	topVisLine = LinesOffTop (hTE);
    newTopVisLine = topVisLine + lDelta;
    if (newTopVisLine < 0)					/* clip to range */
    	newTopVisLine = 0;
    if (newTopVisLine > GetCtlMax (scroll))
		newTopVisLine = GetCtlMax (scroll);
    SetScrollValue (scroll, newTopVisLine);
    TEScroll (0, (topVisLine - newTopVisLine ) * (**hTE).lineHeight, hTE);
}


/*
 * Filter proc for tracking mousedown in scroll bar.
 *
 * Scroll by one line if the mouse is in an arrow.  Scroll by a half
 * window's worth of lines if the mouse is in a page region.
 *
 * TrackScroll() uses scrollDoc and scrollPart, which must be set in
 * Mouse() before calling TrackControl(), which calls TrackScroll().
 * scrollPart is the original part code in which the mousedown occurred.
 */

static DocHandle	scrollDoc;
static short		scrollPart;

static pascal void
TrackScroll (ControlHandle theScroll, short partCode)
{
TEHandle	hTE;
short	lDelta;
short	halfPage;

	hTE = DocTE (scrollDoc);
	if (partCode == scrollPart)			/* still in same part? */
	{
		halfPage = (((**hTE).viewRect.bottom - (**hTE).viewRect.top)
						/ (**hTE).lineHeight) / 2;
		if (halfPage == 0)
			++halfPage;
		switch (partCode)
		{
			case inUpButton: lDelta = -1; break;
			case inDownButton: lDelta = 1; break;
			case inPageUp: lDelta = -halfPage; break;
			case inPageDown: lDelta = halfPage; break;
		}
		ScrollText (scrollDoc, lDelta);
	}
}


/*
 * Overhaul the entire display.  This is called after catastrophic
 * events, such as resizing the window, or changes to the word
 * wrap style.  It makes sure the view and destination rectangles
 * are sized properly, and that the bottom line of text never
 * scrolls up past the bottom line of the window (if there's
 * enough to fill the window), and that the scroll bar max and
 * current values are set properly.
 *
 * Resizing the dest rect just means resetting the right edge
 * (the top is NOT reset), since text might be scrolled off the
 * top (i.e., destRect.top != 0).
 */

static void
OverhaulDisplay (DocHandle doc)
{
WindowPtr	w;
TEHandle	hTE;
ControlHandle	scroll;
Rect	r;
short	nLines;			/* # of lines in TERec */
short	visLines;		/* # of lines displayable in window */
short	topLines;		/* # of lines currently scrolled off top */
short	scrollLines;	/* # of lines to scroll down */
short	lHeight;

	w = DocWind (doc);
	hTE = DocTE (doc);
	scroll = DocScroll (doc);
	CalcEditRect (w, &r);
	(**hTE).destRect.right = r.right;
	(**hTE).viewRect = r;
	TECalText (hTE);		/* recalc line starts */
	lHeight = (**hTE).lineHeight;
	nLines = (**hTE).nLines;
	visLines = (r.bottom - r.top) / lHeight;
	topLines = LinesOffTop (hTE);

	/*
	 * If the text doesn't fill the window (visLines > nLines - topLines),
	 * pull the text down if possible (if topLines > 0).  Make sure not
	 * to try to scroll down by more lines than are hidden off the top.
	 */
	scrollLines = visLines - (nLines - topLines);
	if (scrollLines > 0 && topLines > 0)
	{
		if (scrollLines > topLines)
			scrollLines = topLines;
		TEScroll (0, scrollLines * lHeight, hTE);
		topLines -= scrollLines;
	}
	TEUpdate (&r, hTE);

	SetCtlMax (scroll, nLines - visLines < 0 ? 0 : nLines - visLines);
	SetCtlValue (scroll, topLines);
	HiliteScroll (doc);
}


/* ---------------------------------------------------------------- */
/*						Window Handler Routines						*/
/* ---------------------------------------------------------------- */


/*
 * Handle mouse clicks in window
 */

static pascal void
Mouse (Point thePt, long t, short mods)
{
WindowPtr	w;
DocHandle	doc;
ControlHandle	scroll;
short	thePart;
short	oldCtlValue;

	GetPort (&w);
	doc = WindDocHandle (w);
	scroll = DocScroll (doc);

	if ((thePart = TestControl (scroll, thePt)) == inThumb)
	{
		oldCtlValue = GetCtlValue (scroll);
		if (TrackControl (scroll, thePt, nil) == inThumb)
			ScrollText (doc, GetCtlValue (scroll) - oldCtlValue);
	}
	else if (thePart != 0)
	{
		scrollDoc = doc;		/* set globals for TrackScroll */
		scrollPart = thePart;
		(void) TrackControl (scroll, thePt, &TrackScroll);
	}
}


/*
 * Update window.  The update event might be in response to a
 * window resizing.  If so, move and resize the scroll bar,
 * and recalculate the text display.
 *
 * The ValidRect call is done because the HideControl adds the
 * control bounds box to the update region - which would generate
 * another update event!  Since everything is redrawn below anyway,
 * the ValidRect is used to cancel the update.
 */

static pascal void
Update (Boolean resized)
{
WindowPtr	w;
DocHandle	doc;
ControlHandle	scroll;
TEHandle	hTE;
Rect	r;

	GetPort (&w);
	doc = WindDocHandle (w);
	scroll = DocScroll (doc);
	hTE = DocTE (doc);

	if (resized)
	{
		r = w->portRect;
		EraseRect (&r);
		HideControl (scroll);
		r = (**scroll).contrlRect;
		ValidRect (&r);
		CalcScrollRect (w, &r);
		SizeControl (scroll, 16, r.bottom - r.top);
		MoveControl (scroll, r.left, r.top);
		OverhaulDisplay (doc);
		ShowControl (scroll);
	}
	else
	{
		r = (**hTE).viewRect;
		TEUpdate (&r, hTE);		/* redraw text display */
		if (WindowIsActive (w))
			DrawControls (w);	/* redraw scroll bar */
		else
		{
			/* draw outline of scroll, erase interior */
			r = (**scroll).contrlRect;
			FrameRect (&r);
			InsetRect (&r, 1, 1);
			EraseRect (&r);
		}
	}

	DrawGrowBox (w);
}


/*
 * When the window comes active, highlight the scroll bar appropriately.
 * When the window is deactivated, hide the scroll bar (this is drawn
 * immediately rather than invalidating the rectangle and waiting for
 * Update(), because that just seems too slow).
 *
 * Redraw the grow box.
 *
 * Notify the host as appropriate.
 *
 * Note that clicking close box hides the window, which generates a
 * deactivate event, so there is no need for a close notifier.
 */

static pascal void
Activate (Boolean active)
{
WindowPtr	w;
DocHandle	doc;
ControlHandle	scroll;
RgnHandle	oldClip;
Rect		r;

	GetPort (&w);
	doc = WindDocHandle (w);
	scroll = DocScroll (doc);

	DrawGrowBox (w);
	if (active)
	{
		HiliteScroll (doc);
		ShowControl (scroll);
	}
	else
	{
		/* hide scroll but don't show it being hidden */
		oldClip = NewRgn ();
		GetClip (oldClip);
		SetRect (&r, 0, 0, 0, 0);
		ClipRect (&r);
		HideControl (scroll);
		SetClip (oldClip);
		DisposeRgn (oldClip);
		/* now erase inside of scroll (but not outline, to avoid flicker) */
		r = (**scroll).contrlRect;		/* erase scroll */
		InsetRect (&r, 1, 1);				/* but not outline */
		EraseRect (&r);
	}

	if (DocActivateProc (doc) != nil)
		(*DocActivateProc (doc)) (active);
}


/*
 * Clobber a display window.  This routine is written defensively on the
 * assumption that not all pieces of a complete display window are present.
 * This allows it to be called by SkelRmveWind() during window creation
 * attempts if allocations fail.
 *
 * The window's skelWPropDisplayWind property structure will be disposed
 * of by TransSkel, but the data associated with it (returned by WindDocHandle())
 * must be disposed of here.
 *
 * If the window being clobbered is the current output window, do
 * SetDWindow (nil) to turn output off.  Also set dispWind to nil in
 * case the window is the current display window.
 */

static pascal void
Clobber (void)
{
WindowPtr	w;
DocHandle	doc;
TEHandle	hTE;

	GetPort (&w);
	doc = WindDocHandle (w);

	if (w == curDispWind)
		SetDWindow (nil);

	/*
	 * Toss document record and any pieces that exist
	 */
	if (doc != (DocHandle) nil)
	{
		if ((hTE = DocTE (doc)) != (TEHandle) nil)
			TEDispose (hTE);						/* toss text record */
		DisposeHandle ((Handle) doc);
	}
	DisposeWindow (w);			/* toss window (scroll bar, too) */
}


/* ---------------------------------------------------------------- */
/*							Control Routines						*/
/* ---------------------------------------------------------------- */


/*
 * Test whether a window is a legal display window or not
 */

pascal Boolean
IsDWindow (WindowPtr w)
{
	return (WindDocHandle (w) != nil);
}


/*
 * Return handle to display window's text record
 */

pascal TEHandle
GetDWindowTE (WindowPtr w)
{
DocHandle	doc;

	return ((doc = WindDocHandle (w)) == nil ? nil : DocTE (doc));
}


/*
 * Change the text display characteristics of a display window
 * and redisplay it.  As a side effect, this always scrolls to the
 * home position.
 */

pascal void
SetDWindowStyle (WindowPtr w, short font, short size, short wrap, short just)
{
DocHandle	doc;
GrafPtr		savePort;
FontInfo	f;
TEHandle	hTE;
Rect		r;

	if (w == nil)			/* reset window creation defaults */
	{
		d_font = font;
		d_size = size;
		d_wrap = wrap;
		d_just = just;
		return;
	}

	if ((doc = WindDocHandle (w)) != (DocHandle) nil)
	{
		GetPort (&savePort);
		SetPort (w);
		hTE = DocTE (doc);
		r = (**hTE).viewRect;
		EraseRect (&r);
		r = (**hTE).destRect;	/* scroll home without redrawing */
		OffsetRect (&r, 0, 2 - r.top);
		(**hTE).destRect = r;

		(**hTE).crOnly = wrap;	/* set word wrap */
		TESetJust (just, hTE);	/* set justification */

		TextFont (font);	 	/* set the font and point size */
		TextSize (size);		/* of text record (this is the */
		GetFontInfo (&f);		/* hard part) */
		(**hTE).lineHeight = f.ascent + f.descent + f.leading;
		(**hTE).fontAscent = f.ascent;
		(**hTE).txFont = font;
		(**hTE).txSize = size;

		OverhaulDisplay (doc);
		SetPort (savePort);
	}
}


/*
 * Scroll the text in the window so that line lineNum is at the top.
 * First line is line zero.
 */

pascal void
SetDWindowPos (WindowPtr w, short lineNum)
{
GrafPtr		savePort;
DocHandle	doc;

	if ((doc = WindDocHandle (w)) != (DocHandle) nil)
	{
		GetPort (&savePort);
		SetPort (w);
		ScrollText (doc, lineNum - GetCtlValue (DocScroll (doc)));
		SetPort (savePort);
	}
}


/*
 * Set display window activate notification procedure.
 * Pass nil to disable it.
 */

pascal void
SetDWindowNotify (WindowPtr w, TDispActivateProcPtr p)
{
DocHandle	doc;

	if (w == nil)			/* reset window creation default */
		d_activate = p;
	else if ((doc = WindDocHandle (w)) != (DocHandle) nil)
		DocActivateProc (doc) = p;
}


/*
 * Set display window autoflush characteristics
 */

pascal void
SetDWindowFlush (WindowPtr w, long maxText, long flushAmt)
{
DocHandle	doc;

	if (maxText > 32767L)
		maxText = 32767L;
	if (maxText < d_loMaxText)
		maxText = d_loMaxText;
	if (flushAmt < d_loFlushAmt)
		flushAmt = d_loFlushAmt;

	if (w == nil)			/* reset window creation defaults */
	{
		d_maxText = maxText;
		d_flushAmt = flushAmt;
		return;
	}

	if ((doc = WindDocHandle (w)) != (DocHandle) nil)
	{
		DocMaxText (doc) = maxText;
		DocFlushAmt (doc) = flushAmt;
	}
}


/*
 * Set which display window is to be used for output.  If theWind
 * is nil, output is turned off.  If theWind is not a legal display
 * window, nothing is done.
 */

pascal void
SetDWindow (WindowPtr w)
{
	if (w == nil || IsDWindow (w))
	{
		curDispWind = w;
	}
}


/*
 * Get the WindowPtr of the current output display window.  If
 * output is turned off, this will be nil.
 */

pascal WindowPtr
GetDWindow (void)
{
	return (curDispWind);
}


/*
 * Flush text from the window and readjust the display.
 */

pascal void
FlushDWindow (WindowPtr w, long byteCount)
{
DocHandle	doc;
TEHandle	hTE;

	if ((doc = WindDocHandle (w)) != (DocHandle) nil)
	{
		hTE = DocTE (doc);
		TESetSelect (0L, byteCount, hTE);	/* select text */
		TEDelete (hTE);						/* clobber it */
		OverhaulDisplay (doc);
	}
}


/*
 * Create and initialize a display window and the associated data
 * structures.  If the window and data cannot be allocated, destroy
 * the window and return nil.  Otherwise return the window.
 *
 * The window is made the current output window, but the caller should
 * set and restore the port before and after calling SetupDocWind().
 */
 
static WindowPtr
SetupDocWind (WindowPtr w)
{
Rect	r;
DocHandle	doc;
SkelWindPropHandle	prop;
ControlHandle	scroll;
TEHandle		hTE;

	if (!SkelWindow (w,	/* the window */
				Mouse,			/* mouse click handler */
				nil,			/* key clicks are ignored */
				Update,			/* window updating procedure */
				Activate,		/* window activate/deactivate procedure */
				nil,			/* TransSkel hides window if no close proc */
								/* (generates deactivate event) */
				Clobber,		/* window disposal procedure */
				nil,			/* no idle proc */
				false))			/* irrelevant since no idle proc */
	{
		DisposeWindow (w);
		return (nil);
	}

	/*
	 * After this point SkelRmveWind() can be called to remove the window
	 * if any allocations fail.
	 */

	/*
	 * Get new document record, attach to window property list.
	 * Also make document record point to window.
	 */

	if (!SkelAddWindProp (w, skelWPropDisplayWind, (long) 0L))
	{
		SkelRmveWind (w);
		return (nil);
	}
	doc = New (DocRecord);
	if (doc == (DocHandle) nil)
	{
		SkelRmveWind (w);
		return (nil);
	}
	prop = SkelGetWindProp (w, skelWPropDisplayWind);
	(**prop).skelWPropData = (long) doc;
	DocWind (doc) = w;

	/*
	 * Build the scroll bar.  Make sure the borders overlap the
	 * window frame and the frame of the grow box.
	 */

	CalcScrollRect (w, &r);
	scroll = NewControl (w, &r, "\p", true, 0, 0, 0, scrollBarProc, 0L);
	DocScroll (doc) = scroll;

	/*
	 * Create the TE record used for text display.  Use defaults for
	 * display characteristics.  Setting window style overhauls
	 * display, so can cancel any update event pending for the window.
	 */

	CalcEditRect (w, &r);
	hTE = TENew (&r, &r);
	DocTE (doc) = hTE;

	if (scroll == (ControlHandle) nil || hTE == (TEHandle) nil)
	{
		SkelRmveWind (w);
		return (nil);
	}

	SetDWindowNotify (w, d_activate);
	SetDWindowFlush (w, d_maxText, d_flushAmt);
	SetDWindowStyle (w, d_font, d_size, d_wrap, d_just);

	/*
	 * Make window current display output window
	 */

	SetDWindow (w);
	return (w);
}


/*
 * Create and initialize a display window and the associated data
 * structures, and return the window pointer.
 *
 * The parameters are similar to those for NewWindow.  See Inside
 * Macintosh.
 *
 * Preserves the current port.  If the window is visible,
 * an activate event will follow, at which point the port
 * will be set to the window.
 */
 
pascal WindowPtr
NewDWindow (Rect *bounds, StringPtr title, Boolean visible,
					WindowPtr behind, Boolean goAway, long refCon)
{
WindowPtr	w;
GrafPtr		savePort;

	if (SkelQuery (skelQHasColorQD))
	{
		w = NewCWindow (nil,
						   bounds,
						   title,
						   visible,
						   documentProc + 8,
						   behind,
						   goAway,
						   refCon);
	}
	else
	{
		w = NewWindow (nil,
						   bounds,
						   title,
						   visible,
						   documentProc + 8,
						   behind,
						   goAway,
						   refCon);
	}

	if (w != (WindowPtr) nil)
	{
		GetPort (&savePort);
		w = SetupDocWind (w);		/* nil if allocation failed */
		SetPort (savePort);
	}
	return (w);
}


/*
 * Create and initialize a display window (using a resource) and
 * the associated data structures, and return the window pointer.
 * Install window in list of display windows.  In single-window
 * mode, disallow creation of a new window if one already exists.
 *
 * The parameters are similar to those for GetNewWindow.  See Inside
 * Macintosh.
 */
 
pascal WindowPtr
GetNewDWindow (short resourceNum, WindowPtr behind)
{
WindowPtr	w;
GrafPtr		savePort;

	if (SkelQuery (skelQHasColorQD))
		w = GetNewCWindow (resourceNum, nil, behind);
	else
		w = GetNewWindow (resourceNum, nil, behind);

	if (w != (WindowPtr) nil)
	{
		GetPort (&savePort);
		w = SetupDocWind (w);		/* nil if allocation failed */
		SetPort (savePort);
	}
	return (w);
}


/* ------------------------------------------------------------ */
/*						Output Routines							*/
/* ------------------------------------------------------------ */


/*
 * Write text to display area if output is on (curDispWind != nil).
 * DisplayText is the fundamental output routine.  All other
 * output calls map (eventually) to it.
 *
 * First check whether the insertion will cause overflow and flush
 * out some stuff if so.  Insert new text at the end, then test
 * whether lines must be scrolled to get the new stuff to show up.
 * If yes, then do the scroll.  Set values of scroll bar properly
 * and highlight as appropriate.
 *
 * The current port is preserved.  Since all output calls end up
 * here, it's the only output routine that has to save the port
 * and check whether output is on.
 */

pascal void
DisplayText (Ptr theText, long len)
{
DocHandle	doc;
TEHandle	hTE;
ControlHandle	scroll;
short	nLines;			/* # of lines in TERec */
short	dispLines;		/* # of lines displayable in window */
short	topLines;		/* # of lines currently scrolled off top */
short	scrollLines;	/* # of lines to scroll up */
short	lHeight;
Rect	r;
GrafPtr	savePort;

	if (curDispWind == nil)
		return;

	GetPort (&savePort);
	SetPort (curDispWind);
	doc = WindDocHandle (curDispWind);
	hTE = DocTE (doc);
	scroll = DocScroll (doc);

	if ((**hTE).teLength + len > DocMaxText (doc))	/* check overflow */
	{
		FlushDWindow (curDispWind, DocFlushAmt (doc));
		DisplayString ((StringPtr) "\p\r(autoflush occurred)\r");
	}

	lHeight = (**hTE).lineHeight;
	TESetSelect (32767L, 32767L, hTE);	/* set to insert at end */
	TEInsert (theText, len, hTE);
	r = (**hTE).viewRect;
	nLines = (**hTE).nLines;
	dispLines = (r.bottom - r.top) / lHeight;
	topLines = LinesOffTop (hTE);
	scrollLines = nLines - (topLines + dispLines);
	if (scrollLines > 0) /* must scroll up */
		TEScroll (0, -lHeight * scrollLines, hTE); /* scroll up */
	topLines = nLines - dispLines;
	if (topLines >= 0 && GetCtlMax (scroll) != topLines)
	{
		SetCtlMax (scroll, topLines);
		SetCtlValue (scroll, topLines);
	}
	HiliteScroll (doc);
	SetPort (savePort);
}


/*
 * Derived output routines:
 *
 * DisplayString	Write (Pascal) string
 *
 * DisplayLong		Write value of long integer
 * DisplayShort		Write value of short integer
 * DisplayChar		Write character
 *
 * DisplayHexLong	Write value of long integer in hex (8 digits)
 * DisplayHexShort	Write value of short integer in hex (4 digits)
 * DisplayHexChar	Write value of character in hex (2 digit)
 *
 * DisplayOSType	Write OSType value
 *
 * DisplayBoolean	Write boolean value
 * DisplayLn		Write carriage return
 */

pascal void
DisplayString (StringPtr str)
{
	DisplayText ((Ptr) (str+1), (long) str[0]);
}


pascal void
DisplayCString (char *str)
{
long	len = 0;
char	*s;

	for (s = str; *s != '\0'; s++)
		++len;
	DisplayText ((Ptr) str, len);
}


pascal void
DisplayChar (short c)
{
char	ch = c;

	DisplayText (&ch, 1L);
}


pascal void
DisplayShort (short i)
{
	DisplayLong ((long) i);
}


pascal void
DisplayLong (long l)
{
Str255		s;

	NumToString (l, s);
	DisplayString (s);
}


pascal void
DisplayLn (void)
{
	DisplayChar ('\r');
}


pascal void
DisplayBoolean (Boolean b)
{
	DisplayString (b ? (StringPtr) "\ptrue" : (StringPtr) "\pfalse");
}


static char
HexByte (short value)	/* should be 0..15 */
{
	DisplayChar ((char) (value + (value < 10 ? '0' : 'a' - 10)));
}


pascal void
DisplayHexChar (short c)
{
	HexByte ((short) (c >> 4) & 0x0f);
	HexByte ((short) c & 0x0f);
}


pascal void
DisplayHexShort (short i)
{
	DisplayHexChar ((char) ((i >> 8) & 0xff));
	DisplayHexChar ((char) (i & 0xff));
}


pascal void
DisplayHexLong (long l)
{
	DisplayHexShort ((short) (l >> 16) & 0xffff);
	DisplayHexShort ((short) l & 0xffff);
}


pascal void
DisplayOSType (OSType type)
{
long	l = (long) type;

	DisplayChar ((l >> 24) & 0xff);
	DisplayChar ((l >> 16) & 0xff);
	DisplayChar ((l >> 8) & 0xff);
	DisplayChar (l & 0xff);
}
