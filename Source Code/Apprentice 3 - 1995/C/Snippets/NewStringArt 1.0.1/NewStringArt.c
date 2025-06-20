// NewStringArt 1.0.1
// adapted from a Sun Microsystems demo by Ron Hitchens
// ported to CodeWarrior by Ken Long (kenlong@netcom.com)
// updated for CodeWarrior 6 on 950712

/*	This program is public domain.  It may be be freely distributed,
 *	copied and used.  All I ask is that my name and this notice remain
 *	on it.  And that modifications be noted in this source listing and
 *	returned to me if possible at the above email address.
 *
 *	History:
 *		May 85	Hitchens	Created.
 *		May 28	Hitchens	Look at screen size vars, add comments
 *
 *
 *	Compilation instructions with Megamax C:
 *	You need the C source files stringart.c and gennums.c
 *		1. compile and link gennums.c
 *		2. run gennums, it creates a C source file named stringnums.c
 *		  This name is hard-coded in gennums.c
 *		3. Compile stringnums.c, but don't link it.  Keep stringnums.o
 *		4. Compile stringart.c
 *		5. Link stringart.o and stringnums.o together to make stringart
 *		6. Run StringArt.
 *		7. Sit back and enjoy the show.
 *	If you make changes to stringart, after doing the above, you can skip
 *	to 4.  Stringnums doesn't need to be re-compiled.
 *	If you want to port this to another flavor of C you're on your own.
 *
 *	The icon was created manually with the resource editor, to retain
 *	the icon when recompiling just specify the name of the old 
 *	StringArt application file as the output of the linker.  It will
 *	overwrite the code but won't disturb the icon resources.
 */

/*
	This demo creates random vector designs.  This is accomplished by
	randomly choosing a function for each coordinate halve of the two
	points describing a vector that moves through two dimensional
	space. Both x coordinate halves cannot be the same since the design
	would simply be a collection of vertical lines. Similarly both
	y coordinate halves cannot be the same.

	The functions are:

	function[0][x] = sin( 2 * PI * x / numLines )
	function[1][x] = -sin( 2 * PI * x / numLines )
	function[2][x] = cos( 2 * PI * x / numLines ) 
	function[3][x] = -cos( 2 * PI * x / numLines ) 
	function[4][x] = sin( 4 * PI * x / numLines )
	function[5][x] = -sin( 4 * PI * x / numLines )
	function[6][x] = cos( 4 * PI * x / numLines ) 
	function[7][x] = -cos( 4 * PI * x / numLines ) 
	function[8][x] = sin( 6 * PI * x / numLines )
	function[9][x] = -sin( 6 * PI * x / numLines )
	function[10][x] = cos( 6 * PI * x / numLines ) 
	function[11][x] = -cos( 6 * PI * x / numLines ) 
	function[12][x] = 2 * abs( x - (numLines / 2) - 1 )	*not used*

	The values of the functions were pre-computed to have the demo
	run as fast as possible.  The program runs in an endless loop, it 
	will terminate if the "Quit" menu item is selected or a 'Q' is typed.
*/

#define numLines	343	/* number of vectors in a design */
#define num_functions	12	/* number of functions */
#define Voffset		19	/* offset from top, drag is under menu bar */
#define border		2	/* surrounding blank border */
				
/* get real size of screen, in case of Lisa or something new */
#define Hsize (qd.screenBits.bounds.right)
#define Vsize (qd.screenBits.bounds.bottom)

WindowRecord	myRecord;	/* if you can't figure out what these are */
WindowPtr	myWindow;	/* you'd better give up right now */
MenuHandle	myMenu;

short holding = 0;		/* are we currently holding? */
short step = 0;			/* single step? */

short q [4][numLines];		/* temp array for scaled coordinates */
extern short p [num_functions][numLines];
	/* pre-computed points, scaled by 1024 */
long ticks;

