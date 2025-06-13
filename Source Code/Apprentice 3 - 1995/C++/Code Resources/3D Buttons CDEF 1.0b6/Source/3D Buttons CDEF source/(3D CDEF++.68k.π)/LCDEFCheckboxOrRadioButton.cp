/*
	Public domain by Zig Zichterman.
*/
/*
	LCDEFCheckboxOrRadioButton
	
	Base class for checkboxes and radio buttons
	
	12/28/94	zz	h	initial write
*/
#pragma once

#include "LCDEFCheckboxOrRadioButton.h"

#include "RgnHandleT.h"
#include <Script.h>
#include <TextEdit.h>

/**************************************************************************
	SetBGColorToErase()								[protected, virtual]
													[complete override ]
	Use the background color, or white
**************************************************************************/
void
LCDEFCheckboxOrRadioButton::SetBGColorToErase(
	Is3DFlagT			inIs3D,
	IsEnabledFlagT		/*inIsEnabled*/,
	IsHighlightedFlagT	/*inIsHighlighted*/) const
{
	// if we have enough colors
	if (inIs3D) {
		// use background color
		::RGBBackColor(&mBackgroundColor);
	} else {
		// use white
		::BackColor(whiteColor);
	}
}

/**************************************************************************
	Erase()											[protected, virtual]
													[complete override ]
	
	Just erase the entire rect. This would be really flicker-prone, but
	we have offscreen GWorlds to take away this ugliness. People who
	still run machines incapable of offscreen GWorlds will just have
	to live with the flicker, or buy a new machine...
**************************************************************************/
void 
LCDEFCheckboxOrRadioButton::Erase(void) const
{
	RectT	controlRect;
	GetControlRect(controlRect);
	::EraseRect(&controlRect);
}

/**************************************************************************
	CalcPartRects()									[protected, virtual]
													[complete override ]

	Where should the checkbox/radio button go, and where should the 
	title go? Fills in mRadioCheckRect and mTitleRect.
**************************************************************************/
void
LCDEFCheckboxOrRadioButton::CalcPartRects(void)
{
	// get the control rect
	RectT	controlRect;
	GetControlRect(controlRect);
	
	// ÑÑÑÑ vertical coords ÑÑÑÑ
	// get the vertical center of the control
	const short	CenterV	= (controlRect.top + controlRect.bottom)/2;
	
	// checkbox is v-centered, 12-px tall
	mRadioCheckRect.top 
		= CenterV - (CheckboxOrRadioButtonSizePixels/2);
	mRadioCheckRect.bottom
		= mRadioCheckRect.top + CheckboxOrRadioButtonSizePixels;

	// textbox fills from top to bottom, DrawTitle() takes
	// care of vertically centering the actual text
	mTitleRect.top				= controlRect.top;
	mTitleRect.bottom			= controlRect.bottom;

	// ÑÑÑÑ horizontal coords ÑÑÑÑ
	// is the Macintosh's current system script predominantly left-to-right?
	// (such as English)
	const Boolean		IsLeftToRight	= (::GetSysJust() == 0);
	
	// checkbox is left- or right-aligned, offset by a couple pix
	if (IsLeftToRight) {
		mRadioCheckRect.left	= controlRect.left + 2;
		mRadioCheckRect.right	= mRadioCheckRect.left
									+ CheckboxOrRadioButtonSizePixels;
	} else {
		mRadioCheckRect.right	= controlRect.right - 2;
		mRadioCheckRect.left	= mRadioCheckRect.right
									- CheckboxOrRadioButtonSizePixels;
	}
	
	// textbox is next to the checkbox
	if (IsLeftToRight) {
		mTitleRect.left		= mRadioCheckRect.right + 3;
		mTitleRect.right	= controlRect.right;
	} else {
		mTitleRect.right	= mRadioCheckRect.left - 3;
		mTitleRect.left		= controlRect.left;
	}
}
		
/**************************************************************************
	DrawTitle()										[protected, virtual]
	
	Draw the title, using the system default alignment, centered
	vertically in the title rect. 
**************************************************************************/
void 
LCDEFCheckboxOrRadioButton::DrawTitle(
	Is3DFlagT			/*inIs3D*/,
	IsHighlightedFlagT	/*inIsHighlighted*/,
	Rect &				outTitleRect) const
{
	// get the title
	const StringPtr	TitleStr	= GetControlTitle();
	
	// draw it
	StringBox(TitleStr,mTitleRect,teFlushDefault);

	// return the box we used
	outTitleRect = mTitleRect;
}
