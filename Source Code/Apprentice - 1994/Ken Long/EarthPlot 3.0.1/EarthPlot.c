

/*** Earthplot V3.0

Another revision of Earthplot. This one was prompted by the fact that
System 4.1 broke programs written in the old versions of Megamax C and 
guess what language Earthplot was written in?  So it has been rewritten
to compile under Think Technologies' LightspeedC (V2.13 used for V3.0).  
Also, rather than SimpleTools, the TSkel package from Paul DuBois was 
used as the platform.

Enhancements include:

	- full clipboard support (both bitmaps and PICTs)
	- a choice of using either integer or floating point math
	- ability to resize the earth window (for those BIG new screens)
	- simple color support for color capable machines
	- the raw earth data now lives in an 'eDAT' resource
	- other minor cosmetic and other changes

-- The Black Swamp Software Company
-- AKA michael peirce, reachable at... [as of December 1987]

		ARPANET:	peirce@lll-crg.llnl.gov
		MCI-Mail:	mpeirce
		Delphi:		mpeirce
		US Mail:	1265 Braddock Ct.
					San Jose, CA 95125

***

 *** Earthplot V2.0
Here's a new version of earthplot.  For those who missed it, it is
a program that plots an outline of the earth given the latitute,
longitude, and altitude.  I really liked the program but had problems
with it's performance (5-6 minutes to draw the earth).
 
The new version is vastly improved.  The plot is completed in about
35 seconds.  How did I do it?  I got rid of the unnecessary use of
floating point and substituted integer arithmetic with implied decimal
places (1.0 is represented as 10000).  Accuracy is good enough so that
out of 7000 data points, only 400 are off by no more than one pixel.
I also incorporated the data file into a code segment so the disk i/o
involved in reading the data is also eliminated.
 
A binhex'ed version of the new program is posted as a separate article
so you won't have to compile this to enjoy it.
 
Enjoy!!
  Marsh Gosnell   usenet: ihnp4!lzma!mkg
***
 
 **
 ** EarthPlot V1.0 -- a program to draw the earth as viewed from space.
 **
 ** Written by:	
 **		Michael Peirce (peirce@lll-crg.arpa)
 **		1680 Braddock Ct.
 **		San Jose, CA 95125
 **
 ** Significant performance improvements by:
 **		 Marsh Gosnell
 **		 35 Godfrey Road
 **		 Montclair, NJ  07043
 **
 **	...in Megamax C (version 2.1)   w/ the SimpleTools package
 **
 **	Update by:
 **		Kenneth A. Long
 **		kenlong@netcom.com
 **
 ** Based on Microplot program written in Fortran by Richard Heurtley
 ** (This can be found in file B900:MICROPLOT.FTN on the MTS system 
 **  at Rensselear Polytechnic Institute in Troy, New York)
 **
 ** Who	When			What
 ** ===	===========		====================================
 ** mrp	01-dec-1985		Final work done for version 1.0
 ** mkg 20-dec-1985		Performance improvements
 ** mrp 15-aug-1987		V3.0 in Lightspeed C w/ TSkel
 **
 ** kal 18-jan-1994		V3.0.1 in Think C 5.0.4 w/ TansSkel 2.6
 **
 
 ** [2.0 notes]
 ** The performance improvments are:
 **
 ** - use of integer arithmetic where possible.  All values are small
 **   enough so that using integer arithmetic with 4 implied decimal
 **   places (e.g., multiplying by 10000) yields sufficient accuracy.
 **   (Out of 7000 displayed points, 400 are off by 1 pixel).
 **
 ** - data is kept in a separate code segment.  avoids disk i/o
 **
 ** - new lmul routine.  The Megamax lmul routine always does things the
 **   hard way even if the two long values are shorts.  The new lmul will
 **   lmul will run really fast if the two values are shorts (which they
 **   usually are in this case).
 **
 
 ** [1.0 notes]
 ** Because Earthplot is in such a state (and because I did it 
 ** primarily as a learning experience) both source and executable
 ** are being placed into the public domain.  I encourage anyone to
 ** pick up on this start and expand on it's theme.  All I ask is
 ** that if anyone does improve on this program them send the
 ** results back to me (either via the net or other means).
 **
 ** The following is a list of possible extensions:
 **
 ** - support cut & paste to the clipboard
 **
 ** - support resizing the drawing window to allow various sizes
 **   of earth plots
 **
 ** - save plots directly into a file (a save option)
 **
 ** - either a scripting facility or a batch runable version so
 **   that a large number of plots can be generated without human
 **   interaction.  This would allow one to either feed these plots
 **   into something like Videoworks to create a "real-time" rotation
 **   of the earth or a special "player" program that would do this.
 **   (Note: this has already been done for my roommate's IBM-PC clone.
 **   Come on MacFolks, we can do it better than THEM).
 **
 ** - compress the data (from ASCII into binary data)
 **
 ** - put the data into (a) the data fork of the executable or (b) a
 **   a resource
 **
 ** - improve on hidden line and/or ploting algorithm...
 **
 ** - add a floating point chip to the Mac!!!!! (they did!!! yeah Mac II!)
 **/

