/*-----------------------------------------------------------------------------------------
#
#	Apple Macintosh Developer Technical Support
#
#	Collection of Utilities for DTS Sample code
#
#	Program:	Utilities.c.o
#	File:		Utilities.c	-	C Source
#
#	Copyright © 1988-1990 Apple Computer, Inc.
#	All rights reserved.
#
-----------------------------------------------------------------------------------------*/
/* Given the button control handle, this will cause the button to look as if it
   has been clicked in. This is nice to do for the user if they type return or
   enter to select the default item. */

#define kDelayTime				8			/* For the delay time when flashing the
											   menubar and highlighting a button.
											   8/60ths of a second*/
#define kSelect					1			/* select the control */
#define kDeselect				0			/* deselect the control */

void SelectButton(ControlHandle);
void OutlineControl(ControlHandle);

void	SelectButton(ControlHandle button)
{
	long			finalTicks;

	HiliteControl(button, kSelect);
	Delay(kDelayTime, &finalTicks);
	HiliteControl(button, kDeselect);
}

/* Given any control handle, this will draw an outline around it.  This is used
  for the default button of a window.  The extra nice feature here is that I’ll
  erase the outline for buttons that are inactive.  Seems like there should be
  a Toolbox call for getting a control’s hilite state. Since there isn’t, I have
  to look into the control record myself. This should be called for update and
  activate events.

  The method for determining the oval diameters for the roundrect is a little
  different than that recommended by Inside Mac. IM I-407 suggests that you
  use a hardcoded (16,16) for the diameters. However, this only looks good for
  small roundrects. For larger ones, the outline doesn’t follow the inner
  roundrect because the CDEF for simple buttons doesn’t use (16,16). Instead,
  it uses half the height of the button as the diameter. By using this
  formula, too, our outlines look better.

  WARNING: This will set the current port to the control’s window. */

#ifndef NULL
#define NULL 0L
#endif
#define kButtonFrameInset	-4
#define kButtonFrameSize	3

void		OutlineControl(ControlHandle button)
{
	Rect		theRect;
	PenState	curPen;
	short		buttonOval;

	if ( button != NULL ) {
		SetPort((**button).contrlOwner);
		GetPenState(&curPen);
		PenNormal();
		theRect = (**button).contrlRect;
		InsetRect(&theRect, kButtonFrameInset, kButtonFrameInset);
		buttonOval = (theRect.bottom - theRect.top) / 2;
#if 0
#ifdef applec
		PenPat((**button).contrlHilite == kCntlActivate ? &qd.black : &qd.gray);
#else
		PenPat((**button).contrlHilite == kCntlActivate ? &black : &gray);
#endif
#endif
		PenSize(kButtonFrameSize, kButtonFrameSize);
		FrameRoundRect(&theRect, buttonOval, buttonOval);
		SetPenState(&curPen);
	}
}

