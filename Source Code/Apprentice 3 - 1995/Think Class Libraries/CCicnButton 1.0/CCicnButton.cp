/******************************************************************************
 CCicnButton.cp

		An icon pane that acts like a button. It can have
		push button, checkbox or radio button personalities.
				
	SUPERCLASS = CIconPane
	
	Copyright � 1993 Martin R. Wachter. All rights reserved.
	Portions Copyright � 1993 Object Factory Incorporated. All rights reserved.

	Original Author: 	Martin R. Wachter		email:	mrw@welchgate.welch.jhu.edu
	
	Created:			12/4/93					by:		mrw			TCL Version:	1.1.3

	Version change history:
	
	1.0		Initial release.

	Modified 	12/4/93 by Martin R. Wachter (mrw)
	
				Changed the way the icon is drawn by initializing the frame and
				aperature in ICicnButtonX() to that of the iconBMap.bounds rect.
								
				Added itsLabels STR32s and itsColors RGBColors to dsplay a text label
				centered inside the cicn using draw string.  You can set
				the RGBColor of the text for each of the 4 states using 
				SetLabelColors().
				
				Added SetEnable() and SetDisable() methods to tell CPane
				to draw us grayed out if we are inactive and can not accept
				clicks.
				
				Added AdjustCursor Method to display a pointing finger cursor
				if we are active and enabled.  Added itsPointerHand data member
				to hold a CursHandle to the cursor.
				
				Moved ResetIconGroup() into this class so that it is not dependent
				upon CSwissArmyButton().

	0.1		Initial release by Bob Foster (ObjectFactory - CIconButton)
				
******************************************************************************/

#include "CCicnButton.h"

#include <OSChecks.h>
#include <Commands.h>
#include <Global.h>
#include <OSChecks.h>
#include <CPaneBorder.h>
#include <CControl.h>
#include <CList.h>
#include <CWindow.h>
#include <CEditText.h>
#include <CColorTextEnvirons.h>

extern RgnHandle	gUtilRgn;		/* Utility region					*/
extern RGBColor		gRGBBlack;		/* Black color for everyone to use	*/
extern RGBColor		gRGBWhite;		/* White color for everyone to use	*/

/*
 *	TCL 1.1.2 DLP 12/10/91
 *	When available, we now use the System 7 Icon Utilities for drawing cicns
 *  See TN #306 for more information
 */

#define tcl_IconDispatch	0xABC9

#define kPointerHandCursor 1024

pascal OSErr tcl_PlotCIconHandle( Rect*, short, short, CIconHandle) = { 0x303C, 0x61F, tcl_IconDispatch};

/******************************************************************************
 ICicnButton
 
 	Initialize an CicnButton. The button may use up to four CICN (ICON)
 	icons, to represent the off, off/hilite, on, on/hilite states.
 	If hiliting is done by dimming and/or if the on state is indicated
 	by a border, the IDs for the unused states should be set the same
 	as the iconID. Other states can be duplicated, as well. For example,
 	the hilite states may be graphically the same for both on and off.
******************************************************************************/

void CCicnButton::ICicnButton(CView *anEnclosure, CBureaucrat *aSupervisor,
					short aHEncl, short aVEncl,
					SizingOption aHSizing, SizingOption aVSizing,
					short aButtonKind, short hilightStyle, short onStyle,
					short offID, short offHiliteID, short onID, short onHiliteID)
{
	short		i;
	Rect		r;
	
	CIconPane::IIconPane(anEnclosure, aSupervisor,
					aHEncl, aVEncl,
					aHSizing, aVSizing,
					offID, TRUE);

	wantsClicks = TRUE;

	buttonKind = aButtonKind;
	colorHilite = hilightStyle;
	outlineOn = onStyle;
	value = 0;
	SetGroupID(0);
	if (outlineOn)
		FixupBorder();

	ICicnButtonX(offHiliteID, onID, onHiliteID);

}


/******************************************************************************
 ICicnButtonX
 
 	Fill in the icon arrays
******************************************************************************/