#include "EarthPlot.h"

cWindowDataUpate (Resized) /************************************************************/
Boolean	Resized;
{
char	s[64];

	MoveTo (75, 82);
	NumToString (GetCtlValue (latSB), s);
	EraseRect (&latDataRect);
	DrawString (s);

	MoveTo (82, 162);
	NumToString (GetCtlValue (lonSB), s);
	EraseRect (&lonDataRect);
	DrawString (s);

	MoveTo (73, 242);
	NumToString (GetCtlValue (altSB)* (long)altScale, s);
	EraseRect (&altDataRect);
	DrawString (s);
}

cUpdate (Resized) /************************************************************/
Boolean	Resized;
{
GrafPtr	thePort;

	GetPort (&thePort);

	PlotIcon (&iconRect, iconHandle);
	DrawControls (thePort);

	MoveTo (0, 51);	/* draw nice lines :-) */
	LineTo (175, 51);
	MoveTo (0, 54);
	LineTo (175, 54);

	TextFont (0);
	MoveTo (60, 30);
	DrawString ("\p�EarthPlot v3");
	MoveTo (9, 82);
	DrawString ("\pLatitude:");
	MoveTo (9, 162);
	DrawString ("\pLongitude:");
	MoveTo (9, 242);
	DrawString ("\pAltitude:");
	
	cWindowDataUpate (true);
}

cClose () /************************************************************/
{
	SkelWhoa ();
}

cClobber () /************************************************************/
{
GrafPtr	thePort;

	GetPort (&thePort);				/* grafport of window to dispose of */
	DisposeWindow ((WindowPtr) thePort);
}

cActivate () /************************************************************/
{
}

pascal void Track (Control, partCode)
ControlHandle	Control;
int				partCode;
{
int		i, step;
long	wait;
long	waited;
char	s[128];

	wait = 5;  /* ticks */

	if (partCode == 0) return;
	
	switch (partCode) {
	case inUpButton:
		step = -1;
		break;
	case inDownButton:
		step = 1;
		break;
	case inPageUp:
		step = -10;
		break;
	case inPageDown:
		step = 10;
		break;
	}
	
	i = GetCtlValue (Control) + step;
	
	if (*Control == *latSB) {
		if (i > 90) i = 90;
		if (i <  0) i = 0;

		SetCtlValue (Control, i);

		EraseRect (&latDataRect);
		MoveTo (75, 82);
		NumToString (GetCtlValue (latSB), s);
		DrawString (s);
		Delay (wait, &waited);
	}

	if (*Control == *lonSB) {
		if (i > 180) i = 180;
		if (i <   0) i = 0;

		SetCtlValue (Control, i);

		EraseRect (&lonDataRect);
		MoveTo (82, 162);
		NumToString (GetCtlValue (lonSB), s);
		DrawString (s);
		Delay (wait, &waited);
	}

	if (*Control == *altSB) {
		if (i > 180) i = 180;
		if (i <   0) i = 0;

		SetCtlValue (Control, i);

		EraseRect (&altDataRect);
		MoveTo (73, 242);
		NumToString (GetCtlValue (altSB)* (long)altScale, s);
		DrawString (s);
		Delay (wait, &waited);
	}
}