main()
{
	short i, j, k, l, m;
	long secs;
	
	Init ();		/* do housekeeping */
	ShowInfo ();		/* display bragging info */
	GetDateTime ((unsigned long*) qd.randSeed);	/* randomize the Random() function */

	while (1) {		/* main loop, go round and round forever */
		/* pick the functions */
		while ((i = Random() % num_functions) < 0) /* nothing */ ;
		while ((j = Random() % num_functions) == i || (j < 0));
		while ((k = Random() % num_functions) < 0);
		while ((l = Random() % num_functions) == k || (l < 0));

		/* scalepoints was a hack to speed up the line drawing, by
		  avoiding the scaling computations for each coordinate
		  while the lines are being drawn.  It didn't help as much
		  as I'd hoped, only about a second is saved, maybe less.
		  Apparently I'm pushing QuickDraw as fast as it will go.
		  The same calculations using floating point took about
		  45 seconds, YOW! */
		    
		ScalePoints (i, k, j, l);	/* scale to the window */
		while (holding && !step)	/* loop here if holding */
			CheckMenu ();		/* watch events when holding */

		BlankWindow (myWindow);		/* clear to black or white */
		SetPort (myWindow);		/* point QD at my window */
			/* the above statement shouldn't be necessary, but
			  something sneaky, like the screensaver DA, could
			  be lurking in the darkness and may change the
			  grafport out from under us.  Grrr... */
		ObscureCursor ();		/* get rid of the cursor */

		for (m = 0; m < numLines; m++) {	/* draw the vectors */
			MoveTo (q [0][m], q [1][m]);
			LineTo (q [2][m], q [3][m]);
		}

		SystemTask ();			/* in case of DAs */
		if (! holding) 
			Delay (60L, &ticks);	/* pause for ~6 secs */
		step = 0;			/* only good for one shot */
	}
}


/* the numbers in the array p were pre-computed using floating point
   arithmetic, they were all sin or cos functions and so yielded
   numbers [-1, 1].  These numbers were then scaled up by 1024 so they
   could be stored as integers.  1024 was used so they could be scaled down
   again with a shift-right-10 rather than a divide operation.  I use two
   statements to compute each number to be sure the intermediate values
   are calculated as longs, the values involved far exceed the magnitude
   possible in a 16-bit short.  */
     
ScalePoints (x1, y1, x2, y2)	short x1, y1, x2, y2;
{
	long  l, h, v;
	short i;
	
	h = (Hsize) / 2;
	v = (Vsize - Voffset) / 2;
	for (i = 0; i < numLines; i++) {
		l = (((long)p [x1][i] * (h - border)) >> 10) + h;
		q [0][i] = (short)l;
		l = (((long)p [y1][i] * (v - border)) >> 10) + v;
		q [1][i] = (short)l;
		l = (((long)p [x2][i] * (h - border)) >> 10) + h;
		q [2][i] = (short)l;
		l = (((long)p [y2][i] * (v - border)) >> 10) + v;
		q [3][i] = (short)l;
		CheckMenu ();	/* see if there are any events to be handled */
	}
}


CheckMenu ()
	/* test for and handle any events, such as keyboard and mouse */
{
	EventRecord	theEvent;
	WindowPtr	tmpPtr;

			/* loop as long as there are events in the queue */
	while (GetNextEvent (everyEvent, &theEvent)) {
		switch (theEvent.what) 
		{
//			case abortEvt:		/* something is telling us to stop */
//				exit (0);
			case keyDown:		/* got a key */
					/* does this key belong to a menu? */
				if (MenuKey ((char) (theEvent.message % 256))) 
				{	/* yes */
					Do_Menu (MenuKey ((char) (theEvent.message % 256)));
				} 
				else 
					{
						step = 1;	/* step once (undoc feature) */	
						return;
				}
			break;
			
			case mouseDown:		/* mouse clicked */
				if (FindWindow(theEvent.where, &tmpPtr) == inMenuBar)
				{
					Do_Menu (MenuSelect (theEvent.where));
				} else {
					step = 1;	/* cycle once */
				}
			break;
	
			case updateEvt:	/* always get one of these at startup */
				BeginUpdate (myWindow);
				EndUpdate (myWindow);	/* only way to get rid of it */
			break;
		
			case nullEvent:	/* shouldn't happen, filtered above */
				return;
		
			default:
			break;		/* not interested in anything else */
		}
	}
}

Do_Menu (item)		long item;	/* handle a menu selection */
{
	HiliteMenu (1);
	switch (item) 
	{
		case 0x00010001:		/* menu 1, item 1 (hold) */
			Toggle_Hold ();
		break;
		
		case 0x00010002:		/* menu 1, item 2 (invert) */
			Invert ();
		break;
		
		case 0x00010003:		/* menu 1, item 3 (author info) */
			ShowInfo ();
		break;
		
		case 0x00010004:		/* menu1, item 4 (quit) */
			ExitToShell ();
	}
	HiliteMenu (0);
}
	

Toggle_Hold ()
{
	holding = !holding;		/* toggle the flag */
	CheckItem (myMenu, 1,  holding);
		/* place or remove checkmark, depending on holding */
}

Invert ()
{
	SetPort (myWindow);			/* on the off chance... */
	if (myWindow->pnMode == patBic) {
		PenMode (patCopy);			/* draw black */
		TextMode (srcCopy);
	} 
	else 
		{
			PenMode (patBic);			/* draw white */
			TextMode (srcBic);
	}
	InvertRect (&myWindow->portRect);	/* flip what's onscreen */
}
			