void CCicnButton::ICicnButtonX(short offHiliteID, short onID, short onHiliteID)
{
short	i;

		// Copy the first icon to array

	itsIconID[0] = iconID;
	BlockMove(&icon, &itsIcon[0], kIconBytes);
	itsCicnH[0] = cicnH;
	
	// size the frame to that of the cicn's size		added 12/93 mrw
	width = height = 0;
	ResizeFrame(&((**cicnH).iconPMap).bounds);
	CalcAperture();
	
		// Now do the rest

	SetStateIcons(offHiliteID, onID, onHiliteID);

	itsPointerHand = GetCursor(kPointerHandCursor);
	HNoPurge( (Handle) itsPointerHand);
	
	// initialize itsLabels to empty strings
	for (i=0;i<4;i++){
		CopyPString("\p",itsLabels[i]);
	}
	
	
	// initialize label colors to black and white
	SetLabelColors(gRGBWhite,gRGBBlack,gRGBWhite,gRGBBlack);
	
	// initialize itsShadow to draw NO shadow
	itsShadow = 0;
}


/******************************************************************************
 Dispose	{OVERRIDE}
 
 	Free the color icon if it exists
******************************************************************************/

void  CCicnButton::Dispose(void)
{
	short		i, j;

		// Dispose all the CICN handles here, making sure not
		// to dispose duplicated items twice.

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < i; j++)
			if (itsCicnH[i] == itsCicnH[j])
				break;
		if (j == i && itsCicnH[i])
			DisposCIcon(itsCicnH[i]);
	}

		// Keep CIconPane from trying to dispose again

	cicnH = NULL;

	CIconPane::Dispose();
}


/******************************************************************************
 SetStateIcons
 
 	Add the icons used by the various states. The icon
 	specified in initialization is taken to be the icon for the
 	off, non-hilite state. If a state has no corresponding icon,
 	the ID of a previous state should be specified.
******************************************************************************/

void CCicnButton::SetStateIcons(short offHiliteID, short onID, short onHiliteID)
{
	short		i, j;

	itsIconID[1] = offHiliteID;
	itsIconID[2] = onID;
	itsIconID[3] = onHiliteID;

	for (i = 1; i < 4; i++)
	{
		for (j = 0; j < i; j++)
			if (itsIconID[j] == itsIconID[i])
				break;
		if (j != i)						// If duplicate icon ID
		{
				// Same as previous ID, so just copy the info

			BlockMove(&itsIcon[j], &itsIcon[i], kIconBytes);
			itsCicnH[i] = itsCicnH[j];
		}
		else
		{
				// If not the same as a previous icon ID, let
				// initialization method read the resource

			iconID = itsIconID[i];
			CIconPane::IIconPaneX(TRUE);
			BlockMove(&icon, &itsIcon[i], kIconBytes);
			itsCicnH[i] = cicnH;
		}
	}
	iconID = itsIconID[0];				// Restore this in case anyone looks
}

/******************************************************************************
 SetLabelColors
 
 	Set the RGB colors of the labels.

******************************************************************************/

void CCicnButton::SetLabelColors(RGBColor offColor, RGBColor offHiliteColor, 
								 RGBColor onColor, RGBColor onHiliteColor)

{
	itsColors[kSAOffState] = offColor;
	itsColors[kSAOffHiliteState] = offHiliteColor;
	itsColors[kSAOnState] = onColor;
	itsColors[kSAOnHiliteState] = onHiliteColor;

}
/******************************************************************************
 SetLabel
 
 	Initializes itsLabels to use strings from an STR# resource.
 	aShadow is used to to set itsShadow, for drawing a drop shadow
 	behing the label.
 	
******************************************************************************/

void CCicnButton::SetLabel(short strListID, 
							short offIndex, short offHiliteIndex, short onIndex, short onHiliteIndex,
							short fontNumber, short fontFace, short fontSize, short aShadow)
{
	Str255		theString;
	short		i;
	TextInfoRec	textInfo;
	CColorTextEnvirons		*itsTextEnv;
	
	itsTextEnv = new CColorTextEnvirons;
	itsTextEnv->IColorTextEnvirons();
    textInfo.fontNumber = fontNumber;
    textInfo.theSize = fontSize;
    textInfo.theStyle = fontFace;
    textInfo.theMode = 0;
    
	itsTextEnv->SetTextInfo(&textInfo);
	
	itsEnvironment = itsTextEnv;

	GetIndString(theString,strListID,offIndex);
	CopyPString(theString,itsLabels[0]);
	GetIndString(theString,strListID,offHiliteIndex);	
	CopyPString(theString,itsLabels[1]);
	GetIndString(theString,strListID,onIndex);	
	CopyPString(theString,itsLabels[2]);
	GetIndString(theString,strListID,onHiliteIndex);
	CopyPString(theString,itsLabels[3]);
		
	itsShadow = aShadow;
}