cMouse (thePoint, theTime, theMods) /************************************************************/
Point	thePoint;
long	theTime;
int		theMods;
{
ControlHandle	theControl;
int				partCode;
char			s[100];

	partCode = FindControl (thePoint, cWindow, &theControl);

	if (partCode) {
		switch (partCode) {
		case inCheckBox:
			partCode = TrackControl (theControl, thePoint, 0L);
			if (*theControl == *northCheck) {
				SetCtlValue (northCheck, 1);
				SetCtlValue (southCheck, 0);
			}
			if (*theControl == *southCheck) {
				SetCtlValue (northCheck, 0);
				SetCtlValue (southCheck, 1);
			}
			if (*theControl == *eastCheck) {
				SetCtlValue (eastCheck, 1);
				SetCtlValue (westCheck, 0);
			}
			if (*theControl == *westCheck) {
				SetCtlValue (eastCheck, 0);
				SetCtlValue (westCheck, 1);
			}
			if (*theControl == *mileCheck) {
				SetCtlValue (mileCheck, 1);
				SetCtlValue (kmCheck, 0);
			}
			if (*theControl == *kmCheck) {
				SetCtlValue (mileCheck, 0);
				SetCtlValue (kmCheck, 1);
			}
			break;
		case inUpButton:
		case inDownButton:
		case inPageUp:
		case inPageDown:
			partCode = TrackControl (theControl, thePoint, Track);
			break;
		case inThumb:
			partCode = TrackControl (theControl, thePoint, 0L);
			cWindowDataUpate (FALSE);
			break;
		}
	}
}

cKey () /************************************************************/
{
}

DoAbout ()
{
	Alert (AboutAlert, nil);
}

eraseTerra ()
{
	SetPort (eWindow);

	FillRect (&eWindow->portRect, white);
	FrameOval (&eWindow->portRect);
}

