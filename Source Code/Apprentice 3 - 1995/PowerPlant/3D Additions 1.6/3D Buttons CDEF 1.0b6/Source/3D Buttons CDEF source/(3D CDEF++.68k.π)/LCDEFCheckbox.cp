/*
	Public domain by Zig Zichterman.
*/
/*
	LCDEFCheckbox
	
	12/28/94	zz	h	initial write
*/
#include "LCDEFCheckbox.h"

#include "RgnHandleT.h"
#include "StSaveColor.h"

/**************************************************************************
	CalcRadioCheckRgn()								[protected, virtual]
													[complete override ]
	Return a square checkbox region
**************************************************************************/
void 
LCDEFCheckbox::CalcRadioCheckRgn(
	RgnHandleT &	outRadioCheckRgn) const
{
	outRadioCheckRgn.SetRect(mRadioCheckRect);
}

/**************************************************************************
	Draw3DEffects()									[protected, virtual]
													[complete override ]
	draw a bevel around the checkbox
**************************************************************************/
void 
LCDEFCheckbox::Draw3DEffects(void) const
{
	// get the checkbox rect
	RectT	bevelRect;
	bevelRect.Copy(mRadioCheckRect);
	bevelRect.left--;
	bevelRect.top--;
	
	// save the color
	StSaveColor	saveColor;
	
	// change to shadow
	::RGBForeColor(&mBevelShadow);
	
	// draw the top and left sides
	::MoveTo(bevelRect.left,bevelRect.bottom);
	::LineTo(bevelRect.left,bevelRect.top);
	::LineTo(bevelRect.right,bevelRect.top);
	
	// change to light
	::RGBForeColor(&mBevelLight);
	
	// draw the bottom and right sides
	::MoveTo(bevelRect.right,bevelRect.top + 1);
	::LineTo(bevelRect.right,bevelRect.bottom);
	::LineTo(bevelRect.left + 1, bevelRect.bottom);
	
	// [restore the color through ~saveColor]
}

/**************************************************************************
	DrawValue()										[protected, virtual]
													[complete override ]
	Draw the empty square, X, -, or Microsoft fill
**************************************************************************/
void 
LCDEFCheckbox::DrawValue(
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
	
	// if active
	if (inEnabled) {
		// erase to white
		::BackColor(whiteColor);
		::EraseRect(&partRect);
	}
	
	// draw the X or whatever
	short	value	= GetControlValue();
	switch (value) {
		case Value_Off					:
			// leave empty
			break;
			
		case Value_On					:
			// draw an X
			::MoveTo(partRect.left,partRect.top);
			::LineTo(partRect.right - 1, partRect.bottom - 1);
			::MoveTo(partRect.right - 1, partRect.top);
			::LineTo(partRect.left, partRect.bottom - 1);
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
				::FillRect(&partRect,(PatPtr) &lightGrey);
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
LCDEFCheckbox::DrawFrame(void) const
{
	// frame the checkbox rect
	::FrameRect(&mRadioCheckRect);
	
	// if highlighted
	if (IsHighlighted()) {
		// inset the rect
		RectT	highlightRect;
		highlightRect.Copy(mRadioCheckRect);
		::InsetRect(&highlightRect,1,1);

		// and frame it again
		::FrameRect(&highlightRect);
	}
}

/**************************************************************************
	CalcColorsSelf()								[protected, virtual]
													[complete override ]
	Get the colors for the bevel 3D effect
**************************************************************************/
void
LCDEFCheckbox::CalcColorsSelf(void)
{
	// blend white and background for highlight
	Blend(Grey_White,mBackgroundColor,0.50,mBevelLight);
	
	// blend black and background for shadow
	Blend(Grey_Black,mBackgroundColor,0.30,mBevelShadow);
}