/******************************************************************************
 SetShadow
 
 	Sets the itsShadow data member for drawing a drop shadow behind itsLabels.
 	
******************************************************************************/

void CCicnButton::SetShadow(short aShadow)
{
	itsShadow = aShadow;
}

/******************************************************************************
 GetShadow
 
 	Returns the current value of the itsShadow data member.
 	
******************************************************************************/

short CCicnButton::GetShadow(short aShadow)
{
	return itsShadow;
}

/******************************************************************************
 CalcDrawState
 
 	Draw the icon normally or hilited. The CIconPane method does most
 	of the drawing.
******************************************************************************/

short CCicnButton::CalcDrawState(Boolean fHilite)
{
	return value * (outlineOn ? 0 : 2) + (colorHilite ? 0 : fHilite);
}

/******************************************************************************
 SetEnable
 
 	Enable an CicnButton.  Sets the icon to accept clicks.
 	
******************************************************************************/

void CCicnButton::SetEnable()
{
	if (!enabled){
		enabled = TRUE;
		SetWantsClicks(TRUE);
		Refresh();
	}
}

/******************************************************************************
 SetDisable
 
 	Disables an CicnButton.  Sets the icon to not accept clicks and to be
 	drawn in gray.
 	
******************************************************************************/

void CCicnButton::SetDisable()
{
	if (enabled){
		enabled = FALSE;
		SetWantsClicks(FALSE);
		Refresh();
	}
}

/******************************************************************************
 GetEnable
 
 	Returns the current enable data member for an icon button.
 	
******************************************************************************/

short CCicnButton::GetEnable()
{
	return enabled;
}


/******************************************************************************
 DrawIcon
 
 	Draw the icon normally or hilited. The CIconPane method does most
 	of the drawing.
******************************************************************************/

void CCicnButton::DrawIcon(Boolean fHilite)
{
	short	drawState = CalcDrawState(fHilite);

	Rect		tempRect,theBounds = { 0, 0, kIconPixels, kIconPixels};
	short		copyMode = fHilite ? srcOr : srcBic;
	RGBColor	fore, back;
	
	
		// Set up the correct icon

	cicnH = itsCicnH[drawState];
	BlockMove(&itsIcon[drawState], &icon, kIconBytes);
	Prepare();

	if (gSystem.hasColorQD)	// preserve foreground and background colors
	{
		GetForeColor( &fore);
		GetBackColor( &back);
	}
	ForeColor( blackColor);	// set default colors for CopyBits
	BackColor( whiteColor);	
		
	if (cicnH)
	{
		theBounds = ((**cicnH).iconBMap).bounds;

		// To draw the cicn highlighted, we make a copy of the cicn's
		// pixmap color table and "dim" it by modifying the RGB values.	
		if (fHilite && colorHilite)
		{
			CTabHandle	ctab = (**cicnH).iconPMap.pmTable, dupCTable;
			short		i, numColors;
			ColorSpec	*cspec;

			dupCTable = ctab;
			
				// let this memory allocation come from the reserve if necessary.
				// we'll be giving it back almost immediately.
			HandToHand((Handle *) &dupCTable);
			
			numColors = (**ctab).ctSize; // number of colors in cicn's color table
			cspec = (**ctab).ctTable;	// point to first ColorSpec in the color table
			for(i = 0; i < numColors; i++)
			{
				cspec->rgb.red /= 2;	// dim by halving the RGBs
				cspec->rgb.green /= 2;
				cspec->rgb.blue /= 2;
				++cspec;
			}
			PlotCIcon( &theBounds, cicnH); // plot it dimmed
			
				// put back the undimmed color table, then free the dimmed one.
			(**cicnH).iconPMap.pmTable = dupCTable;
			DisposCTable( ctab);
		}
		else{	// no dimming, just plot it normally
			
			PlotCIcon( &theBounds, cicnH);
			DrawLabel( &theBounds,drawState);
		}
	}
	else	// draw the b&w icon
	{
		BitMap  theImage;
		Boolean wasLocked = Lock( TRUE);
		theImage.baseAddr = icon;
		theImage.rowBytes = kIconRowBytes;
		
		theImage.bounds = theBounds;
				
		CalcMask( icon, cIconMaskBits, kIconRowBytes, kIconRowBytes, kIconPixels, 2);

		CopyBits( &cMaskBitMap, &thePort->portBits, &theBounds, &theBounds, copyMode, NULL);
		CopyBits( &theImage, &thePort->portBits, &theBounds, &theBounds, srcXor, NULL); 
	 
		Lock( wasLocked);	
	}
	if (gSystem.hasColorQD)	// restore the colors
	{
		RGBForeColor( &fore);
		RGBBackColor( &back);
	}

}