DrawEarth ()
{
	register		int					j, a, b, c;
	register		long				la, lb, lc;
					int					temp, i;
	register		unsigned char		**earthData;
	register		unsigned char		*ptr;
	EventRecord		theevent;

	SetPort (cWindow);
	HiliteWindow (cWindow, FALSE);

	SetPort (eWindow);
	BringToFront (eWindow);
	HiliteWindow (eWindow, true);

	SetCursor (*watchCursorHand);

	//� First, we say what these sizes are.
	
	//� Horizontal window size.
	xsize = ixsize = eWindow->portRect.right  - 	
					 eWindow->portRect.left - 2;	
	
	//� Vertial window size.
	ysize = iysize = eWindow->portRect.bottom - 
					 eWindow->portRect.top  - 2;	
	
	//� Then we say half of those are those divided by 2 (duh!).
	//� We need these sizes for plotting of point calculations.
	half_xsize = ihalf_xsize = xsize / 2.0;
	half_ysize = ihalf_ysize = ysize / 2.0;

	ClipRect (&eWindow->portRect);	//� Just do the drawing in eWindow.

	if (ePict != ebmPict)
		HPurge (ePict);		//� Mark this pict purgeable
	HPurge (ebmPict);		//� Same here.

	BackColor (backgroundC);			//� Whatever user said.
	EraseRect (&eWindow->portRect);		//� Get rid of ePict.

	if (npictButton == 1) 
	{
		ePict = OpenPicture (&eWindow->portRect);
		ShowPen ();		
	}

	ForeColor (earthbackgroundC);		//� Whatever user said.
	PaintOval (&eWindow->portRect);		//� Paint it.

	ForeColor (earthoutlineC);			//� User choice or default.
	FrameOval (&eWindow->portRect);		//� Do it.

	//� These both start at zero.
	alatd = GetCtlValue (latSB);		//� Lattitude control setting.
	alond = GetCtlValue (lonSB);		//� Longitude control setting.
	
	height = GetCtlValue (altSB) * (long) altScale;
	
	if (GetCtlValue (mileCheck) == 0)
		height = height * 1.61;			//� Constant times setting.

	alat = alatd * conv;
	alon = alond * conv;	
	
	cos1 = cos (alat);
	cos2 = cos (alon);	
	sin1 = sin (alat);
	sin2 = sin (alon);	
	
	if (GetCtlValue (southCheck) == 1) 
		sin1 = -sin1;
	if (GetCtlValue (westCheck)  == 1) 
		sin2 = -sin2;

	xpos = (height + diam) / diam;
	xmax = 1.0 / xpos;		
	
	
	scaler = sqrt (1.0 - (xmax * xmax));
	
	icos1 = (long) (cos1 * fudge);
	icos2 = (long) (cos2 * fudge);
	isin1 = (long) (sin1 * fudge);
	isin2 = (long) (sin2 * fudge);
	ixmax = (long) (xmax * fudge);
	ixpos = (long) (xpos * fudge);
	iscaler = (long) (scaler * fudge);
	
	
	MoveTo ((int) xsize, (int) half_ysize);
	
	draw = true;	//� Okay to draw.
	
  if (nlnlBox) 
		for (j = 1; j < 11; j++) 
		{
 
			ForeColor (latC);

			alat   = (double) ((j - 6) * 15) * conv;
			coslat = cos (alat) * fudge;
			iz = (long) (sin (alat) * fudge);
			
			draw = FALSE;
			over = FALSE;
			
			for (i = 0; i < 204; i += 2) {
 
				alon = (double) (i) * pi100;
				ix = (long) (coslat * cos (alon));
				iy = (long) (coslat * sin (alon));
 
				xxx_int (ix, iy, iz);
 
				draw = true;
 
			}
			if (GetNextEvent (everyEvent, &theevent) != 0)
				if (theevent.what == mouseDown) goto abort_drawing;
		}

  if (nlnlBox)
		for (j = 0; j < 24; j++) {
 
			ForeColor (latC);

			alon   = (double) ((j * 15)) * conv;
			coslon = cos (alon) * fudge;
			sinlon = sin (alon) * fudge;
			draw = FALSE;
			over = FALSE;
			for (i = 0; i < 104; i += 2) {
 
				alat = ((double) (i) / 100.0 - 0.5) * pi;
				coslat = cos (alat);
				ix = (long) (coslat * coslon);
				iy = (long) (coslat * sinlon);
				iz = (long) (sin (alat) * fudge);
 
				xxx_int (ix, iy, iz);
 
				draw = true;
 
			}
			if (GetNextEvent (everyEvent, &theevent) != 0)
				if (theevent.what == mouseDown) goto abort_drawing;
		}

 	earthData = (unsigned char **)GetResource ('eDAT', 100);
 	HLock (earthData);
 	ptr = *earthData;
	j = 0;
	
	ForeColor (landC);

	if (nfpButton) 
		while (j</*9262*/27786) {
			a = ptr[j++];
			b = ptr[j++];
			c = ptr[j++];
			draw = (a+b+c) != 0;
			if (!draw) {
				a = ptr[j++];
				b = ptr[j++];
				c = ptr[j++];
			}
			
			x = (a / 127.5) - 1.0;
			y = (b / 127.5) - 1.0;
			z = (c / 127.5) - 1.0;
			
			xxx (x, y, z);
							
			if (GetNextEvent (everyEvent, &theevent) != 0)
				if (theevent.what == mouseDown) goto abort_and_close;
		}

	if (nintButton) 
		while (j</*9262*/27786) {
			a = ptr[j++];
			b = ptr[j++];
			c = ptr[j++];
			draw = a || b || c;
			if (!draw) {
				a = ptr[j++];
				b = ptr[j++];
				c = ptr[j++];
			}
			
			la = (((long)a * fudge2) >> 8) - fudge;
			lb = (((long)b * fudge2) >> 8) - fudge;
			lc = (((long)c * fudge2) >> 8) - fudge;
			
			xxx_int (la, lb, lc);
			
			if (GetNextEvent (everyEvent, &theevent) != 0)
				if (theevent.what == mouseDown) goto abort_and_close;
		}

abort_and_close:


abort_drawing:	/******** !ARG! **********/

 	HUnlock (earthData);

	if (npictButton==1) {
		HidePen ();
		ClosePicture ();
	}
		
	ebmPict = OpenPicture (&eWindow->portRect);

	BackColor (backgroundC);
	ForeColor (backgroundC == blackColor ? whiteColor : blackColor);

	if (FALSE /*hasColorQD*/) {
		/*copy the pixMap, when I learn how (no MAC II around here!)*/
	} else {
		CopyBits (&eWindow->portBits, &eWindow->portBits, 
		 &eWindow->portRect, &eWindow->portRect, srcCopy, 0L);
	}

	ClosePicture ();

	if (nbitmapButton==1)
		ePict = ebmPict;

	InitCursor ();

	oldBackground = backgroundC;
	oldFore = backgroundC == blackColor ? whiteColor : blackColor;
	myDrawGrowIcon ();
}

