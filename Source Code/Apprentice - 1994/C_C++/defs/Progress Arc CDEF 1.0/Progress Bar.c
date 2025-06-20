/*****************************************************************

Legal stuff:

Progress Bar CDEF 1.2 by Eddy J. Gurney. This CDEF is copyright �1993�94 by Eddy J. Gurney
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

This CDEF allows you to very easily use progress bars, ala the Finder's "Copying" dialog.
To use it, copy the CDEF to your project's resource file. Add a "CNTL" to your dialog, and set
the control's ProcID to:
	16*the resource ID you gave this CDEF (default ID is 600) + variation code (see below).
Next set the initial, minimum and maximum values. (ie., 0, 0, 100 for a "percentage" bar).
Finally, if you would like to use custom colors for the progress bar, use a variation code of
1 instead of 0 and and edit the colors (stored in a corresponding 'cctb' resource) as desired.
(FrameColor is used for the frame, BodyColor is used as the background color [never
really used], TextColor is used for the "done" part of the bar, and ThumbColor is used for the
"to do" part of the bar. This is the same order as they appear in Resorcerer's "CNTL" editor,
so you don't have to edit the 'cctb' directly.) If you use custom controls, make sure to try
them at various bit depths to make sure QuickDraw maps the done/to do parts to different
colors (the default colors are OK).

After that, in your program all you have to do is call SetCtlValue() and this CDEF will 
automagically draw your progress bar!

!!WARNING!! This CDEF assumes the presence of the _Gestalt trap (available in System 7 or
later.) I didn't want to include the glue code to emulate _Gestalt under System 6 (to keep the
size down).  If you need to use this in an application that may be running under System 6,
just define SystemSevenOrLater to 0 instead of 1 and rebuild the code resource.

If someone knows of a *supported* way to check for Color QuickDraw without calling _Gestalt,
let me know and I'll include it in the next release. (_SysEnvirons turned out to be just as bad
since Think adds glue code to emulate SysEnvirons on system versions earlier than 4.1 or
something�)

Thanks go to Joe Zobkiw's for his "Finder Progress" sample code (posted to alt.sources.mac by
Ken Long) and to Scott Knaster/Keither Rollin for their "Progress Indicator" sample in Macintosh
Programming Secrets. They made this much easier!

P.S. I code in Geneva 9, so if this looks unformatted, that's why. It will look fine in any window
about 640 pixels wide when viewed in Geneva 9. :-)

E-mail comments, suggestions, requests to: egurney@vcd.hp.com
 
1.1: First public release
1.2 changes:
 	. Call StripAddress() instead of &'ing off the lower 3 bytes in calcCRgns (thanks Dave Polaschek).
 	. Now determine whether or not to use custom colors via the variation code (thanks Steve LoBasso)
 	. Added capability to do vertical progress bars for "thermometers". If the progress bar
 	  rectangle is taller than it is wide, then the progress bar will fill "up" instead of "across".
 	. Fixed a bug that prevented proper drawing when a control had a non-zero minimum.
 
******************************************************************/

#ifdef SystemSevenOrLater
#undef SystemSevenOrLater
#endif

#define SystemSevenOrLater 1

#include <GestaltEqu.h>
#include <SetUpA4.h>

const unsigned char copyright[] = "Progress Bar CDEF 1.2 �1993�94 Eddy J. Gurney/TEG. Freeware.";

struct ControlData {
	unsigned	hasColorQD	: 1;
	unsigned	isVertical		: 1;
};

pascal long main(short variation, ControlHandle theControl, short msg, long param)
{
	/* These need to be declared static to work. See the Note on p. 283 of Think C 6 User's Guide */	
	const static RGBColor	kBlack = { 0, 0, 0 };
	const static RGBColor	kWhite = { 0xffff, 0xffff, 0xffff };
	const static RGBColor	kDarkGray = { 0x4000, 0x4000, 0x4000 };
	const static RGBColor	kSteelBlue= { 0xcccc, 0xcccc, 0xffff };
	long					result;
	union {
		unsigned int			iControl;
		struct ControlData		sControl;
	} control;

	RememberA0();
	SetUpA4();

	result = 0;

	switch (msg) {
		case initCntl: {
			long			gestaltResponse;
			Rect			tempRect;

			(void)Gestalt(gestaltQuickdrawVersion, &gestaltResponse);
			control.sControl.hasColorQD = (gestaltResponse >= gestalt8BitQD);
			
			tempRect = (*theControl)->contrlRect;
			if ((tempRect.right - tempRect.left) >= (tempRect.bottom - tempRect.top))
				control.sControl.isVertical = false;
			else
				control.sControl.isVertical = true;
			
			(*theControl)->contrlData = (Handle)control.iControl;
			break;
		}

		case drawCntl: {
			PenState		oldPenState;
			Boolean		hasColorQD;
			Boolean		isVertical;
			Rect			doneRect, toDoRect;
			RGBColor		oldForeColor, oldBackColor;
			AuxCtlHandle	auxControlData;
			short		length;

			if ((*theControl)->contrlVis) {
				control.iControl = (unsigned int)(*theControl)->contrlData;
				hasColorQD = control.sControl.hasColorQD;
				isVertical = control.sControl.isVertical;

				GetPenState(&oldPenState);
				PenNormal();
	
				doneRect = (*theControl)->contrlRect;

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
				FrameRect(&doneRect);	/* Frame the progress bar in the frame color */

				InsetRect(&doneRect,1, 1);
				toDoRect = doneRect;	/* Copy the rect before we change it*/
				
				/* Determine how much of the progress bar to draw, making sure we don't divide by zero */
				if (length = ((*theControl)->contrlMax - (*theControl)->contrlMin)) {
					if (isVertical) {
						doneRect.top = doneRect.bottom - 
									((long)((*theControl)->contrlValue - (*theControl)->contrlMin) *
									(doneRect.bottom - doneRect.top)) / length;					
					} else {
						doneRect.right = doneRect.left + 
									((long)((*theControl)->contrlValue - (*theControl)->contrlMin) *
									(doneRect.right - doneRect.left)) / length;
					}
				}

				if (hasColorQD) {
					if (variation == 1 && auxControlData != nil)
						RGBForeColor(&(*(*auxControlData)->acCTable)->ctTable[cTextColor].rgb);
					else
						RGBForeColor(&kDarkGray);
				} /* If no Color QD, we're still drawing in black */
				PaintRect(&doneRect);	/* Draw the "done" part */ 

				if (isVertical) {
					toDoRect.bottom = doneRect.top - 1;
				} else {
					toDoRect.left = doneRect.right + 1;	/* Calculate the "to do" area rectangle */
				}
				if (hasColorQD) {
					/* QD checks to see if the foreground color maps into the same color as the background
					     color. If so, QD inverts the foreground color and draws with it instead. This is BAD for
					     us! If the user is in 1-bit mode, "Steel Blue" will map to white, which QD sees is the
					     same as the background color, and sets it to black instead� and the progress bar
					     becomes "invisible"! So we "trick" QD by changing the background color, which doesn't
					     matter since we never draw with it anyway. Thanks to Knaster & Rollin (Macintosh
					     Programming Secrets) for describing this behavior� */
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
				PaintRect(&toDoRect);	/* Draw the "to do" part */

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
