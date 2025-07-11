/*
	Public domain by Zig Zichterman.
*/
/*
	LCDEFRadioButton
	
	12/28/94	zz	h	initial write
*/
#include "LCDEFRadioButton.h"

#include "RgnHandleT.h"
#include "StSaveColor.h"
/**************************************************************************
	CalcRadioCheckRgn()								[protected, virtual]
													[complete override ]
	Radio button is a circle
**************************************************************************/
void 
LCDEFRadioButton::CalcRadioCheckRgn(
	RgnHandleT &	outRadioCheckRgn) const
{
	::OpenRgn();
	::FrameOval(&mRadioCheckRect);
	::CloseRgn(outRadioCheckRgn);
}

/**************************************************************************
	Draw3DEffects()									[protected, virtual]
													[complete override ]
	draw the button convex or concave
**************************************************************************/
void 
LCDEFRadioButton::Draw3DEffects(void) const
{
	// if highlighted, no 3D
	if (IsHighlighted()) {
		return;
	}
	
	// save the color
	StSaveColor	saveColor;

	// 3D effects only for Value_On or Value_Off
	short	value	= GetControlValue();
	switch (value) {
		case Value_Off	:	// empty/convex
			{
				// erase the insides
				::BackColor(whiteColor);
				RectT	insides(mRadioCheckRect);
				::InsetRect(&insides,1,1);
				::EraseOval(&insides);
			}
			
			ForeGrey(0xEEEE);
			::MoveTo(mRadioCheckRect.left + 6, mRadioCheckRect.top + 2);
			::LineTo(mRadioCheckRect.left + 3, mRadioCheckRect.top + 5);
			::MoveTo(mRadioCheckRect.left + 4, mRadioCheckRect.top + 5);
			::LineTo(mRadioCheckRect.left + 7, mRadioCheckRect.top + 2);
			::MoveTo(mRadioCheckRect.left + 3, mRadioCheckRect.top + 5);
			::LineTo(mRadioCheckRect.left + 3, mRadioCheckRect.top + 7);
			
			ForeGrey(0xDDDD);
			::MoveTo(mRadioCheckRect.left + 3, mRadioCheckRect.top + 8);
			::LineTo(mRadioCheckRect.left + 4, mRadioCheckRect.top + 8);
			::LineTo(mRadioCheckRect.left + 4, mRadioCheckRect.top + 6);
			::LineTo(mRadioCheckRect.left + 6, mRadioCheckRect.top + 6);
			::LineTo(mRadioCheckRect.left + 6, mRadioCheckRect.top + 4);
			::LineTo(mRadioCheckRect.left + 7, mRadioCheckRect.top + 4);
			::LineTo(mRadioCheckRect.left + 7, mRadioCheckRect.top + 3);
			::LineTo(mRadioCheckRect.left + 8, mRadioCheckRect.top + 3);
			::LineTo(mRadioCheckRect.left + 8, mRadioCheckRect.top + 2);
			::MoveTo(mRadioCheckRect.left + 5, mRadioCheckRect.top + 5);
			::Line(0,0);
			::MoveTo(mRadioCheckRect.left + 7, mRadioCheckRect.top + 5);
			::Line(0,0);
			::MoveTo(mRadioCheckRect.left + 5, mRadioCheckRect.top + 7);
			::Line(0,0);

			ForeGrey(0xBBBB);
			::MoveTo(mRadioCheckRect.left + 3, mRadioCheckRect.top + 9);
			::LineTo(mRadioCheckRect.left + 5, mRadioCheckRect.top + 9);
			::LineTo(mRadioCheckRect.left + 5, mRadioCheckRect.top + 8);
			::LineTo(mRadioCheckRect.left + 6, mRadioCheckRect.top + 8);
			::LineTo(mRadioCheckRect.left + 6, mRadioCheckRect.top + 7);
			::LineTo(mRadioCheckRect.left + 7, mRadioCheckRect.top + 7);
			::LineTo(mRadioCheckRect.left + 7, mRadioCheckRect.top + 6);
			::LineTo(mRadioCheckRect.left + 8, mRadioCheckRect.top + 6);
			::LineTo(mRadioCheckRect.left + 8, mRadioCheckRect.top + 4);

			ForeGrey(0xAAAA);
			::MoveTo(mRadioCheckRect.left + 6, mRadioCheckRect.top + 9);
			::LineTo(mRadioCheckRect.left + 7, mRadioCheckRect.top + 9);
			::LineTo(mRadioCheckRect.left + 7, mRadioCheckRect.top + 8);
			::LineTo(mRadioCheckRect.left + 8, mRadioCheckRect.top + 8);
			::LineTo(mRadioCheckRect.left + 8, mRadioCheckRect.top + 7);
			::LineTo(mRadioCheckRect.left + 9, mRadioCheckRect.top + 7);
			::LineTo(mRadioCheckRect.left + 9, mRadioCheckRect.top + 2);

			ForeGrey(0x8888);
			::MoveTo(mRadioCheckRect.left + 4, mRadioCheckRect.top + 10);
			::LineTo(mRadioCheckRect.left + 7, mRadioCheckRect.top + 10);
			::MoveTo(mRadioCheckRect.left + 8, mRadioCheckRect.top + 9);
			::LineTo(mRadioCheckRect.left + 9, mRadioCheckRect.top + 9);
			::LineTo(mRadioCheckRect.left + 9, mRadioCheckRect.top + 8);
			::MoveTo(mRadioCheckRect.left + 10, mRadioCheckRect.top + 7);
			::LineTo(mRadioCheckRect.left + 10, mRadioCheckRect.top + 4);
			break;
			
		case Value_On	:	// dot/concave
			{
				// erase the insides
				::BackColor(whiteColor);
				RectT	insides(mRadioCheckRect);
				::InsetRect(&insides,1,1);
				::EraseOval(&insides);
			}
			
			ForeGrey(0xEEEE);
			::MoveTo(mRadioCheckRect.left + 9, mRadioCheckRect.top + 5);
			::LineTo(mRadioCheckRect.left + 9, mRadioCheckRect.top + 7);
			::MoveTo(mRadioCheckRect.left + 5, mRadioCheckRect.top + 9);
			::LineTo(mRadioCheckRect.left + 7, mRadioCheckRect.top + 9);
			
			ForeGrey(0xDDDD);
			::MoveTo(mRadioCheckRect.left + 3, mRadioCheckRect.top + 9);
			::LineTo(mRadioCheckRect.left + 4, mRadioCheckRect.top + 9);
			::MoveTo(mRadioCheckRect.left + 9, mRadioCheckRect.top + 3);
			::LineTo(mRadioCheckRect.left + 9, mRadioCheckRect.top + 4);
			
			ForeGrey(0xBBBB);
			::MoveTo(mRadioCheckRect.left + 2, mRadioCheckRect.top + 9);
			::LineTo(mRadioCheckRect.left + 2, mRadioCheckRect.top + 8);
			::LineTo(mRadioCheckRect.left + 3, mRadioCheckRect.top + 8);
			::MoveTo(mRadioCheckRect.left + 8, mRadioCheckRect.top + 3);
			::LineTo(mRadioCheckRect.left + 8, mRadioCheckRect.top + 2);
			::LineTo(mRadioCheckRect.left + 9, mRadioCheckRect.top + 2);

			ForeGrey(0xAAAA);
			::MoveTo(mRadioCheckRect.left + 2, mRadioCheckRect.top + 7);
			::LineTo(mRadioCheckRect.left + 2, mRadioCheckRect.top + 4);
			::LineTo(mRadioCheckRect.left + 4, mRadioCheckRect.top + 2);
			::LineTo(mRadioCheckRect.left + 7, mRadioCheckRect.top + 2);

			ForeGrey(0x8888);
			::MoveTo(mRadioCheckRect.left + 1, mRadioCheckRect.top + 7);
			::LineTo(mRadioCheckRect.left + 1, mRadioCheckRect.top + 4);
			::LineTo(mRadioCheckRect.left + 4, mRadioCheckRect.top + 1);
			::LineTo(mRadioCheckRect.left + 7, mRadioCheckRect.top + 1);
			::MoveTo(mRadioCheckRect.left + 2, mRadioCheckRect.top + 2);
			::Line(0,0);

			// draw �
			saveColor.Restore();
			RectT	dotRect;
			dotRect.Copy(mRadioCheckRect);
			::InsetRect(&dotRect,3,3);
			::PaintOval(&dotRect);
			break;
	}
	
	// [restore color through ~saveColor]
}

