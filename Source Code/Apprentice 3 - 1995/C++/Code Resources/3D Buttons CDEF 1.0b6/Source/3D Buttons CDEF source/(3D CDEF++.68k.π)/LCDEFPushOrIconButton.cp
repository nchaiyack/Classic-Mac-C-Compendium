/*
	Public domain by Zig Zichterman.
*/
/*
	LCDEFPushOrIconButton
	
	Base class for push button and icon button
	
	12/27/94	zz	h	initial write
*/
#include "LCDEFPushOrIconButton.h"

#include "RectT.h"
#include "StSaveColor.h"

/**************************************************************************
	SetBGColorToErase()								[protected, virtual]
													[complete override ]
	
	if not 3D, use white (normal) or black (highlighted). If 3D but
	inactive, use background color. If 3D and active, use a mild
	grey (normal) or dark grey (highlighted).
**************************************************************************/
void
LCDEFPushOrIconButton::SetBGColorToErase(
	Is3DFlagT			inIs3D,
	IsEnabledFlagT		inIsEnabled,
	IsHighlightedFlagT	inIsHighlighted) const
{
	if (inIs3D) {	// we're in 3D
		// if enabled
		if (inIsEnabled) {
			// if highlighted, 
			if (inIsHighlighted) {
				// dark grey background
				BackGrey(Grey_HighlightedButtonContent);
			} else {
				// light grey background
				BackGrey(Grey_ButtonContent);
			}
		} else {
			// inactive controls always use background color
			::RGBBackColor(&mBackgroundColor);
		}
	} else {		// not 3D: it's white
		::BackColor(whiteColor);
	}
}

/**************************************************************************
	Erase()											[protected, virtual]
													[complete override ]
	
	Take the control rect, inset it by 1 pixel, and erase it as a 
	rounded rectangle. Get the rounded rectangle's corner radius
	from a virtual function
**************************************************************************/
void 
LCDEFPushOrIconButton::Erase(void) const
{
	// get the control rect
	RectT	eraseRect;
	GetControlRect(eraseRect);
	
	// inset by one pixel
	::InsetRect(&eraseRect,1,1);
	
	// erase the rounded rect
	const short	InsideRadius	= GetInsideRadius();
	::EraseRoundRect(&eraseRect,InsideRadius,InsideRadius);
}

/**************************************************************************
	DrawFrame()										[protected, virtual]
													[complete override ]
	Frame a rounded rect
**************************************************************************/
void 
LCDEFPushOrIconButton::DrawFrame(void) const
{
	// get the control rect
	RectT	controlRect;
	GetControlRect(controlRect);
	
	// frame it
	const short	FrameRadius	= GetFrameRadius();
	::FrameRoundRect(&controlRect,FrameRadius,FrameRadius);
}

/**************************************************************************
	InvertHighlight()								[protected, virtual]
													[complete override ]
	invert the insides of the button
**************************************************************************/
void
LCDEFPushOrIconButton::InvertHighlight(void) const
{
	// get the inside rect
	RectT	insides;
	GetControlRect(insides);
	::InsetRect(&insides,1,1);
	
	// save the colors before we change them
	StSaveColor	saveColor;
	
	// we like black on white
	::ForeColor(blackColor);
	::BackColor(whiteColor);
	
	// invert it
	const short	InsideRadius	= GetInsideRadius();
	::InvertRoundRect(&insides,InsideRadius,InsideRadius);
	
	// [restore colors through ~saveColor]
}

/**************************************************************************
	GetControlRgn()									[protected, virtual]
													[complete override ]
	Return a rounded rect
**************************************************************************/
void
LCDEFPushOrIconButton::GetControlRgn(
	RgnHandle	outControlRgn) const
{
	::OpenRgn();
	RectT	controlRect;
	GetControlRect(controlRect);
	const short	FrameRadius	= GetFrameRadius();
	::FrameRoundRect(&controlRect,FrameRadius,FrameRadius);
	::CloseRgn(outControlRgn);	
}