/******************************************************************************
 DrawLabel
 
 	Draw the text label for the button and optionaly draw itsShadow.
 	
******************************************************************************/

void CCicnButton::DrawLabel(Rect *fBounds, short state)
{
	Str32		label;
	short		iWidth;
	FontInfo	fInfo;
	
	CopyPString(itsLabels[state], label);

	iWidth = StringWidth(label);
	GetFontInfo(&fInfo);
	
	TextMode(srcOr);
	
	if (itsShadow){

		// draw the background shaddow
		switch(state){
			case kSAOffState:
			case kSAOnState:
				RGBForeColor( &gRGBBlack);
				break;
			
			case kSAOffHiliteState:
			case kSAOnHiliteState:
				RGBForeColor( &gRGBWhite);
				break;
		}
	
		MoveTo((((*fBounds).right - iWidth)/2)+itsShadow,(((*fBounds).bottom/2 + fInfo.ascent/2) - fInfo.descent)+itsShadow);
		DrawString(label);
	}
	
	// draw the text on top of the label
	RGBForeColor( &itsColors[state]);
	MoveTo(((*fBounds).right - iWidth)/2,((*fBounds).bottom/2 + fInfo.ascent/2) - fInfo.descent);
	DrawString(label);
		

}

/******************************************************************************
 ChangeValue
 
 	Change the value for an CicnButton. Like SetValue, with no
 	Refresh.
******************************************************************************/

void CCicnButton::ChangeValue(short aValue)
{
	if (aValue != value)
	{
		if (aValue)						// Interact with the group as approp.
			ResetIconGroup();

		value = aValue;					// OK to change value now

										// Tell dependents, just like any
										//	 other control
		BroadcastChange(controlValueChanged, &aValue);

		FixupBorder();					// Border changes based on value
	}
}


/******************************************************************************
 SetValue
 
 	Set the value for an CicnButton
******************************************************************************/

void CCicnButton::SetValue(short aValue)
{
	if (buttonKind == kSAPushButton)	// Can't change value of pushbutton
		return;

	aValue = aValue & 1;				// Only values 0 and 1 are allowed

	if (aValue != value)
	{
		ChangeValue(aValue);
		if (!outlineOn)
			Refresh();
	}
}


/******************************************************************************
 GetValue
 
 	Get the value for an CicnButton
******************************************************************************/

short CCicnButton::GetValue()
{
	return value;
}


/******************************************************************************
 SetGroupID
 
 	Set the group ID
******************************************************************************/

void CCicnButton::SetGroupID(short anID)
{
	groupID = anID;
}


/******************************************************************************
 GetGroupID
 
 	Get the group ID
******************************************************************************/

short CCicnButton::GetGroupID()
{
	return groupID;
}


/******************************************************************************
 SetButtonKind
 
 	Set the button action
******************************************************************************/

void CCicnButton::SetButtonKind(short aKind)
{
	buttonKind = aKind;
}


/******************************************************************************
 GetButtonKind
 
 	Get the button action
******************************************************************************/

short CCicnButton::GetButtonKind()
{
	return buttonKind;
}


/******************************************************************************
 DoDrawIcon
 
 	Check whether icon needs to be erased before redrawing.
 	B&W icons do, color icons don't.
******************************************************************************/

void CCicnButton::DoDrawIcon(Boolean fHilite)
{
	Rect	erase;

	if (itsCicnH[CalcDrawState(fHilite)] == NULL)
	{
		FrameToQDR(&frame, &erase);
		EraseRect(&erase);
	}
	DrawIcon(fHilite);
}


