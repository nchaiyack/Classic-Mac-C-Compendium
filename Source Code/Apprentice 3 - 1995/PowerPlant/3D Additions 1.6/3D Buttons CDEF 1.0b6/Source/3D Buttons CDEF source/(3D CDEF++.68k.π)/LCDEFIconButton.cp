/*
	Public domain by Zig Zichterman.
*/
/*
	LCDEFIconButton
	
	An icon button
	
	12/28/94	zz	h	initial write
*/
#include "LCDEFIconButton.h"

#include <Icons.h>
#include "RectT.h"
#include <TextUtils.h>

/**************************************************************************
	Draw3DEffects()									[protected, virtual]
													[complete override ]
	Draw the spiffy 3D bevels
**************************************************************************/
void 
LCDEFIconButton::Draw3DEffects(void) const
{
	if (IsHighlighted()) {	// draw highlighted
		{	// ouside edge
			RectT	box;
			GetControlRect(box);
			::InsetRect(&box,1,1);
			
			// top left shadow
			ForeGrey(0x4444);
			::MoveTo(box.left, box.bottom - 2);
			::LineTo(box.left, box.top);
			::LineTo(box.right - 2, box.top);
			
			// bottom right unshadow
			ForeGrey(0xCCCC);
			::MoveTo(box.left, box.bottom - 1);
			::LineTo(box.right - 1, box.bottom - 1);
			::LineTo(box.right - 1, box.top);
			
			// top left corner
			ForeGrey(0x2222);
			::MoveTo(box.left, box.top);
			::Line(0,0);
			
			// top right corner
			ForeGrey(0x7777);
			::MoveTo(box.right - 1, box.top);
			::Line(0,0);
						
			// bottom left corner
			ForeGrey(0x7777);
			::MoveTo(box.left, box.bottom - 1);			
			::Line(0,0);

			// bottom right corner
			ForeGrey(0xCCCC);
			::MoveTo(box.right - 1, box.bottom - 1);
			::Line(0,0);
		}
		
		{	// inside edge
			Rect	box	= (**mControl).contrlRect;
			::InsetRect(&box,2,2);
			
			// top left shadow
			ForeGrey(0x5555);
			::MoveTo(box.left, box.bottom - 2);
			::LineTo(box.left, box.top);
			::LineTo(box.right - 2, box.top);
			
			// bottom right unshadow
			ForeGrey(0xAAAA);
			::MoveTo(box.left, box.bottom - 1);
			::LineTo(box.right - 1, box.bottom - 1);
			::LineTo(box.right - 1, box.top);

			// top left corner
			ForeGrey(0x4444);
			::MoveTo(box.left, box.top);
			::Line(0,0);
			
			// top right corner
			ForeGrey(0x8888);
			::MoveTo(box.right - 1, box.top);
			::Line(0,0);
			
			// bottom left corner
			ForeGrey(0x8888);
			::MoveTo(box.left, box.bottom - 1);			
			::Line(0,0);
			
			// bottom right corner
			ForeGrey(0xDDDD);
			::MoveTo(box.right - 1, box.bottom - 1);
			::Line(0,0);
		}
	} else {						// draw unhighlighted
		{	// ouside edge
			Rect	box	= (**mControl).contrlRect;
			::InsetRect(&box,1,1);
			
			// top left unshadow
			ForeGrey(0xDDDD);
			::MoveTo(box.left, box.bottom - 2);
			::LineTo(box.left, box.top);
			::LineTo(box.right - 2, box.top);
			
			// bottom right shadow
			ForeGrey(0x7777);
			::MoveTo(box.left, box.bottom - 1);
			::LineTo(box.right - 1, box.bottom - 1);
			::LineTo(box.right - 1, box.top);
			
			// top left corner
			ForeGrey(0xFFFF);
			::MoveTo(box.left, box.top);
			::Line(0,0);
			
			// top right corner
			ForeGrey(0xCCCC);
			::MoveTo(box.right - 1, box.top);
			::Line(0,0);
						
			// bottom left corner
			ForeGrey(0xCCCC);
			::MoveTo(box.left, box.bottom - 1);			
			::Line(0,0);

			// bottom right corner
			ForeGrey(0x4444);
			::MoveTo(box.right - 1, box.bottom - 1);
			::Line(0,0);
		}
		
		{	// inside edge
			Rect	box	= (**mControl).contrlRect;
			::InsetRect(&box,2,2);
			
			// top left shadow
			ForeGrey(0xFFFF);
			::MoveTo(box.left, box.bottom - 2);
			::LineTo(box.left, box.top);
			::LineTo(box.right - 2, box.top);
			
			// bottom right unshadow
			ForeGrey(0x8888);
			::MoveTo(box.left, box.bottom - 1);
			::LineTo(box.right - 1, box.bottom - 1);
			::LineTo(box.right - 1, box.top);

			// top left corner
			ForeGrey(0xFFFF);
			::MoveTo(box.left, box.top);
			::Line(0,0);
			
			// top right corner
			ForeGrey(0xCCCC);
			::MoveTo(box.right - 1, box.top);
			::Line(0,0);
			
			// bottom left corner
			ForeGrey(0xCCCC);
			::MoveTo(box.left, box.bottom - 1);			
			::Line(0,0);
			
			// bottom right corner
			ForeGrey(0x5555);
			::MoveTo(box.right - 1, box.bottom - 1);
			::Line(0,0);
		}
	}
}