xxx (xabs, yabs, zabs)	/* plot a line <-- Richard's Name! */
double	xabs, yabs, zabs;
{
register int		move;
register int		x, y;
static	int over = FALSE;
double	factor, xtmp, xrel, yrel, zrel;

	move = (! draw) || over;
	draw = ! move;
	xtmp = xabs * cos2 + yabs * sin2;
	xrel = xtmp * cos1 + zabs * sin1;
	over = (xrel < xmax);
/*	if (nhiddenlinesBox == 0) over = FALSE;	*/

	if (over) return;

	yrel = yabs * cos2 - xabs * sin2;
	zrel = zabs * cos1 - xtmp * sin1;
	
	factor = (-xpos) / (xrel - xpos);
	
	x = (((yrel * factor * scaler) + 1.0) * half_xsize) + 1 ;
	y = (((-zrel * factor * scaler) + 1.0) * half_ysize) + 1 ;
	
	if (move) MoveTo (x, y);
	if (draw) LineTo (x, y);
}

xxx_int (xabs, yabs, zabs)	/* plot a line or move current position */
long	xabs, yabs, zabs;
{
register	int		x, y;
register	long	ixtmp, ixrel, ifactor;
long		 iyrel, izrel;

	/*
	 * draw line if we want to draw (as opposed to move)
	 * and the previous point was drawn too.
	 */
	draw = draw && !over;

/*	ixtmp = ((xabs * icos2) + (yabs * isin2)) / fudge;
	ixrel = ((ixtmp * icos1) + (zabs * isin1)) / fudge;
asm for SPEED! */
asm {
		move.l	xabs, d0		/* 		xabs * icos2		*/
		move.l	icos2, d2
		muls.w	d2, d0
		move.l	yabs, d1		/*		yabs * isin2		*/
		move.l	isin2, d2
		muls.w	d2, d1
		add.l	d1, d0		/*		add these			*/
		divs.w	#fudge, d0	/*		/ fudge				*/
		ext.l	d0
		move.l	d0, ixtmp

	/*	move.l	xabs, d0				not required		*/
		move.l	icos1, d2	/* 		ixtmp * icos1		*/
		muls.w	d2, d0
		move.l	zabs, d1		/*		zabs * isin1		*/
		move.l	isin1, d2
		muls.w	d2, d1
		add.l	d1, d0		/*		add these			*/
		divs.w	#fudge, d0	/*		/ fudge				*/
		ext.l	d0
		move.l	d0, ixrel
	}

	over = (ixrel < ixmax);
 
	if (over) return;
 
/*	iyrel = ((yabs * icos2) - (xabs * isin2)) / fudge;
	izrel = ((zabs * icos1) - (ixtmp * isin1)) / fudge;
asm for SPEED! */
asm {
		move.l	yabs, d0		/* 		yabs * icos2		*/
		move.l	icos2, d2
		muls.w	d2, d0
		move.l	xabs, d1		/*		xabs * isin2		*/
		move.l	isin2, d2
		muls.w	d2, d1
		sub.l	d1, d0		/*		add these			*/
		divs.w	#fudge, d0	/*		/ fudge				*/
		ext.l	d0
		move.l	d0, iyrel

		move.l	zabs, d0		/* 		zabs * icos1		*/
		move.l	icos1, d2	
		muls.w	d2, d0
		move.l	ixtmp, d1	/*		ixtmp * isin1		*/
		move.l	isin1, d2
		muls.w	d2, d1
		sub.l	d1, d0		/*		add these			*/
		divs.w	#fudge, d0	/*		/ fudge				*/
		ext.l	d0
		move.l	d0, izrel
	}

	ifactor = ixpos * iscaler / (ixpos - ixrel);

	x = ((((iyrel * ifactor) / fudge) + fudge) * ihalf_xsize) / fudge + 1;
  	y = ((((-izrel * ifactor) / fudge) + fudge) * ihalf_ysize) / fudge + 1;

	if (draw)	LineTo (x, y);
	else		MoveTo (x, y);
}

DoEarthMenu (item)
int		item;
{
	switch (item) {
	case 1:
		DrawEarth ();
		break;
	case 2:
		setOptions ();
 		break;
	case 3:
		setColors ();
		break;
	}
}

DoEditMenu (item)
int		item;
{
long	result;

	switch (item) { 
	case 3: /* Cut */
	case 4: /* Copy */
		ZeroScrap ();
		HLock (ePict);
		result = PutScrap (GetHandleSize (ePict), 'PICT', *ePict);
		HUnlock (ePict);
		result = UnloadScrap ();
 		break;
 	case 6: /* clear */
 		ZeroScrap ();
 		break;
 	default:
 		SysBeep (3);
 		break;
	}
}

