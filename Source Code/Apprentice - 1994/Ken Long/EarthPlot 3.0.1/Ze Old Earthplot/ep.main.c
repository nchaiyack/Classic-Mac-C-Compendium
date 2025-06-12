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

***/

/*** Earthplot V2.0
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
***/
 
/**
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
 ** Based on Microplot program written in Fortran by Richard Heurtley
 ** (This can be found in file B900:MICROPLOT.FTN on the MTS system 
 **  at Rensselear Polytechnic Institute in Troy, New York)
 **
 ** Who	When			What
 ** ===	===========		====================================
 ** mrp	01-dec-1985		Final work done for version 1.0
 ** mkg 20-dec-1985		Performance improvements
 ** mrp 15-aug-1987		V3.0 in Lightspeed C w/ TSkel
 **/
 
/** [2.0 notes]
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
 **/
 
/** [1.0 notes]
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

#include	"ep.const.h"
#include	"ep.extern.h"
#include	<MenuMgr.h>
#include	<DialogMgr.h>
#include	"EventMgr.h"

extern	DoAbout();
extern	DoFileMenu();
extern	DoEditMenu();
extern	DoEarthMenu();

WindowPtr	cWindow;
WindowPtr	eWindow;

main ()
{
MenuHandle	m;
Boolean		b;
EventRecord myevent;

	SkelInit ();

	/* when glue for SysEnvirons exists use it, but until then… */
	hasColorQD = !(ROM85 & 0xC000); /* from TechNote 118 */

	b = GetNextEvent (everyEvent, &myevent); /* but override w/ OPTION key */
	if (myevent.modifiers && optionKey) {
		hasColorQD = TRUE;
	}

	/*  I've left the color code in, but only as a basis for further work.  It should
		really be done using the new ColorQuickDraw calls, rather than the only stuff.
		The main problem now, is that the image is saved a a bitmap and this isn't
		good enough with CQD, we need to save it as a color PixMap.  Well, fodder for
		a V3.1!  ...mrp
	*/	hasColorQD = FALSE;

	SkelApple ("\pAbout Earthplot…", DoAbout);

	m = NewMenu (2, "\pFile");
	AppendMenu  (m, "\pQuit/Q");
	SkelMenu    (m, DoFileMenu, nil);

	m = NewMenu (3, "\pEdit");
	AppendMenu  (m, "\pUndo/Z;(-;Cut/X;Copy/C;Paste/V;Clear");
	SkelMenu    (m, DoEditMenu, nil);

	m = NewMenu(4, "\p\265EarthPlot");
	AppendMenu (m, "\pDraw Earth/D");
	AppendMenu (m, "\pOptions…/O");
	if (hasColorQD) 
		AppendMenu (m, "\pColors…/K");
	SkelMenu   (m, DoEarthMenu, nil);
	DrawMenuBar();

	epInit();

	SkelMain ();		/* loop 'til Quit selected */

	SkelClobber ();		/* clean up */
}
