/*****************************************************************

Legal stuff:

Progress Arc CDEF 1.0 by Eddy J. Gurney. This CDEF is copyright ©1993Ð4 by Eddy J. Gurney
and The Eccentricity Group, but is "freeware". It may be freely distributed, modified, included
in any application, on a source code compilation CD-ROM, etc. as long as the following simple
restrictions are met:
	1) this entire copyright notice remains intact when this source is redistributed, modified, etc.
	2) the copyright notice in the CDEF itself (the "copyright" string) remains intact,
	3) this source, or any of its derivitave works (including the CDEF itself), are NOT made
	    available on or used in any form by any electronic service (or associated electronic
	    service) which restricts the electronic distribution of some or all of its files (ie, ZiffNet).
Feel free to add your name, company, whatever to the copyright. However you can NOT remove
the restrictions I've described above or add further restrictions on its use or distribution.
Finally, no warranty of any kind is expressed or implied.

Usage:

This CDEF allows you to very easily display a circular progress indicator from your application.
To use it, copy the CDEF to your project's resource file. Add a "CNTL" to your dialog, and set
the control's ProcID to:
	16*the resource ID you gave this CDEF (default ID is 601) + variation code (see below).
Next set the initial, minimum and maximum values. (ie., 0, 0, 100 for a "percentage" arc).
Finally, if you would like to use custom colors for the progress arc, use a variation code of
1 instead of 0 and and edit the colors (stored in a corresponding 'cctb' resource) as desired.
(FrameColor is used for the frame, BodyColor is used as the background color [never
really used], TextColor is used for the "done" part of the arc, and ThumbColor is used for the
"to do" part of the arc. This is the same order as they appear in Resorcerer's "CNTL" editor,
so you don't have to edit the 'cctb' directly.) If you use custom controls, make sure to try
them at various bit depths to make sure QuickDraw maps the done/to do parts to different
colors (the default colors are OK).

After that, in your program all you have to do is call SetCtlValue() and this CDEF will 
automagically draw your "progress arc"!

!!WARNING!! This CDEF assumes the presence of the _Gestalt trap (available in System 7 or
later.) I didn't want to include the glue to emulate _Gestalt under System 6 (to keep the
size down).  If you need to use this in an application that may be running under System 6,
just define SystemSevenOrLater to 0 instead of 1 and rebuild the code resource.

If someone knows of a "supported" way to check for Color QuickDraw without calling _Gestalt,
let me know and I'll include it in the next release.

P.S. I code in Geneva 9, so if this looks unformatted, that's why. It will look fine in any window
about 640 pixels wide when viewed in Geneva 9. :-)

E-mail comments, suggestions, requests to: egurney@vcd.hp.com
 
1.0: First public release

******************************************************************/

#ifdef SystemSevenOrLater
#undef SystemSevenOrLater
#endif

#define SystemSevenOrLater 1

#include <GestaltEqu.h>
#include <SetUpA4.h>

const unsigned char copyright[] = "Progress Arc CDEF 1.0 ©1993Ð94 Eddy J. Gurney/TEG. Freeware.";