DoFileMenu (item)
int		item;
{
	SkelWhoa ();
}

eUpdate (Resized) /************************************************************/
Boolean	Resized;
{
	Rect	tr;

	if (Resized) {
		ClipRect (&eWindow->portRect);
		EraseRect (&eWindow->portRect);
	}
	DrawPicture (ebmPict, &eWindow->portRect);
	myDrawGrowIcon ();
}

eClose () /************************************************************/
{
	SkelWhoa ();
}

eClobber () /************************************************************/
{
GrafPtr	thePort;

	GetPort (&thePort);				/* grafport of window to dispose*/
	DisposeWindow ((WindowPtr) thePort);
}

eActivate () /************************************************************/
{
}

eMouse () /************************************************************/
{
}

eKey () /************************************************************/
{
}

myDrawGrowIcon ()
{
	Rect	tr;

	tr.bottom = eWindow->portRect.bottom - 2;
	tr.right  = eWindow->portRect.right  - 2;
	tr.top    = eWindow->portRect.bottom - 11;
	tr.left   = eWindow->portRect.right  - 11;

	BackColor (oldBackground);
	ForeColor (oldFore);
	FrameRect (&tr);
	
	OffsetRect (&tr, -3, -3);
	InsetRect (&tr, 1, 1);
	EraseRect (&tr);
	FrameRect (&tr);
}

epInit ()
{
	watchCursorHand = GetCursor (watchCursor);
	HLock (watchCursorHand);

	iconHandle = GetIcon (iconID);

	initGlobalRects ();

	ebmPict = GetPicture (110);
	ePict   = ebmPict;

	eWindow = NewWindow (nil, &earthWindowRect, "\pEarthView", 
						 true, 8, (WindowPtr)-1L, true, 0L);

	SkelWindow (eWindow, eMouse, eKey, eUpdate, eActivate, eClose, 
                		 eClobber, nil, false);

	cWindow = NewWindow (nil, &controlWindowRect, "\pSettings", 
						 true, documentProc, (WindowPtr)-1L, true, 0L);
	SkelWindow (cWindow, cMouse, cKey, cUpdate, cActivate, cClose, 
                		 cClobber, nil, false);

	latSB		= NewControl (cWindow, &latSBRect, "", true,  0, 0, 90, 16, 0L);	
	lonSB		= NewControl (cWindow, &lonSBRect, "", true,  0, 0, 180, 16, 0L);	
	altSB		= NewControl (cWindow, &altSBRect, "", true, 160, 1, 160, 16, 0L);	

	northCheck	= NewControl (cWindow, &northRect, "\pNorth", true, 1, 0, 1, 2, 0L);
	southCheck	= NewControl (cWindow, &southRect, "\pSouth", true, 0, 0, 1, 2, 0L);
	eastCheck	= NewControl (cWindow, &eastRect, "\pEast", true, 1, 0, 1, 2, 0L);
	westCheck	= NewControl (cWindow, &westRect, "\pWest", true, 0, 0, 1, 2, 0L);
	mileCheck	= NewControl (cWindow, &mileRect, "\pMiles", true, 1, 0, 1, 2, 0L);
	kmCheck		= NewControl (cWindow, &kmRect, "\pKilometers", true, 0, 0, 1, 2, 0L);

	npictButton		= 0;
	nbitmapButton	= 1;
	nintButton		= 1;
	nfpButton		= 0;
	nhiddenlinesBox	= 1;
	nlnlBox			= 0;
	nsquareBox		= 1;

	backgroundC			= whiteColor;
	earthbackgroundC	= whiteColor;
	earthoutlineC		= blackColor;
	latC				= redColor;
	longC				= redColor;
	landC				= blueColor;
}

initGlobalRects ()
{
	SetRect (&latSBRect, -1, 118, 164, 134);
	SetRect (&lonSBRect, -1, 198, 164, 214);
	SetRect (&altSBRect, -1, 279, 164, 295);

	SetRect (&latDataRect, 75, 62, 175, 82);
	SetRect (&lonDataRect, 82, 142, 175, 162);
	SetRect (&altDataRect, 73, 222, 175, 242);

	SetRect (&northRect,  5, 94, 74, 114);
	SetRect (&southRect, 75, 94, 145, 114);
	SetRect (&eastRect,  5, 174, 74, 194);
	SetRect (&westRect, 75, 174, 145, 194);

	SetRect (&mileRect, 5, 255, 65, 274);
	SetRect (&kmRect, 66, 254, 175, 274);

	SetRect (&iconRect, 10, 10, 42, 42);

	SetRect (&controlWindowRect,  12, 30+13, 175, 324+13);
	SetRect (&earthWindowRect, 195, 30+13, 499, 324+13);

}