/******************************************************************************
 AdjustCursor {OVERRIDE}

		Mouse is inside a CCicnButton Pane. Use the pointing finger cursor.
 ******************************************************************************/

void	CCicnButton::AdjustCursor(
	Point		where,					/* Mouse location in Window coords	*/
	RgnHandle	mouseRgn)
{
	if (GetEnable())
		SetCursor(*itsPointerHand);
}


/******************************************************************************
 Track	{OVERRIDE}
 
 	Override Track to set the value if the mouse winds up in the icon.
 	Also, Track called DrawIcon one too many times, resulting in an
 	unnecessary flash for a button with multiple states.
******************************************************************************/

Boolean CCicnButton::Track(void)
{
	short		oldValue = value;
	Boolean		wasInButton = TRUE;
	Point		where;
	Rect		iconFrame;

	LongToQDRect(&frame, &iconFrame);

	DoDrawIcon(TRUE);
	
	while (StillDown())
	{
		GetMouse(&where);
		if (PtInRect(where, &iconFrame))
		{
			if (!wasInButton)
				DoDrawIcon(TRUE);
			wasInButton = TRUE;
		}
		else
		{
			if (wasInButton)
				DoDrawIcon(FALSE);
			wasInButton = FALSE;
		}
	}

	if (wasInButton && buttonKind > kSAPushButton)
	{
		if (buttonKind == kSACheckBox)
			ChangeValue(1 - value);
			
		else if (buttonKind == kSARadioButton){
			ResetIconGroup();
			ChangeValue(1);
		}
		else
			ChangeValue(1);
	}

		// Now it's OK to redraw the button, after it has
		// been set to its final state

	DoDrawIcon(FALSE);

	return wasInButton;
}


/******************************************************************************
 FixupBorder
 
 	Fixup the border per the current button state
******************************************************************************/

void CCicnButton::FixupBorder()
{
	Point	pensize;
	Rect	wFrame;

	if (outlineOn && itsBorder == NULL)
	{
		Rect		margin;
		CPaneBorder	*border;

		border = new CPaneBorder;
		border->IPaneBorder(kBorderFrame);
		SetRect(&margin, -1, -1, 1, 1);
		border->SetMargin(&margin);
		border->SetPenSize(2, 2);
		SetBorder(border);
	}
	if (value == 0 && itsBorder)
	{
		itsBorder->GetPenSize(&pensize.h, &pensize.v);
		if (pensize.h)
		{
			RefreshBorder();
			FrameToWindR(&frame, &wFrame);
			ValidRect(&wFrame);
			itsBorder->SetPenSize(0, 0);
			borderPen = pensize;
		}
	}
	else if (value && outlineOn)
	{
		Point	pensize;

		itsBorder->GetPenSize(&pensize.h, &pensize.v);
		if (pensize.h == 0)
		{
			itsBorder->SetPenSize(borderPen.h, borderPen.v);
			RefreshBorder();
			FrameToWindR(&frame, &wFrame);
			ValidRect(&wFrame);
		}
	}
}

/******************************************************************************
  ResetIconGroup
 
	Reset a button group according to kind of button
******************************************************************************/
	typedef struct {
		CCicnButton	*pane;
		short	group;
		Boolean	radio;
	} ResetRec;

	static void ResetButtons(CCicnButton *pane, ResetRec *rec)
	{
		short	group, kind;
		
			// Don't reset self

		if (pane == rec->pane)
			return;

			if (member( pane, CCicnButton) && rec->radio && (rec->group == pane->groupID)){
				pane->ChangeValue(0);
				pane->DoDrawIcon(0);
			}

	}

void CCicnButton::ResetIconGroup(void)
{
	ResetRec	rec;
	
	rec.group = groupID;

	if (rec.group == 0 || buttonKind == 0)
		return;
	
	rec.pane = this;

	switch (buttonKind)
	{
		case kSARadioButton:
			rec.radio = TRUE;
			break;
		case kSACheckBox:
			rec.radio = FALSE;
			break;
	}
	
	itsEnclosure->itsSubviews->DoForEach1((EachFunc1) ResetButtons, (long) &rec);
}