pascal long main(short variation, ControlHandle theControl, short msg, long param)
{
	/* These need to be declared static to work. See the Note on p. 283 of Think C 6 User's Guide */	
	const static RGBColor	kBlack = { 0, 0, 0 };
	const static RGBColor	kWhite = { 0xffff, 0xffff, 0xffff };
	const static RGBColor	kDarkGray = { 0x4000, 0x4000, 0x4000 };
	const static RGBColor	kSteelBlue= { 0xcccc, 0xcccc, 0xffff };
	long					result;

	RememberA0();
	SetUpA4();

	result = 0;

	switch (msg) {
		case initCntl: {
			long			gestaltResponse;

			(void)Gestalt(gestaltQuickdrawVersion, &gestaltResponse);
			(*theControl)->contrlData = (Handle)(gestaltResponse >= gestalt8BitQD);
			break;
		}

		case drawCntl: {
			PenState		oldPenState;
			Boolean		hasColorQD = (Boolean)(*theControl)->contrlData;
			RGBColor		oldForeColor, oldBackColor;
			Rect			arcRect;
			AuxCtlHandle	auxControlData;
			short		length, doneArcDegree, toDoArcDegree;

			if ((*theControl)->contrlVis) {
				GetPenState(&oldPenState);
				PenNormal();
	
				arcRect = (*theControl)->contrlRect;
	
				if (hasColorQD) {
					GetForeColor(&oldForeColor);
					GetBackColor(&oldBackColor);
					/* Check if we should use custom colors, or the default Finder colors */
					if (variation == 1 && GetAuxCtl(theControl, &auxControlData)) {
						RGBForeColor(&(*(*auxControlData)->acCTable)->ctTable[cFrameColor].rgb);
						RGBBackColor(&(*(*auxControlData)->acCTable)->ctTable[cBodyColor].rgb);
					} else {
						auxControlData = nil;
						RGBForeColor(&kBlack);
						RGBBackColor(&kWhite);
					}
				} /* If no Color QD, we'll be drawing in black because of our call to PenNormal above */
				FrameOval(&arcRect);	/* Frame the progress arc in the frame color */

				InsetRect(&arcRect, 1, 1);
				
				/* Determine how much of the progress bar to draw, making sure we don't divide by zero */
				if (length = ((*theControl)->contrlMax - (*theControl)->contrlMin)) {
					doneArcDegree = ((long)((*theControl)->contrlValue - (*theControl)->contrlMin) * 360) / length;
				}
				toDoArcDegree = 360 - doneArcDegree;

				if (hasColorQD) {
					if (variation == 1 && auxControlData != nil)
						RGBForeColor(&(*(*auxControlData)->acCTable)->ctTable[cTextColor].rgb);
					else
						RGBForeColor(&kDarkGray);
				} /* If no Color QD, we're still drawing in black */
				PaintArc(&arcRect, 0, doneArcDegree);		/* Draw the "done" part */ 

				if (hasColorQD) {
					/* QD checks to see if the foreground color maps into the same color as the background
					     color. If so, QD inverts the foreground color and draws with it instead. This is BAD for
					     us! If the user is in 1-bit mode, "Steel Blue" will map to white, which QD sees is the
					     same as the background color, and sets it to black insteadÉ and the progress bar
					     becomes "invisible"! So we "trick" QD by changing the background color, which doesn't
					     matter since we never draw with it anyway. Thanks to Knaster & Rollin (Macintosh
					     Programming Secrets) for describing this behaviorÉ */
					if (variation == 1 && auxControlData != nil) {
						RGBForeColor(&(*(*auxControlData)->acCTable)->ctTable[cThumbColor].rgb);
						RGBBackColor(&(*(*auxControlData)->acCTable)->ctTable[cFrameColor].rgb);
					} else {
						RGBForeColor(&kSteelBlue);
						RGBBackColor(&kBlack);
					}
				} else { /* If no Color QD, we switch to white via patBic (force white where pattern black) mode */
					PenMode(patBic);
				}
				PaintArc(&arcRect, doneArcDegree, toDoArcDegree);	/* Draw the "to do" part */

				if (hasColorQD) {
					RGBForeColor(&oldForeColor);
					RGBBackColor(&oldBackColor);
				}
				SetPenState(&oldPenState);
			}
			break;
		}
		
		case calcCRgns:	/* This is the message sent if using 24-bit addressing */
			param = (long)StripAddress((Ptr)param);		/* Mask off the high byte if necessary */
			/* Fall through on purpose! */

		case calcCntlRgn:	/* Under 32-bit addressing we get this message */
			RectRgn((RgnHandle)param, &(*theControl)->contrlRect);
			break;

	#ifdef FOR_COMPLETENESS
		case testCntl:
		case dispCntl:
		case posCntl:
		case thumbCntl:
		case dragCntl:
		case autoTrack:
		case undoDev:
		case cutDev:
		case copyDev:
		case pasteDev:
		case clearDev:
		case cursorDev:
		case calcThumbRgn:
			break;	/* We don't respond to any of these messages */
	#endif

		default:	/* Any other messages we don't handle or which aren't defined yet */
			break;
	}

	RestoreA4();

	return result;
}