main ()
{
MenuHandle	m;
Boolean		b;
EventRecord myevent;

	SkelInit ();

	/* when glue for SysEnvirons exists use it, but until then� */
	hasColorQD = ! (ROM85 & 0xC000); /* from TechNote 118 */

	b = GetNextEvent (everyEvent, &myevent); /* but override w/ OPTION key */
	if (myevent.modifiers && optionKey) {
		hasColorQD = true;
	}

	/*  I've left the color code in, but only as a basis for further work.  It should
		really be done using the new ColorQuickDraw calls, rather than the only stuff.
		The main problem now, is that the image is saved a a bitmap and this isn't
		good enough with CQD, we need to save it as a color PixMap.  Well, fodder for
		a V3.1!  ...mrp
	*/	hasColorQD = FALSE;

	SkelApple ("\pAbout Earthplot�", DoAbout);

	m = NewMenu (2, "\pFile");
	AppendMenu (m, "\pQuit/Q");
	SkelMenu (m, DoFileMenu, nil);

	m = NewMenu (3, "\pEdit");
	AppendMenu (m, "\pUndo/Z;(-;Cut/X;Copy/C;Paste/V;Clear");
	SkelMenu (m, DoEditMenu, nil);

	m = NewMenu (4, "\p\265EarthPlot");
	AppendMenu (m, "\pDraw Earth/D");
	AppendMenu (m, "\pOptions�/O");
	if (hasColorQD) 
		AppendMenu (m, "\pColors�/K");
	SkelMenu (m, DoEarthMenu, nil);
	DrawMenuBar ();

	epInit ();

	SkelMain ();		/* loop 'til Quit selected */

	SkelClobber ();		/* clean up */
}

setColors ()
{
	DialogPtr	colorsDialogPtr;
	int			itemHit;
	Rect	junkRect;
    int		tempKind;

ControlHandle
	backgroundButton, /* colors dialog items */
	earthbackgroundButton, 
	earthoutlineButton, 
	latButton, 
	longButton, 
	landmassButton, 
	
	IButton[6], 
	CButton[8];

int		i, 
		currentC;
long	c[8], 
		t[6];		/* colors temp storage */

int	currentT;

	c[0] = blackColor;
	c[1] = whiteColor;
	c[2] = redColor;
	c[3] = greenColor;
	c[4] = blueColor;
	c[5] = cyanColor;
	c[6] = magentaColor;
	c[7] = yellowColor;

	t[0] = backgroundC;
	t[1] = earthbackgroundC;
	t[2] = earthoutlineC;
	t[3] = latC;
	t[4] = longC;
	t[5] = landC;

	colorsDialogPtr = GetNewDialog (300, 0L, (WindowPtr)-1L);
	GetDItem (colorsDialogPtr, 11, &tempKind, &IButton[0], &junkRect);
	GetDItem (colorsDialogPtr, 12, &tempKind, &IButton[1], &junkRect);
	GetDItem (colorsDialogPtr, 13, &tempKind, &IButton[2], &junkRect);
	GetDItem (colorsDialogPtr, 14, &tempKind, &IButton[3], &junkRect);
	GetDItem (colorsDialogPtr, 15, &tempKind, &IButton[4], &junkRect);
	GetDItem (colorsDialogPtr, 16, &tempKind, &IButton[5], &junkRect);
	GetDItem (colorsDialogPtr, 3, &tempKind, &CButton[0], &junkRect);
	GetDItem (colorsDialogPtr, 4, &tempKind, &CButton[1], &junkRect);
	GetDItem (colorsDialogPtr, 5, &tempKind, &CButton[2], &junkRect);
	GetDItem (colorsDialogPtr, 6, &tempKind, &CButton[3], &junkRect);
	GetDItem (colorsDialogPtr, 7, &tempKind, &CButton[4], &junkRect);
	GetDItem (colorsDialogPtr, 8, &tempKind, &CButton[5], &junkRect);
	GetDItem (colorsDialogPtr, 9, &tempKind, &CButton[6], &junkRect);
	GetDItem (colorsDialogPtr, 10, &tempKind, &CButton[7], &junkRect);

	currentT = 0;

	do {
		for (i=0;i<6;i++)
			SetCtlValue (IButton[i], i == currentT);
		
		for (i=0;i<8;i++)
			SetCtlValue (CButton[i], c[i] == t[currentT]);

		ModalDialog (0L, &itemHit);
		
		switch (itemHit) {
		case 11:				/* "items" */
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
			currentT = itemHit - 11;
			break;
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			t[currentT] = c[itemHit - 3];
			break;
		}
	
	} while ((itemHit != 1/* OK */) && (itemHit != 2/* CANCEL */));

	DisposDialog (colorsDialogPtr);

	if (itemHit == 1) {
		backgroundC 	= t[0];
		earthbackgroundC= t[1];
		earthoutlineC 	= t[2];
		latC 			= t[3];
		longC 			= t[4];
		landC 			= t[5];
	}
}

