/*
	Public domain by Zig Zichterman.
*/
/*
	LCDEFPushButton
	
	A push button
	
	12/27/94	zz	h	initial write
*/
#include "LCDEFPushButton.h"

#include "StSaveClip.h"
#include "RectT.h"
#include <TextEdit.h>

/**************************************************************************
	DrawTitle()										[protected, virtual]
													[complete override ]
													
	Draw the button title
**************************************************************************/
void 
LCDEFPushButton::DrawTitle(
	Is3DFlagT			inIs3D,
	IsHighlightedFlagT	inIsHighlighted,
	Rect &				outTitleRect) const
{
	// get our rect
	GetControlRect(outTitleRect);
	
	// inset it a few pixels
	::InsetRect(&outTitleRect,2,2);
	
	// fudge by 1 pixel to line up with system CDEF 0
	outTitleRect.left--;
	outTitleRect.right--;
	
	// bump down and to the right, if asked
	if (mIndentWhenHighlighted && inIs3D && inIsHighlighted) {
		::OffsetRect(&outTitleRect,1,1);
	}
	
	// clip to it
	StSaveClip	saveClip;
	RgnHandleT	newClip(saveClip.mRgn);
	RgnHandleT	rect(outTitleRect);
	newClip.SectWith(rect);
	newClip.CopyToClip();
	
	// draw the title
	const StringPtr	TitleStr	= GetControlTitle();
	StringBox(TitleStr,outTitleRect,teCenter);
}

/**************************************************************************
	Draw3DEffects()									[protected, virtual]
													[complete override ]
	Draw th 3D bevels around the button edges
**************************************************************************/
void 
LCDEFPushButton::Draw3DEffects(void) const
{
	const Boolean	isHighlighted = IsHighlighted();
	
	if (isHighlighted) {	// draw highlighted
		{	// outer rim
			RectT	outer;
			GetControlRect(outer);
			::InsetRect(&outer,1,1);
			
			// outside edge, top left shadow
			ForeGrey(0x4444);
			::MoveTo(outer.left,outer.bottom - 3);
			::LineTo(outer.left,outer.top + 2);
			::MoveTo(outer.left + 2, outer.top);
			::LineTo(outer.right - 3,outer.top);
			
			// outside edge, bottom right unshadow
			ForeGrey(0xCCCC);
			::MoveTo(outer.left + 2, outer.bottom - 1);
			::LineTo(outer.right - 3, outer.bottom - 1);
			::MoveTo(outer.right - 1, outer.bottom - 3);
			::LineTo(outer.right - 1, outer.top + 2);
			
			// outside edge, top left corner
			ForeGrey(0x5555);
			::MoveTo(outer.left + 1, outer.top + 1);
			::Line(0,0);
			
			// outside edge, top right corner
			ForeGrey(0x5555);
			::MoveTo(outer.right - 2, outer.top + 1);
			::Line(0,0);
			
			// outside edge, bottom left corner
			ForeGrey(0x5555);
			::MoveTo(outer.left + 1, outer.bottom - 2);
			::Line(0,0);
			
			// outside edge, bottom right corner
			ForeGrey(0xCCCC);
			::MoveTo(outer.right - 2, outer.bottom - 2);
			::Line(0,0);
		}
		
		{	// inner rim
			RectT	inner;
			GetControlRect(inner);
			::InsetRect(&inner,2,2);
			
			// inside edge, top left shadow
			ForeGrey(0x5555);
			::MoveTo(inner.left, inner.bottom - 2);
			::LineTo(inner.left, inner.top + 1);
			::MoveTo(inner.left + 1, inner.top);
			::LineTo(inner.right - 2, inner.top);
			
			// inside edge, bottom right unshadow
			ForeGrey(0xAAAA);
			::MoveTo(inner.left + 1, inner.bottom - 1);
			::LineTo(inner.right - 2, inner.bottom - 1);
			::MoveTo(inner.right - 1, inner.bottom - 2);
			::LineTo(inner.right - 1, inner.top + 1);
			
			// inside edge, top left corner
			ForeGrey(0x4444);
			::MoveTo(inner.left + 1, inner.top + 1);
			::Line(0,0);
			
			// inside edge, top right corner
			ForeGrey(0x4444);
			::MoveTo(inner.right - 2, inner.top + 1);
			::Line(0,0);
			
			// inside edge, bottom left corner
			ForeGrey(0x4444);
			::MoveTo(inner.left + 1, inner.bottom - 2);
			::Line(0,0);
			
			// inside edge, bottom right corner
			ForeGrey(0xAAAA);
			::MoveTo(inner.right - 2, inner.bottom - 2);
			::Line(0,0);
		}
	} else {	// draw unhighlighted
		{	// outer rim
			RectT	outer;
			GetControlRect(outer);
			::InsetRect(&outer,1,1);

			// outside edge, bottom left shadow
			ForeGrey(0x5555);
			::MoveTo(outer.left + 2,outer.bottom - 1);
			::LineTo(outer.right - 3,outer.bottom - 1);
			::MoveTo(outer.right - 1,outer.bottom - 3);
			::LineTo(outer.right - 1,outer.top + 2);
						
			// outside edge, top left corner
			ForeGrey(0xFFFF);
			::MoveTo(outer.left + 1, outer.top + 1);
			::Line(0,0);
			
			// outside edge, top right corner
			ForeGrey(0xBBBB);
			::MoveTo(outer.right - 2, outer.top + 1);
			::Line(0,0);
			
			// outside edge, bottom left corner
			ForeGrey(0xBBBB);
			::MoveTo(outer.left + 1, outer.bottom - 2);
			::Line(0,0);
			
			// outside edge, bottom right corner
			ForeGrey(0x5555);
			::MoveTo(outer.right - 2, outer.bottom - 2);
			::Line(0,0);
		}
		
		{	// inner rim
			RectT	inner;
			GetControlRect(inner);
			::InsetRect(&inner,2,2);
			
			// inside edge, top left unshadow
			ForeGrey(0xFFFF);
			::MoveTo(inner.left, inner.bottom - 2);
			::LineTo(inner.left, inner.top + 1);
			::MoveTo(inner.left + 1, inner.top);
			::LineTo(inner.right - 2, inner.top);
			
			// inside edge, bottom right shadow
			ForeGrey(0x8888);
			::MoveTo(inner.left + 1, inner.bottom - 1);
			::LineTo(inner.right - 2, inner.bottom - 1);
			::MoveTo(inner.right - 1, inner.bottom - 2);
			::LineTo(inner.right - 1, inner.top + 1);
			
			// inside edge, top left corner
			ForeGrey(0xFFFF);
			::MoveTo(inner.left + 1, inner.top + 1);
			::Line(0,0);
			
			// inside edge, top right corner
			ForeGrey(0xBBBB);
			::MoveTo(inner.right - 2, inner.top + 1);
			::Line(0,0);
			
			// inside edge, bottom left corner
			ForeGrey(0xBBBB);
			::MoveTo(inner.left + 1, inner.bottom - 2);
			::Line(0,0);
			
			// inside edge, bottom right corner
			ForeGrey(0x8888);
			::MoveTo(inner.right - 2, inner.bottom - 2);
			::Line(0,0);
		}
	}
}
