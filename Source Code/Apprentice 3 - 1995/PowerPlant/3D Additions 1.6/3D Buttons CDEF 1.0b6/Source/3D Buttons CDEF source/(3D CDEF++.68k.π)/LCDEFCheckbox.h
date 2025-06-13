/*
	Public domain by Zig Zichterman.
*/
/*
	LCDEFCheckbox
	
	12/28/94	zz	h	initial write
*/
#pragma once

#include "LCDEFCheckboxOrRadioButton.h"

class LCDEFCheckbox :
	public LCDEFCheckboxOrRadioButton
{
	public :
		LCDEFCheckbox(
			ControlHandle	inControl,
			ButtonKindT		inButtonKind,
			Boolean			inIndentWhenHighlighted,
			Boolean			inUseWindowFont)
			: LCDEFCheckboxOrRadioButton(inControl,inButtonKind,inIndentWhenHighlighted,
				inUseWindowFont)
			{}
	
	protected :
		RGBColor	mBevelShadow;
		RGBColor	mBevelLight;
		
		virtual void 
		CalcRadioCheckRgn(
			RgnHandleT &	outRadioCheckRgn) const;

		virtual void 
		Draw3DEffects(void) const;
		
		virtual void 
		DrawValue(
			Is3DFlagT			inIs3D,
			IsEnabledFlagT		inEnabled,
			IsHighlightedFlagT	inHighlight) const;

		virtual void 
		DrawFrame(void) const;

		virtual void
		CalcColorsSelf(void);

};