setOptions ()
{
	DialogPtr	optionsDialogPtr;
	int			itemHit;
	Rect	junkRect;
    int		tempKind;

ControlHandle
	pictButton, /* options dialog items */
	bitmapButton, 
	intButton, 
	fpButton, 
	hiddenlineBox, 
	lnlBox, 
	squareBox;

int
	tpictButton, /* options dialog items */
	tbitmapButton, 
	tintButton, 
	tfpButton, 
	thiddenlinesBox, 
	tlnlBox, 
	tsquareBox;

	tpictButton		= npictButton;
	tbitmapButton	= nbitmapButton;
	tintButton		= nintButton;
	tfpButton		= nfpButton;
	thiddenlinesBox	= nhiddenlinesBox;
	tlnlBox			= nlnlBox;
	tsquareBox		= nsquareBox;

	optionsDialogPtr = GetNewDialog (200, 0L, (WindowPtr)-1L);
	GetDItem (optionsDialogPtr, 2, &tempKind, &pictButton, &junkRect);
	GetDItem (optionsDialogPtr, 3, &tempKind, &bitmapButton, &junkRect);
	GetDItem (optionsDialogPtr, 9, &tempKind, &intButton, &junkRect);
	GetDItem (optionsDialogPtr, 8, &tempKind, &fpButton, &junkRect);
	GetDItem (optionsDialogPtr, 5, &tempKind, &lnlBox, &junkRect);
	GetDItem (optionsDialogPtr, 13, &tempKind, &hiddenlineBox, &junkRect);
	GetDItem (optionsDialogPtr, 6, &tempKind, &squareBox, &junkRect);
	
	do {
		SetCtlValue (pictButton, tpictButton);
		SetCtlValue (bitmapButton, tbitmapButton);
		SetCtlValue (intButton, tintButton);
		SetCtlValue (fpButton, tfpButton);
		SetCtlValue (hiddenlineBox, thiddenlinesBox);
		SetCtlValue (lnlBox, tlnlBox);
		SetCtlValue (squareBox, tsquareBox);
	
		ModalDialog (0L, &itemHit);
		
		switch (itemHit) {
		case 2:						/* "picture" */
			tpictButton   = 1;
			tbitmapButton = 0;
			break;
		case 3:						/* "bitmap" */
			tpictButton   = 0;
			tbitmapButton = 1;
			break;
		case 9: 					/* "integer" */
			tintButton		= 1;
			tfpButton		= 0;
			break;
		case 8: 					/* "floating point" */
			tintButton		= 0;
			tfpButton		= 1;
			break;
		case 13:						/* "use hidden lines" */
			thiddenlinesBox	= (thiddenlinesBox) ? 0 : 1;
			break;
		case 5:						/* "draw lat & long" */
			tlnlBox			= (tlnlBox) ? 0 : 1;
			break;
		case 6:						/* "draw lat & long" */
			tsquareBox		= (tsquareBox) ? 0 : 1;
			break;
		}
	
	} while ((itemHit != 1/* OK */) && (itemHit != 12/* CANCEL */));

	DisposDialog (optionsDialogPtr);

	if (itemHit == 1) {
		npictButton		= tpictButton;
		nbitmapButton	= tbitmapButton;
		nintButton		= tintButton;
		nfpButton		= tfpButton;
		nhiddenlinesBox	= thiddenlinesBox;
		nlnlBox			= tlnlBox;
		nsquareBox		= tsquareBox;
	}
}