/**************************************************************************
	DrawValue()										[protected, virtual]
													[complete override ]
	Draw the dot, dash, whatever
**************************************************************************/
void 
LCDEFRadioButton::DrawValue(
	Is3DFlagT			inIs3D,
	IsEnabledFlagT		inEnabled,
	IsHighlightedFlagT	/*inHighlight*/) const
{
	// save the color, since we mess with it in here
	StSaveColor	saveColor;
	
	// get the rect for the checkbox insides
	RectT	partRect;
	partRect.Copy(mRadioCheckRect);
	::InsetRect(&partRect,1,1);
	
	// get value
	short	value	= GetControlValue();

	// if active
	if (inEnabled) {
		if (inIs3D && ((value == Value_Off) || (value == Value_On))) {
			// erase to light grey
			BackGrey(Grey_Background);
		} else {
			// erase to white
			::BackColor(whiteColor);
		}
		::EraseOval(&partRect);
	}
	

	// draw the dot or whatever
	switch (value) {
		case Value_Off					:
			// leave empty
			break;
			
		case Value_On					:
			// draw an X
			{
				RectT	dotRect;
				dotRect.Copy(partRect);
				::InsetRect(&dotRect,2,2);
				::PaintOval(&dotRect);
			}
			break;
			
		case Value_MicrosoftTristate	:
			{
				// fabricate a ltGray pattern on the fly. We don't
				// have quickdraw globals and I'm not going to set
				// up A4 to get them.
				unsigned char lightGrey[8];
				lightGrey[0] = lightGrey[2]
					= lightGrey[4] = lightGrey[6] = 0x88;
				lightGrey[1] = lightGrey[3]
					= lightGrey[5] = lightGrey[7] = 0x22;
				
				// if we have enough colors
				if (inIs3D) {
					// use shades of grey
					ForeGrey(Grey_MicrosoftTristateFore);
					BackGrey(Grey_MicrosoftTristateBack);
				}
				
				// fill the box with a pattern
				::FillOval(&partRect,(PatPtr) &lightGrey);
			}
			break;
		
		case Value_Tristate				:
		default							:
			{
				const short	centerV	= (partRect.top + partRect.bottom) / 2;
				::MoveTo(partRect.left + 2,centerV);
				::LineTo(partRect.right - 3, centerV);
			}
			break;
	}
	
	// [restore the color through ~saveColor]
}

/**************************************************************************
	DrawFrame()										[protected, virtual]
													[complete override ]
														
	Draw the border around the checkbox or radio button. If highlighted,
	also draw the inset highlight border
**************************************************************************/
void 
LCDEFRadioButton::DrawFrame(void) const
{
	// frame the checkbox rect
	::FrameOval(&mRadioCheckRect);
	
	// if highlighted
	if (IsHighlighted()) {
		// inset the rect
		RectT	highlightRect;
		highlightRect.Copy(mRadioCheckRect);
		::InsetRect(&highlightRect,1,1);

		// and frame it again
		::FrameOval(&highlightRect);
	}
}