Init ()					/* do initial housekeeping chores */
{
	ManagerInits ();
	WindowInits ();
	MenuInits ();
}

ManagerInits()
{
   	InitGraf(&qd.thePort);		/* initialize qd (theport in qd) */
	InitFonts();			/* initialize font manager */
	InitWindows();			/* initialize window manager */
	InitMenus();			/* initialize menus */
	InitCursor();			/* set cursor to arrow */
	FlushEvents(everyEvent, 0);	/* lose any outstanding events */
}

WindowInits()
{
	Rect		tempRect;
	
	SetRect(&tempRect, 0, Voffset, Hsize, Vsize);
	myWindow = NewWindow(&myRecord, &tempRect, "\pStringart", 1, 0,
				(WindowPtr)-1L, 0, (long) 0);
	SetPort(myWindow);		/* draw in my window */
	SelectWindow(myWindow);		/* make mine active */
	PenNormal ();			/* make sure pen has defaults */
	PenMode (patBic);		/* make it draw white */
	TextMode (srcBic);		/* make text draw white */
	TextFont (0);			/* use system font */
	TextFace ((short) bold);	/* use a fancy text style */
}

MenuInits ()
{
	myMenu = NewMenu (1, "\pStringArt");	/* allocate a menu struct */
	AppendMenu (myMenu,  "\pHold/H");		/* load the item values */
	AppendMenu (myMenu, "\pInvert/I");
	AppendMenu (myMenu, "\pAuthor Info/A");
	AppendMenu (myMenu, "\pQuit/Q");
	InsertMenu (myMenu, 0);			/* let the OS know about it */
	DrawMenuBar ();				/* make it show up */
}
/*
Sleep (short s)
	// sleep for approx s seconds, could vary from s-1 to s seconds
{
	unsigned long s1, s2;

	GetDateTime ((unsigned long*) s1);		//what time is it now?
	s1 += s;			// looking for now + s seconds.
	while (1) 
	{
		GetDateTime ((unsigned long*) s2);
		if (s1 <= s2) 
			break;	// ok, the time has come.
		CheckMenu ();		// keep an eye on events around you.
	}
}
*/
BlankWindow (w)		WindowPtr w;
{
	if (w->pnMode == patBic) 
	{	/* drawing white? */
		FillRect (&w->portRect, &qd.black); /* yes, paint window black */
	} 
	else 
		{
			FillRect (&w->portRect, &qd.white); /* no, paint it white */
	}
}

short width;

ShowInfo ()		/* display bragging info */
{
	static char *s [] = {
		(char*) "\pStringArt by Ron Hitchens.",
		(char*) "\p13 September 1985.",
		(char*) "\phitchens@ut-sally.ARPA  or  ...!ihnp4!ut-sally!hitchens.UUCP",
		(char*) "\p", 
		(char*) "\pWritten in MegaMax C.",
		(char*) "\pThis program is public domain.", 
		(char*) "\p",
		(char*) "\pWhile in Hold, click mouse to single step",
		(char*) "\pTo save an image, type:  SHIFT/CMD/3. (duh)" ,
		(char*) "\pTo print an image, type:  SHIFT/CMD/4. (duh)",
		(char*) "\p", 
		(char*) "\pMade to run in Code Warrior as",
		(char*) "\pan itty bitty bytes� revival project, ",
		(char*) "\pperpetrated by Kenneth A. Long - 20 Nov 94." 
	};
	
	short i, n;
	static int busy = 0;
	
	HiliteMenu (1);			/* turn on the menu */
	if (busy) return;		/* info screen already up */
	busy = 1;			/* to prevent recursive displays */
	SetPort (myWindow);		/* just in case */
	ObscureCursor ();		/* get rid of the cursor */
	BlankWindow (myWindow);
	n = sizeof (s) / sizeof (s [0]);	/* how many lines of text? */
	TextSize (24);				/* get fancy for first line */
	for (i = 0; i < n; i++) {		/* display all the text */
		MoveTo ((Hsize / 2) - (StringWidth ((StringPtr) s [i]) / 2),
			(Vsize / (n + 1)) + (i * (Vsize / (n + 1))));
		DrawString ((StringPtr) s [i]);	/* draw one of the text lines */
		TextSize (12);		/* back to normal for the rest */
	}
	if (!holding) 
		Delay (60L, &ticks);		/* wait a bit */
	HiliteMenu (0);				/* turn off the menu */
	busy = 0;				/* ok, I'm finished */
}