/**************************************************************************
	DrawValue()										[protected, virtual]
													[complete override ]
	Draw an icon
**************************************************************************/
void 
LCDEFIconButton::DrawValue(
	Is3DFlagT			inIs3D,
	IsEnabledFlagT		inEnabled,
	IsHighlightedFlagT	inHighlighted) const
{
	// get the icon ID (the button's value)
	short	iconID	= GetControlValue();
	
	// if value is 0, draw nothing
	if (iconID == 0) {
		return;
	}
	
	// get the control rect
	RectT	iconRect;
	GetControlRect(iconRect);
	
	// leave room for the the frame & 3D stuff
	const short	MinimumMarginPixels	= 4;
	::InsetRect(&iconRect,MinimumMarginPixels,MinimumMarginPixels);
	
	// are we big enough for large or small?
	short	width	= iconRect.right - iconRect.left;
	short	height	= iconRect.bottom - iconRect.top;
	
	// use the smallest dimension--either height or width
	short	iconSize	= 0;
	if (width < height) {
		iconSize	= width;
	} else {
		iconSize	= height;
	}
	
	// pin to 32 or 16 pixels for large or small icon
	const short	LargeIconSizePixels	= 32;
	const short	SmallIconSizePixels	= 16;
	if (LargeIconSizePixels <= iconSize) {
		iconSize = LargeIconSizePixels;
	} else if (SmallIconSizePixels < iconSize) {
		iconSize = SmallIconSizePixels;
	}

	// center the icon in the control
	const short	CenterH	= (iconRect.left + iconRect.right)/2;
	const short	CenterV	= (iconRect.top + iconRect.bottom)/2;
	iconRect.left	= CenterH - (iconSize/2);
	iconRect.top	= CenterV - (iconSize/2);
	iconRect.right	= iconRect.left + iconSize;
	iconRect.bottom	= iconRect.top + iconSize;

	// draw the icon normal? selected? dim?
	short	iconTransform	= ttNone;
	if (inEnabled == false) {
		iconTransform |= ttDisabled;
	}
	if (inHighlighted) {
		iconTransform |= ttSelected;
	}
	
	// shift down and to the right. If requested
	if (mIndentWhenHighlighted && inIs3D && inHighlighted) {
		::OffsetRect(&iconRect,1,1);
	}
	
	// draw the icon
	::PlotIconID(&iconRect,atNone,iconTransform,iconID);
}

